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
#include "can_open_timer.h"

#include "pdo.h"
#include "objacces.h"
#include "objdictdef.h"
#include "states.h"

/* PDO */
s_process_var process_var;
TIMER_HANDLE tpdo_timer[4];

extern const dict_cste dict_cstes;

/****************************************************************************/
UNS8 sendPDO(s_PDO pdo, UNS8 req)
{
	if( getState() == Operational ) {
		Message m;

		/* Message copy for sending */
		m.cob_id.w = pdo.cobId & 0x7FF; /* Because the cobId is 11 bytes length */
		if (req == NOT_A_REQUEST) {
			m.rtr = NOT_A_REQUEST;
			m.len = pdo.len;
			memcpy(&m.data, &pdo.data, m.len);
		}
		else {
			m.rtr = REQUEST;
			m.len = 0;
		}

		/* return canSend(d->canHandle,&m); */
		return can_std_transmit(m.cob_id.w, m.data, m.rtr, m.len);
	} /* end if */
	return 0xFF;
}

/***************************************************************************/
UNS8 PDOmGR(UNS32 cobId) /* PDO Manager */
{
	UNS8 res;
	UNS8 i;
	s_PDO pdo;

	/* if PDO is waiting for transmission,
	   preparation of the message to send */
	pdo.cobId = cobId;
	pdo.len =  process_var.count;
	/* memcpy(&(pdo.data), &(process_var.data), pdo.len); */
	/* Ce memcpy devrait 阾re portable */
	for ( i = 0 ; i < pdo.len ; i++) 
		pdo.data[i] = process_var.data[i];

	res = sendPDO(pdo, NOT_A_REQUEST);

	return res;
}

UNS8 buildPDO(UNS16 index, UNS32 *pwCobId)
{
	UNS32 *pMapIndex;
	void  *pMappedAppObject;
	UNS8  subInd;
	UNS8  size, dataType;
	UNS8  offset, flag = 0xff;

	subInd = 0x00;
	offset = 0x00;

	/* only operational state allows PDO transmission */
	if( getState() != Operational )
	{
		return 0xFF;
	}
	while(1)
	{
        size = 0;
		if(OD_SUCCESSFUL == getODentry(index, subInd, (void * *)&pMapIndex, &size, &dataType, 0))
		{
            size = 0;
			if(OD_SUCCESSFUL == getODentry(((*pMapIndex) >> 16) & 0xffff, 
				((*pMapIndex) >> 8) & 0xff, (void **)&pMappedAppObject, &size, &dataType, 0))
			{
				memcpy(&process_var.data[offset], pMappedAppObject, size);

#ifdef CANOPEN_BIG_ENDIAN
				{
					// data must be transmited with low byte first
					UNS8 pivot, i;
					UNS8 sizeData = ((*pMapIndex) & 0xff) >> 3;// in bytes

					for ( i = 0 ; i < (sizeData >> 1)  ; i++) {
						pivot = process_var.data[offset + (sizeData - 1) - i];
						process_var.data[offset + (sizeData - 1) - i] = process_var.data[offset + i];
						process_var.data[offset + i] = pivot;
					}
				}
#endif
				offset += (((*pMapIndex) & 0xff) >> 3);
				flag = 0;
				process_var.count = offset;
			}
			else
			{
				break;
			}
		}
		else
		{
			break;
		}
        subInd += 1;
	}

	return flag;
}

/**************************************************************************/
UNS8 sendPDOrequest(UNS32 cobId )
{		
	UNS32 *pwCobId;	
	UNS8  i, err, size, dataType;

	/* Sending the request only if the cobid have been found on the PDO receive */
	/* part dictionary */
	for (i = 0; (i < dict_cstes.max_count_of_PDO_transmit)&&(i < 4); i++)
	{
		if (OD_SUCCESSFUL != getODentry(0x1800, 1, (void * *)&pwCobId, &size, &dataType, 0))
		{
			if ( *pwCobId  == cobId )
			{
				s_PDO pdo;
				pdo.cobId = *pwCobId;
				pdo.len = 0;
				err  = sendPDO(pdo, REQUEST);
				return err;
			} 
		}
	}

	return 0xFF;
}

/***********************************************************************/
UNS8 proceedPDO(Message *m)
{		
	UNS8  numMap;/* Number of the mapped varable */                      
	UNS8  i;
	UNS32 *pwCobId = NULL;
	UNS8  Size;
	UNS8  dataType;
	UNS8  offset;
	UNS16 offsetObjdict;

	if((*m).rtr == NOT_A_REQUEST ) {/* The PDO received is not a request. */
		UNS8 flag = 0;

		/* memcpy(&(process_var.data), &m->data, (*m).len); */
		for( i = 0 ; i < m->len ; i++) 
			process_var.data[i] = m->data[i];
		process_var.count = (*m).len;

		offsetObjdict = 0x1400;
		/* 第一步查找对应的COBID */
		while(offsetObjdict  <= dict_cstes.transmit_PDO_last)
		{
			/* study of all PDO stored in the objects dictionary */
			/* get CobId of the dictionary which match to the received PDO */
			Size = 0; 
			if (OD_SUCCESSFUL == getODentry(offsetObjdict, 1, (void * *)&pwCobId, &Size, &dataType, 0))
			{
				if(*pwCobId == (*m).cob_id.w)
				{
					flag = 1;
					break;
				}
			}
			offsetObjdict += 1;
		}

		if(flag == 1)
		{
			UNS32 *pMapIndex;

			flag = 0;
			offsetObjdict -= 0x1400;
			offsetObjdict += 0x1600;
			numMap = 0;
			offset = 1;
			while(1)
			{
				Size = 0;
				/* 获取对应TXPAO第一个地址映射 */
				if(OD_SUCCESSFUL == getODentry(offsetObjdict, numMap, (void * *)&pMapIndex, &Size, &dataType, 0))
				/* 找到对应地址映射 */
				{
					if(*pMapIndex != 0)
					/* 第一个地址有效 */
					{
						/* 设置地址对应参数里的数据 */
						if(OD_SUCCESSFUL == setODentry(((*pMapIndex) >> 16) & 0xffff, 
							((*pMapIndex) >> 8) & 0xff, 
							(void *)&process_var.data[offset], &Size, 0))
						/* 获取到参数 */
						{
							flag = 1;
							offset += ((*pMapIndex & 0xff) >> 8);
						}
						else
						{
							break;
						}
					}
				}
				else
				{
					break;
				}
				numMap += 1;
			}
		}
	}/* end if Donnees */
	else if ((*m).rtr == REQUEST ){
		UNS8 flag = 0;

		offsetObjdict = 0x1800;
		/* 第一步查找对应的COBID */
		while(offsetObjdict  <= dict_cstes.transmit_PDO_last)
		{
			/* study of all PDO stored in the objects dictionary */
			/* get CobId of the dictionary which match to the received PDO */
			Size = 0; 
			if (OD_SUCCESSFUL == getODentry(offsetObjdict, 1, (void * *)&pwCobId, &Size, &dataType, 0))
			{
				if(*pwCobId == (*m).cob_id.w)
				{
					flag = 1;
					break;
				}
			}
			offsetObjdict += 1;
		}
		if(flag == 1)
		{
			UNS32 *pMapIndex;

			flag = 0;
			offsetObjdict -= 0x1800;
			offsetObjdict += 0x1A00;
			Size = 0;
			/* 获取对应TXPAO第一个地址映射 */
			if(OD_SUCCESSFUL == getODentry(offsetObjdict, 0, (void * *)&pMapIndex, &Size, &dataType, 0))
			/* 找到对应地址映射 */
			{
				if(*pMapIndex != 0)
				/* 第一个地址有效 */
				{
					UNS8 *pMapParameter;

					Size = 0;
					/* 获取地址对应参数里的数据 */
					if(OD_SUCCESSFUL == getODentry(((*pMapIndex) >> 16) & 0xffff, 
						((*pMapIndex) >> 8) & 0xff, 
						(void * *)&pMapParameter, &Size, &dataType, 0))
					/* 获取到参数 */
					{
						flag = 1;
					}
				}
			}
		}
	    if(flag == 1)
	    {
	        if(0 == buildPDO(offset, pwCobId))
	        {
	            PDOmGR(*pwCobId);
	        }
	    }
	}/* end if Requete */

	return 0;
}

/*********************************************************************/
void sendPDOevent(UNS32 cobID)
{
	UNS32 *pwCobId;
	UNS8  size, flag = 0;
	UNS8  dataType;
	UNS16 offset;

	size = 0;
	offset = 0x1800;
	// look for the index and subindex where the variable is mapped
	// Then, send the pdo which contains the variable.
	while(offset  <= dict_cstes.transmit_PDO_last)
	{
		if(OD_SUCCESSFUL == getODentry(offset, 1, (void * *)&pwCobId, &size, &dataType, 0))
		{
			if(*pwCobId == cobID)
			{
				flag = 1;
				break;
			}
		}
		offset += 1;
	}
	if(flag == 1)
	{
		UNS32 *pMapIndex;

		flag = 0;
		offset -= 0x1800;
		offset += 0x1A00;
		size = 0;
		/* 获取对应TXPAO第一个地址映射 */
		if(OD_SUCCESSFUL == getODentry(offset, 0, (void * *)&pMapIndex, &size, &dataType, 0))
		/* 找到对应地址映射 */
		{
			if(*pMapIndex != 0)
			/* 第一个地址有效 */
			{
				UNS8 *pMapParameter;
				size = 0;
				/* 获取地址对应参数里的数据 */
				if(OD_SUCCESSFUL == getODentry(((*pMapIndex) >> 16) & 0xffff, 
					((*pMapIndex) >> 8) & 0xff, 
					(void * *)&pMapParameter, &size, &dataType, 0))
				/* 获取到参数 */
				{
					flag = 1;
				}
			}
		}
	}
    if(flag == 1)
    {
        if(0 == buildPDO(offset, pwCobId))
        {
            PDOmGR(*pwCobId);
        }
    }
}

void startTPDOEvent(void)
{
	UNS32 *pwCobId;
	UNS16 *time_period;
	UNS8  size, flag = 0;
	UNS8  dataType;
	UNS16 offset;

	size = 0;
	offset = 0;
	// look for the index and subindex where the variable is mapped
	// Then, send the pdo which contains the variable.
	while(offset  <= dict_cstes.transmit_PDO_last)
	{
		flag = 0;
		size = 0;
		if(OD_SUCCESSFUL == getODentry(0x1800 + offset, 1, (void * *)&pwCobId, &size, &dataType, 0))
		{
			UNS32 *pMapIndex;

			size = 0;
			if(OD_SUCCESSFUL == getODentry(0x1800 + offset, 3, (void * *)&time_period, &size, &dataType, 0))
			{
				/* 获取对应TXPAO第一个地址映射 */
				size = 0;
				if(OD_SUCCESSFUL == getODentry(0x1A00 + offset, 0, (void * *)&pMapIndex, &size, &dataType, 0))
				/* 找到对应地址映射 */
				{
					if(*pMapIndex != 0)
					/* 第一个地址有效 */
					{
						UNS8 *pMapParameter;
						size = 0;
						/* 获取地址对应参数里的数据 */
						if(OD_SUCCESSFUL == getODentry(((*pMapIndex) >> 16) & 0xffff, 
							((*pMapIndex) >> 8) & 0xff, 
							(void * *)&pMapParameter, &size, &dataType, 0))
						/* 获取到参数 */
						{
							flag = 1;
						}
					}
				}
			}
		}
		if(flag == 1)
		{
			tpdo_timer[offset] = SetAlarm(*pwCobId, sendPDOevent, *time_period, 0/*1*/);/* 0x2F0 */
		}
		offset += 1;
	}
	return;
}

void stopTPDOEvent(void)
{
    UNS16 offset;

    offset = 0;

    while((0x1800 + offset) <= dict_cstes.transmit_PDO_last)
    {
        if(tpdo_timer[offset] != TIMER_NONE)
        {
            tpdo_timer[offset] = DelAlarm(tpdo_timer[offset]);
        }
        offset += 1;
    }
}
