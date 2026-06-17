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
#include "can.h"
#include "data.h"
#include "sync.h"
#include "objdictdef.h"
#include "def.h"
#include "can_open_timer.h"
#include "states.h"
#include "pdo.h"
#include "sdo.h"
#include "Objacces.h"

extern const indextable *objdict;
extern quick_index obj_firstIndex;
extern quick_index obj_lastIndex;
extern UNS8 *count_sync;
extern s_process_var process_var;

/* SYNC */
TIMER_HANDLE syncTimer;
UNS32 *COB_ID_Sync;
UNS32 *Sync_Cycle_Period;
/*UNS32 *Sync_window_length;;*/

/* Prototypes for internals functions */
void SyncAlarm(UNS32 id);

/*****************************************************************************/
void SyncAlarm(UNS32 id)
{
	sendSYNC(*COB_ID_Sync & 0x1FFFFFFF);
}

/*****************************************************************************/
void startSYNC(void)
{
	////RegisterSetODentryCallBack(d, 0x1005, 0, &OnCOB_ID_SyncUpdate);
	////RegisterSetODentryCallBack(d, 0x1006, 0, &OnCOB_ID_SyncUpdate);

	if(syncTimer != TIMER_NONE)
	{
		stopSYNC();
	}

	if(*COB_ID_Sync & 0x40000000 && *Sync_Cycle_Period)
	{
		    syncTimer = SetAlarm(0/* No id needed */, &SyncAlarm,
			US_TO_TIMEVAL(*Sync_Cycle_Period), 
			US_TO_TIMEVAL(*Sync_Cycle_Period));
	}
}

/*****************************************************************************/
void stopSYNC(void)
{
	syncTimer = DelAlarm(syncTimer);
}

/*********************************************************************/
UNS8 sendSYNC(UNS32 cob_id)
{
	Message m;
	UNS8 resultat;

	m.cob_id.w = cob_id ;
	m.rtr = NOT_A_REQUEST;
	m.len = 0;
	resultat = can_std_transmit(m.cob_id.w, m.data, m.rtr, m.len);
	proceedSYNC(&m);

	return resultat;
}

/*****************************************************************************/
UNS8 proceedSYNC(Message *m)
{
	UNS8  pdoNum,       /* number of the actual processed pdo-nr. */
	      prp_j;
	const UNS8 *pMappingCount = NULL;      /* count of mapped objects...*/
	/* pointer to the var which is mapped to a pdo */
	/* void *     pMappedAppObject = NULL; */
	/* pointer fo the var which holds the mapping parameter of an mapping entry  */
	UNS32 *pMappingParameter = NULL;  
	/* pointer to the transmissiontype...*/
	UNS8  *pTransmissionType = NULL;  
	UNS32 *pwCobId = NULL;	
	UNS8  dataType;
	UNS16 index;
	UNS8  subIndex;
	UNS8  offset;
	UNS8  status;
	UNS8  sizeData;
	UNS32 objDict;	
	UNS16 offsetObjdict;
	UNS16 offsetObjdictMap;
	UNS16 endIndex;

	status = state3;
	pdoNum = 0x00;
	prp_j = 0x00;
	offset = 0x00;

	post_sync();

	/* only operational state allows PDO transmission */
	if( getState() != Operational ) 
		return 0;
  
	/* So, the node is in operational state */
	/* study all PDO stored in the objects dictionary */
	offsetObjdict = obj_firstIndex.PDO_TRS;
	endIndex = obj_lastIndex.PDO_TRS;
	offsetObjdictMap = obj_firstIndex.PDO_TRS_MAP;
  
	if(offsetObjdict) while( offsetObjdict <= endIndex) {  
		switch( status ) {
		case state3:    /* get the PDO transmission type */
			if (objdict[offsetObjdict].bSubCount <= 2) {
				return 0xFF;
			}
			pTransmissionType = objdict[offsetObjdict].pSubindex[2].pObject;    
			status = state4; 
			break;     
		case state4:	/* check if transmission type is after (this) SYNC */
			/* The message may not be transmited every SYNC but every n SYNC */      
			if( (*pTransmissionType >= TRANS_SYNC_MIN) && (*pTransmissionType <= TRANS_SYNC_MAX) &&
				(++count_sync[pdoNum] == *pTransmissionType) ) {	
				count_sync[pdoNum] = 0;
				status = state5;
				break;
			}
			else {
				pdoNum++;
				offsetObjdict++;
				offsetObjdictMap++;
				status = state11;
				break;
			}      
		case state5:	/* get PDO CobId */
			pwCobId = objdict[offsetObjdict].pSubindex[1].pObject;     
			status = state7;
			break;
		case state7:  /* get mapped objects number to transmit with this PDO */
			pMappingCount = objdict[offsetObjdictMap].pSubindex[0].pObject;
			status = state8;
		case state8:	/* get mapping parameters */
			pMappingParameter = objdict[offsetObjdictMap].pSubindex[prp_j + 1].pObject;
			status = state9;

		case state9:	/* get data to transmit */ 
			index = (UNS16)((*pMappingParameter) >> 16);
			subIndex = (UNS8)(( (*pMappingParameter) >> (UNS8)8 ) & (UNS32)0x000000FF);
			/* <<3 because in *pMappingParameter the size is in bits */
			sizeData = (UNS8) ((*pMappingParameter & (UNS32)0x000000FF) >> 3) ;

			objDict = getODentry(index, subIndex, (void *)&process_var.data[offset], &sizeData, &dataType, 0 ); 

			if( objDict != OD_SUCCESSFUL ){
				return 0xFF;
			}

			offset += sizeData ;
			process_var.count = offset;
			prp_j++;
			status = state10;
			break;

		case state10:	/* loop to get all the data to transmit */
			if( prp_j < *pMappingCount ){
				status = state8;
				break;
			}
			else {
				PDOmGR(*pwCobId );	
				pdoNum++;
				offsetObjdict++;
				offsetObjdictMap++;
				offset = 0x00;
				prp_j = 0x00;
				status = state11;
				break;
			}

		case state11:     
			status = state3;
			break;

		default:
			MSG_ERR(0x1019,"Unknown state has been reached : %d",status);
			return 0xFF;
		}/* end switch case */
	}/* end while( prp_i<dict_cstes.max_count_of_PDO_transmit ) */

	post_TPDO();

	return 0;
}

void post_sync(void)
{
}

void post_TPDO(void)
{
}

