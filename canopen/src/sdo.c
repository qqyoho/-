/*
This file is part of CanFestival, a library implementing CanOpen Stack. 

Copyright (C): Edouard TISSERANT and Francis DUPIN

See COPYING file for copyrights details.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* #define DEBUG_WAR_CONSOLE_ON */
/* #define DEBUG_ERR_CONSOLE_ON */

#include <stdlib.h>
#include "can.h"
#include "objacces.h"
#include "sdo.h"
#include "objdictdef.h"
#include "can_open_timer.h"
#include "def.h"
#include "states.h"

#include "iap_app.h"

#define BOOTLOAD_INDEX                      0x5ff0
#define SDO_MAX_SIMULTANEOUS_TRANSFERTS     4
#define SDO_TIMEOUT_MS                      100

/* Object dictionary */
extern UNS8 bDeviceNodeId;

extern uint32_t g_soft_version;
extern uint32_t g_start_soft_version;
extern uint32_t g_end_soft_version;

/* The Transfer structure
Used to store the different segments of 
 - a SDO received before writing in the dictionary  
 - the reading of the dictionary to put on a SDO to transmit 
*/
struct struct_s_transfer {
    UNS8           nodeId;     /* own ID if server, or node ID of the server if client */
    UNS8           whoami;     /* Takes the values SDO_CLIENT or SDO_SERVER */
    UNS8           state;      /* state of the transmission : Takes the values SDO_... */
    UNS8           toggle;
    UNS32          abortCode;  /* Sent or received */
    /* index and subindex of the dictionary where to store */
    /* (for a received SDO) or to read (for a transmit SDO) */
    UNS16          index; 
    UNS8           subIndex; 
    UNS32          count;      /* Number of data received or to be sent. */
    UNS32          offset;     /* stack pointer of data[]
                                * Used only to tranfer part of a line to or from a SDO.
                                * offset is always pointing on the next free cell of data[].
                                * WARNING s_transfer.data is subject to ENDIANISATION 
                                * (with respect to CANOPEN_BIG_ENDIAN)
                               */
    UNS8           data[SDO_MAX_LENGTH_TRANSFERT];
    UNS8           dataType;   /* Defined in objdictdef.h Value is visible_string 
                                * if it is a string, any other value if it is not a string, 
                                * like 0. In fact, it is used only if client.
                               */
    TIMER_HANDLE   timer;      /* Time counter to implement a timeout in milliseconds.
                                * It is automatically incremented whenever 
                                * the line state is in SDO_DOWNLOAD_IN_PROGRESS or 
                                * SDO_UPLOAD_IN_PROGRESS, and reseted to 0 
                                * when the response SDO have been received.
                                */
    SDOCallback_t Callback;    /* The user callback func to be called at SDO transaction end */
};
typedef struct struct_s_transfer s_transfer;

/*Internals prototypes*/
/* SDO */
s_transfer transfers[SDO_MAX_SIMULTANEOUS_TRANSFERTS];
SDOtimeoutError_t SDOtimeoutError = 0;

UNS8 g_iap_type = 0;

/* s_sdo_parameter *sdo_parameters; */

/***************************************************************************/
/* SDO (un)packing macros */

/** Returns the command specifier (cs, ccs, scs) from the first byte of the SDO   
 */
#define getSDOcs(byte) (byte >> 5)

/** Returns the number of bytes without data from the first byte of the SDO. Coded in 2 bits   
 */
#define getSDOn2(byte) ((byte >> 2) & 3)

/** Returns the number of bytes without data from the first byte of the SDO. Coded in 3 bits   
 */
#define getSDOn3(byte) ((byte >> 1) & 7)

/** Returns the transfer type from the first byte of the SDO   
 */
#define getSDOe(byte) ((byte >> 1) & 1)

/** Returns the size indicator from the first byte of the SDO   
 */
#define getSDOs(byte) (byte & 1)

/** Returns the indicator of end transmission from the first byte of the SDO   
 */
#define getSDOc(byte) (byte & 1)

/** Returns the toggle from the first byte of the SDO   
 */
#define getSDOt(byte) ((byte >> 4) & 1)

/** Returns the index from the bytes 1 and 2 of the SDO   
 */
#define getSDOindex(byte1, byte2) ((byte2 << 8) | (byte1))

/** Returns the subIndex from the byte 3 of the SDO   
 */

#define getSDOsubIndex(byte3) (byte3)

#define StopSDO_TIMER(id)    transfers[id].timer = DelAlarm(transfers[id].timer);

#define StartSDO_TIMER(id)   transfers[id].timer = SetAlarm(id, &SDOTimeoutAlarm, MS_TO_TIMEVAL(SDO_TIMEOUT_MS), 0);

#define RestartSDO_TIMER(id) if(transfers[id].timer != TIMER_NONE) { StopSDO_TIMER(id) StartSDO_TIMER(id) }

/***************************************************************************
**
*/
void SDOTimeoutAlarm(UNS32 id)
{
	UNS8 i;

	for(i = 0; i < SDO_MAX_SIMULTANEOUS_TRANSFERTS; i ++)
	{
		if(transfers[i].nodeId == (UNS8)(id & 0x7f))
		{
			/* Reset timer handler */
			transfers[i].timer = TIMER_NONE;
			/* Call the user function to inform of the problem.*/
			////(SDOtimeoutError)((UNS8)i);
			/* Sending a SDO abort */
			sendSDOabort(transfers[i].whoami, transfers[i].index, transfers[i].subIndex, SDOABT_TIMED_OUT);
			/* Reset the line*/
			resetSDOline(i);
			break;
		}
	}
}

/***************************************************************************/
/** Reset all sdo buffers
 */
void resetSDO(void)
{
	UNS8 j;

	/* transfer structure initialization */
	for (j = 0 ; j < SDO_MAX_SIMULTANEOUS_TRANSFERTS ; j++) 
		resetSDOline(j);
}

/***************************************************************************/
UNS32 SDOlineToObjdict (UNS8 line)
{
	UNS8  size;
	UNS32 errorCode;

	size = (UNS8)transfers[line].count;
	errorCode = setODentry(transfers[line].index, transfers[line].subIndex, (void *)transfers[line].data, &size, 1);
	if (errorCode != OD_SUCCESSFUL)
		return errorCode;
	return 0;
}

/***************************************************************************/
UNS32 objdictToSDOline(UNS8 line)
{
	UNS8  size = 0;
	UNS8  dataType;
    void  *pData;
	UNS32 errorCode;

	errorCode = getODentry(transfers[line].index, 
        transfers[line].subIndex, 
        (void * *)&pData, 
        &size, &dataType, 0);

    memcpy(transfers[line].data, pData, size);
	if (errorCode != OD_SUCCESSFUL)
		return errorCode;

	transfers[line].count = size;
	transfers[line].offset = 0;

	return 0;
}

/***************************************************************************/
UNS8 lineToSDO(UNS8 line, UNS8 nbBytes, UNS8* data) {
	UNS8 i;
	UNS8 offset;

	if ((transfers[line].offset + nbBytes) > SDO_MAX_LENGTH_TRANSFERT) {
		return 0xFF;
	}
    if ((transfers[line].offset + nbBytes) > transfers[line].count) {
		return 0xFF;
	}
	offset = (UNS8)transfers[line].offset;
	for (i = 0 ; i < nbBytes ; i++) 
		*(data + i) = transfers[line].data[offset + i];
	transfers[line].offset += nbBytes;
	return 0;
}

/***************************************************************************/
UNS8 SDOtoLine(UNS8 line, UNS8 nbBytes, UNS8* data)
{
	UNS8 i;
	UNS8 offset;

	if ((transfers[line].offset + nbBytes) > SDO_MAX_LENGTH_TRANSFERT) {
		return 0xFF;
	}
	offset = (UNS8)transfers[line].offset;
	for (i = 0 ; i < nbBytes ; i++) 
		transfers[line].data[offset + i] = *(data + i);
	transfers[line].offset += nbBytes;
	return 0;
}

/***************************************************************************/
UNS8 failedSDO (UNS8 nodeId, UNS8 whoami, UNS16 index, UNS8 subIndex, UNS32 abortCode)
{
	UNS8 err;
	UNS8 line;

	err = getSDOlineOnUse(nodeId, whoami, &line);
	if ((! err) && (whoami == SDO_SERVER)) {
		resetSDOline(line);
	}
	if ((! err) && (whoami == SDO_CLIENT)) {
		StopSDO_TIMER(line);
		transfers[line].state = SDO_ABORTED_INTERNAL;
	}
	err = sendSDOabort(whoami, index, subIndex, abortCode);
	if (err) {
		return 0xFF;
	}

	return 0;
}

/***************************************************************************/
void resetSDOline (UNS8 line)
{
	UNS8 i;

	initSDOline(line, 0, 0, 0, SDO_RESET);
	for (i = 0; i < SDO_MAX_LENGTH_TRANSFERT; i ++)
		transfers[line].data[i] = 0;
}

/***************************************************************************/
UNS8 initSDOline(UNS8 line, UNS8 nodeId, UNS16 index, UNS8 subIndex, UNS8 state)
{
	if (state == SDO_DOWNLOAD_IN_PROGRESS || state == SDO_UPLOAD_IN_PROGRESS)
	{
		StartSDO_TIMER(line)
	}
    else
	{
		StopSDO_TIMER(line)
	}
	transfers[line].nodeId = nodeId; 
	transfers[line].index = index;
	transfers[line].subIndex = subIndex;
	transfers[line].state = state;
	transfers[line].toggle = 0;
	transfers[line].count = 0;
	transfers[line].offset = 0;
	transfers[line].dataType = 0;
	transfers[line].Callback = NULL;  
	return 0;
}

/***************************************************************************/
UNS8 getSDOfreeLine (UNS8 whoami, UNS8 *line)
{
	UNS8 i, ret = 0xff;

	for (i = 0 ; i < SDO_MAX_SIMULTANEOUS_TRANSFERTS ; i++){
		if (transfers[i].state == SDO_RESET) {
			*line = i;
			transfers[i].whoami = whoami;
			ret = 0;
			break;
		} /* end if */
	} /* end for */
	return ret;
}

/***************************************************************************/
UNS8 getSDOlineOnUse(UNS8 nodeId, UNS8 whoami, UNS8 *line)
{	
	UNS8 i, ret = 0xff;

	for (i = 0 ; i < SDO_MAX_SIMULTANEOUS_TRANSFERTS ; i++){
		if ( (transfers[i].state != SDO_RESET) &&
			(transfers[i].nodeId == nodeId) && 
			(transfers[i].whoami == whoami) ) {
			*line = i;
			ret = 0;
			break;
		}
	}
	return ret;
}

/***************************************************************************/
UNS8 closeSDOtransfer (UNS8 nodeId, UNS8 whoami)
{
	UNS8 line;
	UNS8 ret = 0xff;

	if(!getSDOlineOnUse(nodeId, whoami, &line))
	{
		resetSDOline(line);  
		ret = 0;
	}

	return ret;
}

/***************************************************************************/
UNS8 getSDOlineRestBytes (UNS8 line, UNS8 * nbBytes)
{
	if (transfers[line].count == 0) /* if received initiate SDO protocol with e=0 and s=0 */
		*nbBytes = 0;
	else
		*nbBytes = (UNS8)transfers[line].count - (UNS8)transfers[line].offset;
	return 0;
}

/***************************************************************************/
UNS8 setSDOlineRestBytes (UNS8 line, UNS8 nbBytes)
{
	if (nbBytes > SDO_MAX_LENGTH_TRANSFERT) {
		return 0xFF;
	}
	transfers[line].count = nbBytes;
	return 0;
}


/***************************************************************************/
UNS8 sendSDO(UNS8 whoami, s_SDO sdo)
{	
	Message m;
	UNS32   *pwCobId = NULL;
	UNS8    found = 0;
	UNS8    i, size, dataType;

	if(!((getState() == Operational) ||  (getState() == Pre_operational ))) {
		return 0xFF;
	}

	/* get the server->client cobid */
	if (whoami == SDO_SERVER)
	/* case server. Easy because today only one server SDO is authorized in CanFestival */
	{
        dataType = 0;
        size = 0;
		if(OD_SUCCESSFUL == getODentry(0x1200, 2, (void * *)&pwCobId, &size, &dataType, 0))
		{
			found = 1;
		}
	}
	else
	/* case client */
	{
        dataType = 0;
        size = 0;
		if(OD_SUCCESSFUL == getODentry(0x1200, 1, (void * *)&pwCobId, &size, &dataType, 0))
		{
			if(sdo.nodeId == (UNS8)((*pwCobId) & 0x7f))
			{
				found = 1;
			}
		}
	}
    if(found == 1)
    {
        /* message copy for sending */
        m.cob_id.w = *pwCobId;
        m.rtr = NOT_A_REQUEST; 
        /* the length of SDO must be 8 */
        m.len = 8;
        for (i = 0 ; i < 8 ; i++)
        {
            m.data[i] =  sdo.body.data[i];
        }
        return can_std_transmit(m.cob_id.w, m.data, m.rtr, m.len);
    }
    return 0;
}

/***************************************************************************/
UNS8 sendSDOabort (UNS8 whoami, UNS16 index, UNS8 subIndex, UNS32 abortCode)
{
	s_SDO sdo;
	UNS8 ret;

	sdo.nodeId = bDeviceNodeId;
	sdo.body.data[0] = 0x80;
	/* Index */
	sdo.body.data[1] = index & 0xFF; /* LSB */
	sdo.body.data[2] = (index >> 8) & 0xFF; /* MSB */
	/* Subindex */
	sdo.body.data[3] = subIndex;
	/* Data */
	sdo.body.data[4] = (UNS8)(abortCode & 0xFF);
	sdo.body.data[5] = (UNS8)((abortCode >> 8) & 0xFF);
	sdo.body.data[6] = (UNS8)((abortCode >> 16) & 0xFF);
	sdo.body.data[7] = (UNS8)((abortCode >> 24) & 0xFF);
	ret = sendSDO(whoami, sdo);

	return ret;
}
/***************************************************************************/
UNS8 proceedSDO(Message *m)
{
	UNS8  err;
	UNS8  line;
	UNS8  nbBytes; /* received or to be transmited. */
	UNS8  nodeId; /* The node from which the SDO is received */
	UNS8  whoami; /* SDO_SERVER or SDO_CLIENT.*/
	UNS32 errorCode; /* while reading or writing in the local object dictionary.*/
	s_SDO sdo; /* SDO to transmit */
	UNS16 index;
	UNS8  subIndex, size;
	UNS8  i, dataType;
	UNS32 *pCobId = NULL;

	nodeId = (m->cob_id.w & 0x7f);
    
    if (0 == getSDOcs(m->data[0]))
    {
        whoami = SDO_SERVER;
    }

	whoami = SDO_UNKNOWN;
	/* Looking for the cobId in the object dictionary. */
	/* Am-I a server ? */
	size = 0;
	if (OD_SUCCESSFUL == getODentry(0x1200, 2, (void * *)&pCobId, &size, &dataType, 0))
	/* 获取SDO Server ID */
	{
		if (*pCobId == m->cob_id.w)
		{
			whoami = SDO_SERVER;
		}
	}
	else
	{
		size = 0;
		if (OD_SUCCESSFUL == getODentry(0x1200, 1, (void * *)&pCobId, &size, &dataType, 0))
		/* 获取SDO Client ID */
		{
			if (*pCobId == m->cob_id.w)
			{
				whoami = SDO_CLIENT;
			}
		}
	}

	if (whoami == SDO_UNKNOWN)
	/* 本SDO与本设备无关 */
	{
		return 0xFF;/* This SDO was not for us ! */
	}

	/* Test if the size of the SDO is ok */
	if (m->len != 8)
	{
		failedSDO(nodeId, whoami, 0, 0, SDOABT_GENERAL_ERROR);
		return 0xFF;
	}

	/* Testing the command specifier */
	/* Allowed : cs = 0, 1, 2, 3, 4. (=  all except those for block tranfert). */
	/* cs = other : Not allowed -> abort. */
	switch (getSDOcs(m->data[0]))
	{
	case 0:
		/* I am SERVER */
		if (whoami == SDO_SERVER)
		{
			/* Receiving a download segment data. */
			/* A SDO transfert should have been yet initiated. */
			err = getSDOlineOnUse(nodeId, whoami, &line);
			if (!err)
			{
				err = (transfers[line].state != SDO_DOWNLOAD_IN_PROGRESS);
			}
			if (err)
			{
				failedSDO(nodeId, whoami, 0, 0, SDOABT_LOCAL_CTRL_ERROR);
				return 0xFF;
			}
			/* Reset the wathdog */
			RestartSDO_TIMER(line)
			index = transfers[line].index;

			subIndex = transfers[line].subIndex;
			/* Toggle test. */
			if (transfers[line].toggle != getSDOt(m->data[0]))
			{
				failedSDO(nodeId, whoami, index, subIndex, SDOABT_TOGGLE_NOT_ALTERNED);
				return 0xFF;
			}
			/* Nb of data to be downloaded */
			nbBytes = 7 - getSDOn3(m->data[0]);
			/* Store the data in the transfert structure. */
			err = SDOtoLine(line, nbBytes, (*m).data + 1);
			if (err)
			{
				failedSDO(nodeId, whoami, index, subIndex, SDOABT_GENERAL_ERROR);
				return 0xFF;
			}
			/* Sending the SDO response, CS = 1 */
			sdo.nodeId = bDeviceNodeId; /* The node id of the server, (here it is the sender). */
			sdo.body.data[0] = (1 << 5) | (transfers[line].toggle << 4);
			for (i = 1 ; i < 8 ; i++)
				sdo.body.data[i] = 0;
			sendSDO(whoami, sdo);
			/* Inverting the toggle for the next segment. */
			transfers[line].toggle = !transfers[line].toggle & 1;
			/* If it was the last segment, */
			if (getSDOc(m->data[0]))
			{
				/* Transfering line data to object dictionary. */
				/* The code does not use the "d" of initiate frame. So it is safe if e=s=0 */
				errorCode = SDOlineToObjdict(line);
				if(errorCode)
				{
					failedSDO(nodeId, whoami, index, subIndex, errorCode);
					/* Release of the line */
					resetSDOline(line);
					return 0xFF;
				}
				/* Release of the line */
				resetSDOline(line);
			}
		} /* end if SERVER */
		else{ /* if CLIENT */
			/* I am CLIENT */
			/* It is a request for a previous upload segment. We should find a line opened for this.*/
			err = getSDOlineOnUse(nodeId, whoami, &line);
			if (!err)
				err = transfers[line].state != SDO_UPLOAD_IN_PROGRESS;
			if (err) {
				failedSDO(nodeId, whoami, 0, 0, SDOABT_LOCAL_CTRL_ERROR);
				return 0xFF;
			}
			/* Reset the wathdog */
			RestartSDO_TIMER(line)
			index = transfers[line].index;
			subIndex = transfers[line].subIndex;
			/* test of the toggle; */
			if (transfers[line].toggle != getSDOt(m->data[0])) {
				failedSDO(nodeId, whoami, index, subIndex, SDOABT_TOGGLE_NOT_ALTERNED);
				return 0xFF;
			}
			/* nb of data to be uploaded */
			nbBytes = 7 - getSDOn3(m->data[0]);
			/* Storing the data in the line structure. */
			err = SDOtoLine(line, nbBytes, (*m).data + 1);
			if (err) {
				failedSDO(nodeId, whoami, index, subIndex, SDOABT_GENERAL_ERROR);
				return 0xFF;
			}
			/* Inverting the toggle for the next segment. */
			transfers[line].toggle = ! transfers[line].toggle & 1;
			/* If it was the last segment,*/
			if ( getSDOc(m->data[0])) {
				/* Put in state finished */
				/* The code is safe for the case e=s=0 in initiate frame. */
				StopSDO_TIMER(line)
				transfers[line].state = SDO_FINISHED;
				if(transfers[line].Callback) (*transfers[line].Callback)(nodeId);
			}
			else { /* more segments to receive */
				/* Sending the request for the next segment. */
				sdo.nodeId = nodeId;
				sdo.body.data[0] = (3 << 5) | (transfers[line].toggle << 4);
				for (i = 1 ; i < 8 ; i++)
					sdo.body.data[i] = 0;
				sendSDO(whoami, sdo);
			}            
		} /* End if CLIENT */
		break;

	case 1:
		/* I am SERVER */
		/* Receive of an initiate download */
		if (whoami == SDO_SERVER)
		{
            uint32_t soft_temp = m->data[7];

            soft_temp = (soft_temp << 8) + m->data[6];
            soft_temp = (soft_temp << 8) + m->data[5];
            soft_temp = (soft_temp << 8) + m->data[4];

			index = getSDOindex(m->data[1],m->data[2]);
			subIndex = getSDOsubIndex(m->data[3]);
            
			if ((index == BOOTLOAD_INDEX) && (subIndex == 2))
			{
                if (g_iap_type == 1)
                {
                    /* Expedited upload. (cs = 2 ; e = 1) */
                    sdo.body.data[0] = (3 << 5);  
                    sdo.body.data[1] = index & 0xFF;        /* LSB */
                    sdo.body.data[2] = (index >> 8) & 0xFF; /* MSB */
                    sdo.body.data[3] = subIndex;
                    sdo.body.data[4] = 0x00;
                    sdo.body.data[5] = 0x00;
                    sdo.body.data[6] = 0x00;
                    sdo.body.data[7] = 0x00;
                    sendSDO(whoami, sdo);
                    start_iap_process();
                }
                else
                {
                    failedSDO(nodeId, whoami, index, subIndex, SDOABT_LOCAL_CTRL_ERROR);
                    return 0xFF;
                }
			}
			else
			{
				/* Search if a SDO transfert have been yet initiated */
				err = getSDOlineOnUse(nodeId, whoami, &line );
				if (!err)
				{
					failedSDO(nodeId, whoami, index, subIndex, SDOABT_LOCAL_CTRL_ERROR);
					return 0xFF;
				}
				/* No line on use. Great ! */
				/* Try to open a new line. */
				err = getSDOfreeLine(whoami, &line);
				if(err)
				{
					failedSDO(nodeId, whoami, index, subIndex, SDOABT_LOCAL_CTRL_ERROR);
					return 0xFF;
				}
				initSDOline(line, nodeId, index, subIndex, SDO_DOWNLOAD_IN_PROGRESS);      

				if(getSDOe(m->data[0]))
				/* If SDO expedited */
				{
					/* nb of data to be downloaded */
					nbBytes = 4 - getSDOn2(m->data[0]);
					/* Storing the data in the line structure. */
					transfers[line].count = nbBytes;
					err = SDOtoLine(line, nbBytes, (*m).data + 4);

					if(err)
					{
						failedSDO(nodeId, whoami, index, subIndex, SDOABT_GENERAL_ERROR);
						return 0xFF;
					}

					/* SDO expedited -> transfert finished. Data can be stored in the dictionary. */
					/* The line will be reseted when it is downloading in the dictionary. */
					/* Transfering line data to object dictionary. */
					errorCode = SDOlineToObjdict(line);
					if (errorCode) {
						failedSDO(nodeId, whoami, index, subIndex, errorCode);
						return 0xFF;
					}
					/* Release of the line. */
					resetSDOline(line);
				}
				else{/* So, if it is not an expedited transfert */
					if (getSDOs(m->data[0])) {
						/* TODO : if e and s = 0, not reading m->data[4] but put nbBytes = 0 */
						nbBytes = m->data[4]; /* Transfert limited to 255 bytes. */
						err = setSDOlineRestBytes(line, nbBytes);
						if (err) {
							failedSDO(nodeId, whoami, index, subIndex, SDOABT_GENERAL_ERROR);
							return 0xFF;
						}
					}
				}
				/*Sending a SDO, cs=3*/
				sdo.nodeId = bDeviceNodeId; /* The node id of the server, (here it is the sender).*/
				sdo.body.data[0] = 3 << 5;
				sdo.body.data[1] = index & 0xFF;        /* LSB */
				sdo.body.data[2] = (index >> 8) & 0xFF; /* MSB */
				sdo.body.data[3] = subIndex;
				for (i = 4 ; i < 8 ; i++)
					sdo.body.data[i] = 0;
				sendSDO(whoami, sdo);
			}
		} /* end if I am SERVER */
		else
		{
			/* I am CLIENT */
			/* It is a response for a previous download segment. We should find a line opened for this. */
			err = getSDOlineOnUse(nodeId, whoami, &line);
			if(!err)
				err = transfers[line].state != SDO_DOWNLOAD_IN_PROGRESS;
			if(err)
			{
				failedSDO(nodeId, whoami, 0, 0, SDOABT_LOCAL_CTRL_ERROR);
				return 0xFF;
			}
			/* Reset the wathdog */
			RestartSDO_TIMER(line);
			index = transfers[line].index;
			subIndex = transfers[line].subIndex;
			/* test of the toggle; */
			if(transfers[line].toggle != getSDOt(m->data[0]))
			{
				failedSDO(nodeId, whoami, index, subIndex, SDOABT_TOGGLE_NOT_ALTERNED);
				return 0xFF;
			}

			/* End transmission or downloading next segment. We need to know if it will be the last one. */
			getSDOlineRestBytes(line, &nbBytes);
			if(nbBytes == 0)
			{
				StopSDO_TIMER(line);
				transfers[line].state = SDO_FINISHED;
				if(transfers[line].Callback) (*transfers[line].Callback)(nodeId);
				return 0x00;
			}
			/* At least one transfer to send.	*/
			if(nbBytes > 7)
			{
				/* several segments to download.*/
				/* code to send the next segment. (cs = 0; c = 0) */
				transfers[line].toggle = ! transfers[line].toggle & 1;
				sdo.nodeId = nodeId; /* The server node Id; */
				sdo.body.data[0] = (transfers[line].toggle << 4);
				err = lineToSDO(line, 7, sdo.body.data + 1);	 
				if(err)
				{
					failedSDO(nodeId, whoami, index, subIndex, SDOABT_GENERAL_ERROR);
					return 0xFF;
				}
			}
			else
			{
				/* Last segment. */
				/* code to send the last segment. (cs = 0; c = 1)*/
				transfers[line].toggle = ! transfers[line].toggle & 1;
				sdo.nodeId = nodeId; /* The server node Id; */
				sdo.body.data[0] = (transfers[line].toggle << 4) | ((7 - nbBytes) << 1) | 1;
				err = lineToSDO(line, nbBytes, sdo.body.data + 1);	 
				if(err)
				{
					failedSDO(nodeId, whoami, index, subIndex, SDOABT_GENERAL_ERROR);
					return 0xFF;
				}
				for (i = nbBytes + 1 ; i < 8 ; i++)
					sdo.body.data[i] = 0;
			}
			sendSDO(whoami, sdo);
		} /* end if I am a CLIENT */			  
		break;

	case 2:
		/* I am SERVER */
		/* Receive of an initiate upload.*/
		if(whoami == SDO_SERVER)
		{
			index = getSDOindex(m->data[1],m->data[2]);
			subIndex = getSDOsubIndex(m->data[3]);
			if ((index == BOOTLOAD_INDEX) && (subIndex == 1))
			{
				/* Expedited upload. (cs = 2 ; e = 1) */
				sdo.body.data[0] = (2 << 5) | 3;  
				sdo.body.data[1] = index & 0xFF;        /* LSB */
				sdo.body.data[2] = (index >> 8) & 0xFF; /* MSB */
				sdo.body.data[3] = subIndex;
				sdo.body.data[4] = g_soft_version & 0xff;
				sdo.body.data[5] = (g_soft_version >> 8) & 0xff;
				sdo.body.data[6] = (g_soft_version >> 16) & 0xff;
				sdo.body.data[7] = (g_soft_version >> 24) & 0xff;
				sendSDO(whoami, sdo);

				g_iap_type = 1;
			}
			else
			{
				/* Search if a SDO transfert have been yet initiated*/
				err = getSDOlineOnUse(nodeId, whoami, &line );
				if(! err)
				{
					failedSDO(nodeId, whoami, index, subIndex, SDOABT_LOCAL_CTRL_ERROR);
					return 0xFF;
				}
				/* No line on use. Great !*/
				/* Try to open a new line.*/
				err = getSDOfreeLine( whoami, &line );
				if(err)
				{
					failedSDO(nodeId, whoami, index, subIndex, SDOABT_LOCAL_CTRL_ERROR);
					return 0xFF;
				}
				initSDOline(line, nodeId, index, subIndex, SDO_UPLOAD_IN_PROGRESS);
				/* Transfer data from dictionary to the line structure. */
				errorCode = objdictToSDOline(line);
		     
				if(errorCode)
				{
					failedSDO(nodeId, whoami, index, subIndex, errorCode);
					return 0xFF;
				}
				/* Preparing the response.*/
				getSDOlineRestBytes(line, &nbBytes);	/* Nb bytes to transfer ? */
				sdo.nodeId = nodeId; /* The server node Id; */
				if (nbBytes > 4) {
					/* normal transfert. (segmented). */
					/* code to send the initiate upload response. (cs = 2) */
					sdo.body.data[0] = (2 << 5) | 1;
					sdo.body.data[1] = index & 0xFF;        /* LSB */
					sdo.body.data[2] = (index >> 8) & 0xFF; /* MSB */
					sdo.body.data[3] = subIndex;
					sdo.body.data[4] = nbBytes; /* Limitation of canfestival2 : Max tranfert is 256 bytes.*/
					/* It takes too much memory to upgrate to 2^32 because the size of data is also coded */
					/* in the object dictionary, at every index and subindex. */
					for (i = 5 ; i < 8 ; i++)
						sdo.body.data[i] = 0;
					sendSDO(whoami, sdo); 
				}
				else
                {
					/* Expedited upload. (cs = 2 ; e = 1) */
					sdo.body.data[0] = (2 << 5) | ((4 - nbBytes) << 2) | 3;  
					sdo.body.data[1] = index & 0xFF;        /* LSB */
					sdo.body.data[2] = (index >> 8) & 0xFF; /* MSB */
					sdo.body.data[3] = subIndex;
					err = lineToSDO(line, nbBytes, sdo.body.data + 4);
					if (err)
                    {
						failedSDO(nodeId, whoami, index, subIndex, SDOABT_GENERAL_ERROR);
						return 0xFF;
					}
					for (i = 4 + nbBytes ; i < 8 ; i++)
						sdo.body.data[i] = 0;
					sendSDO(whoami, sdo); 
					/* Release the line.*/
					resetSDOline(line);
				}
			}
		} /* end if I am SERVER*/
		else
		{
			/* I am CLIENT */
			/* It is the response for the previous initiate upload request.*/
			/* We should find a line opened for this. */
			err = getSDOlineOnUse(nodeId, whoami, &line);
			if (!err)
				err = transfers[line].state != SDO_UPLOAD_IN_PROGRESS;
			if (err) {
				failedSDO(nodeId, whoami, 0, 0, SDOABT_LOCAL_CTRL_ERROR);
				return 0xFF;
			}
			/* Reset the wathdog */
			RestartSDO_TIMER(line)
			index = transfers[line].index;
			subIndex = transfers[line].subIndex;

			if (getSDOe(m->data[0])) { /* If SDO expedited */
				/* nb of data to be uploaded */
				nbBytes = 4 - getSDOn2(m->data[0]);
				/* Storing the data in the line structure. */
				err = SDOtoLine(line, nbBytes, (*m).data + 4);
				if (err) {
					failedSDO(nodeId, whoami, index, subIndex, SDOABT_GENERAL_ERROR);
					return 0xFF;
				}
				/* SDO expedited -> transfert finished. data are available via  getReadResultNetworkDict(). */
				StopSDO_TIMER(line)
				transfers[line].count = nbBytes;
				transfers[line].state = SDO_FINISHED;
				if(transfers[line].Callback) (*transfers[line].Callback)(nodeId);
					return 0;
			}
			else { /* So, if it is not an expedited transfert */
				/* Storing the nb of data to receive. */
				if (getSDOs(m->data[0])) {
					nbBytes = m->data[4]; /* Remember the limitation to 255 bytes to transfert */
					err = setSDOlineRestBytes(line, nbBytes);
					if(err)
					{
						failedSDO(nodeId, whoami, index, subIndex, SDOABT_GENERAL_ERROR);
						return 0xFF;
					}	
				}
				/* Requesting next segment. (cs = 3) */
				sdo.nodeId = nodeId;
				sdo.body.data[0] = 3 << 5;
				for (i = 1 ; i < 8 ; i++)
					sdo.body.data[i] = 0;
				sendSDO(whoami, sdo);  
			}
		} /* End if CLIENT */
		break;

	case 3:
		/* I am SERVER */
		if(whoami == SDO_SERVER)
		{
			/* Receiving a upload segment. */
			/* A SDO transfert should have been yet initiated. */
			err = getSDOlineOnUse(nodeId, whoami, &line ); 
			if(!err)
				err = transfers[line].state != SDO_UPLOAD_IN_PROGRESS;
			if(err)
			{
				failedSDO(nodeId, whoami, 0, 0, SDOABT_LOCAL_CTRL_ERROR);
				return 0xFF;
			}
			/* Reset the wathdog */
			RestartSDO_TIMER(line)
			index = transfers[line].index;
			subIndex = transfers[line].subIndex;
			/* Toggle test.*/
			if(transfers[line].toggle != getSDOt(m->data[0]))
			{
				failedSDO(nodeId, whoami, index, subIndex, SDOABT_TOGGLE_NOT_ALTERNED);
				return 0xFF;
			}
			/* Uploading next segment. We need to know if it will be the last one. */
			getSDOlineRestBytes(line, &nbBytes);	  	  
			if(nbBytes > 7)
			{
				/* The segment to transfer is not the last one.*/
				/* code to send the next segment. (cs = 0; c = 0) */
				sdo.nodeId = nodeId; /* The server node Id; */
				sdo.body.data[0] = (transfers[line].toggle << 4);
				err = lineToSDO(line, 7, sdo.body.data + 1);	 
				if (err) {
					failedSDO(nodeId, whoami, index, subIndex, SDOABT_GENERAL_ERROR);
					return 0xFF;
				}
				/* Inverting the toggle for the next tranfert. */
				transfers[line].toggle = ! transfers[line].toggle & 1;
				sendSDO(whoami, sdo); 
			} 
			else
			{
				/* Last segment. */
				/* code to send the last segment. (cs = 0; c = 1) */	    
				sdo.nodeId = nodeId; /* The server node Id; */
				sdo.body.data[0] = (transfers[line].toggle << 4) | ((7 - nbBytes) << 1) | 1;
				err = lineToSDO(line, nbBytes, sdo.body.data + 1);	 
				if(err)
				{
					failedSDO(nodeId, whoami, index, subIndex, SDOABT_GENERAL_ERROR);
					return 0xFF;
				}
				for (i = nbBytes + 1 ; i < 8 ; i++)
					sdo.body.data[i] = 0;
				sendSDO(whoami, sdo);
				/* Release the line */
				resetSDOline(line);
			}
		} /* end if SERVER*/
		else
		{
			/* I am CLIENT */
			/* It is the response for the previous initiate download request. */
			/* We should find a line opened for this. */
			err = getSDOlineOnUse(nodeId, whoami, &line);
			if (!err)
			err = transfers[line].state != SDO_DOWNLOAD_IN_PROGRESS;
			if(err)
			{
				failedSDO(nodeId, whoami, 0, 0, SDOABT_LOCAL_CTRL_ERROR);
				return 0xFF;
			}
			/* Reset the watchdog */
			RestartSDO_TIMER(line)
			index = transfers[line].index;
			subIndex = transfers[line].subIndex;
			/* End transmission or requesting  next segment. */
			getSDOlineRestBytes(line, &nbBytes);
			if(nbBytes == 0)
			{
				StopSDO_TIMER(line)
				transfers[line].state = SDO_FINISHED;
				if(transfers[line].Callback) (*transfers[line].Callback)(nodeId);
					return 0x00;
			}	  
			if(nbBytes > 7)
			{
				/* more than one request to send */
				/* code to send the next segment. (cs = 0; c = 0)	*/    
				sdo.nodeId = nodeId; /* The server node Id; */
				sdo.body.data[0] = (transfers[line].toggle << 4);
				err = lineToSDO(line, 7, sdo.body.data + 1);	 
				if(err)
				{
					failedSDO(nodeId, whoami, index, subIndex, SDOABT_GENERAL_ERROR);
					return 0xFF;
				}
			}
			else
			{
				/* Last segment.*/
				/* code to send the last segment. (cs = 0; c = 1)	*/   
				sdo.nodeId = nodeId; /* The server node Id; */
				sdo.body.data[0] = (transfers[line].toggle << 4) | ((7 - nbBytes) << 1) | 1;
				err = lineToSDO(line, nbBytes, sdo.body.data + 1);	 
				if (err) {
					failedSDO(nodeId, whoami, index, subIndex, SDOABT_GENERAL_ERROR);
					return 0xFF;
				}
				for (i = nbBytes + 1 ; i < 8 ; i++)
					sdo.body.data[i] = 0;
			}
			sendSDO(whoami, sdo); 
		} /* end if I am a CLIENT		*/	  
		break;

	case 4:
#if 0
		abortCode = (*m).data[3] |
			((UNS32)m->data[5] << 8) |
			((UNS32)m->data[6] << 16) |
			((UNS32)m->data[7] << 24);
#endif
		/* Received SDO abort. */
		/* Looking for the line concerned. */
		if(whoami == SDO_SERVER)
		{
			err = getSDOlineOnUse(nodeId, whoami, &line);
			if(!err)
			{
				resetSDOline(line);
			}
			/* Tips : The end user has no way to know that the server node has received an abort SDO. */
			/* Its is ok, I think.*/
		}
		else
		/* If I am CLIENT */
		{
			err = getSDOlineOnUse(nodeId, whoami, &line );
			if (!err)
			{
				/* The line *must* be released by the core program. */
				StopSDO_TIMER(line)
				transfers[line].state = SDO_ABORTED_RCV;
			}
		} 
		break;
	default:
		/* Error : Unknown cs */
		return 0xFF;
	} /* End switch */
	return 0;
}

/*******************************************************************)******/
UNS8 writeNetworkDict(UNS8 nodeId, UNS16 index, UNS8 subIndex, UNS8 count, UNS8 dataType, void *data, SDOCallback_t Callback)
{
	UNS32 *pNodeIdServer;
	s_SDO sdo;/* SDO to transmit */
	UNS8  err;
	UNS8  SDOfound = 0;
	UNS8  line;
	UNS8  i, j, size;

	/* Verify that there is no SDO communication yet. */
	err = getSDOlineOnUse(nodeId, SDO_CLIENT, &line);
	if (!err) {
		return 0xFF;
	}
	/* Taking the line ... */
	err = getSDOfreeLine(SDO_CLIENT, &line );
	if (err) {
		return (0xFF);
	}

	if(OD_SUCCESSFUL == getODentry(0x1200, 2, (void * *)&pNodeIdServer, &size, &dataType, 0))
	{
		if((UNS8)(*pNodeIdServer & 0x7f) == nodeId)
		{
			SDOfound = 1;
		}
	}
	if (!SDOfound) {
		return 0xFF;
	}

	initSDOline(line, nodeId, index, subIndex, SDO_DOWNLOAD_IN_PROGRESS);
	transfers[line].count = count;
	transfers[line].dataType = dataType;

	/* Copy data to transfers structure. */
	for (j = 0 ; j < count ; j++) {
#ifdef CANOPEN_BIG_ENDIAN
		if (dataType == 0)
			transfers[line].data[count - 1 - j] = ((char *)data)[j];
		else /* String of bytes. */
			transfers[line].data[j] = ((char *)data)[j];
#else 
			transfers[line].data[j] = ((char *)data)[j];
#endif
	}
	/* Send the SDO to the server. Initiate download, cs=1. */
	sdo.nodeId = nodeId;
	if (count <= 4) { /* Expedited transfert */
		sdo.body.data[0] = (1 << 5) | ((4 - count) << 2) | 3;
		for (i = 4 ; i < 8 ; i++)
			sdo.body.data[i] = transfers[line].data[i - 4];
		transfers[line].offset = count;
	}	
	else { /* Normal transfert */
		sdo.body.data[0] = (1 << 5) | 1;
	sdo.body.data[4] = count; /* nb of byte to transmit. Max = 255. (canfestival2 limitation). */
	for (i = 5 ; i < 8 ; i++)
		sdo.body.data[i] = 0;
	}
	sdo.body.data[1] = index & 0xFF;        /* LSB */
	sdo.body.data[2] = (index >> 8) & 0xFF; /* MSB */
	sdo.body.data[3] = subIndex;

	transfers[line].Callback = Callback;

	err = sendSDO(SDO_CLIENT, sdo);
	if (err) {
		/* release the line */
		resetSDOline(line);
		return 0xFF;
	}

	return 0;
}

/*--------------------------------------------------------------------------*/

UNS8 writeNetworkDictCallBack (UNS8 nodeId, UNS16 index, 
		       UNS8 subIndex, UNS8 count, UNS8 dataType, void *data, SDOCallback_t Callback)
{
	return writeNetworkDict (nodeId, index, subIndex, count, dataType, data, Callback);	
}


/***************************************************************************/
UNS8 readNetworkDict (UNS8 nodeId, UNS16 index, UNS8 subIndex, UNS8 dataType, SDOCallback_t Callback)
{
	UNS32 *pNodeIdServer;
	UNS8  err;
	UNS8  SDOfound = 0;
	UNS8  i, size;
	UNS8  line;
	s_SDO sdo;    /* SDO to transmit */

	/* Verify that there is no SDO communication yet. */
	err = getSDOlineOnUse(nodeId, SDO_CLIENT, &line);
	if (!err) {
		return 0xFF;
	}
	/* Taking the line ... */
	err = getSDOfreeLine(SDO_CLIENT, &line);
	if (err)
	{
		return (0xFF);
	}

	if(OD_SUCCESSFUL == getODentry(0x1200, 2, (void * *)&pNodeIdServer, &size, &dataType, 0))
	{
		if((UNS8)(*pNodeIdServer & 0x7f) == nodeId)
		{
			SDOfound = 1;
		}
	}
	if (!SDOfound) {
		return 0xFF;
	}

	initSDOline(line, nodeId, index, subIndex, SDO_UPLOAD_IN_PROGRESS);
	getSDOlineOnUse(nodeId, SDO_CLIENT, &line);
	sdo.nodeId = nodeId;
	/* Send the SDO to the server. Initiate upload, cs=2. */
	transfers[line].dataType = dataType;				
	sdo.body.data[0] = (2 << 5);	
	sdo.body.data[1] = index & 0xFF;        /* LSB */
	sdo.body.data[2] = (index >> 8) & 0xFF; /* MSB */
	sdo.body.data[3] = subIndex;
	for (i = 4 ; i < 8 ; i++)
		sdo.body.data[i] = 0;
	transfers[line].Callback = Callback;
	err = sendSDO(SDO_CLIENT, sdo);
	if (err) {
		/* release the line */
		resetSDOline(line);
		return 0xFF;
	}
	return 0;
}

/*--------------------------------------------------------------------------*/
UNS8 readNetworkDictCallback (UNS8 nodeId, UNS16 index, UNS8 subIndex, UNS8 dataType, SDOCallback_t Callback)
{
	return readNetworkDict (nodeId, index, subIndex, dataType, Callback);
}

/***************************************************************************/
UNS8 getReadResultNetworkDict (UNS8 nodeId, void* data, UNS8 *size, UNS32 * abortCode)
{
	UNS8 i;
	UNS8 err;
	UNS8 line;

	*size = 0;

	/* Looking for the line tranfert. */
	err = getSDOlineOnUse(nodeId, SDO_CLIENT, &line);
	if (err) {
		return SDO_ABORTED_INTERNAL;
	}
	if (transfers[line].state != SDO_FINISHED)
		return transfers[line].state;

	/* Transfert is finished. Put the value in the data. */
	* size = (UNS8)transfers[line].count;
	for  ( i = 0 ; i < *size ; i++) {
#ifdef CANOPEN_BIG_ENDIAN
		if (transfers[line].dataType != visible_string)
			((char *)data)[*size - 1 - i] = transfers[line].data[i];
		else /* String of bytes. */
			((char *) data)[i] = transfers[line].data[i];
#else 
			( (char *) data)[i] = transfers[line].data[i];
#endif
	} 
	return SDO_FINISHED;
}

/***************************************************************************/
UNS8 getWriteResultNetworkDict (UNS8 nodeId, UNS32 * abortCode)
{
	UNS8 line = 0;
	UNS8 err;

	* abortCode = 0;
	/* Looking for the line tranfert. */
	err = getSDOlineOnUse(nodeId, SDO_CLIENT, &line);
	if (err) {
		return SDO_ABORTED_INTERNAL;
	}
	*abortCode = transfers[line].abortCode;
	return transfers[line].state;
}

void _SDOtimeoutError (UNS8 line)
{
}

