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

#include "states.h"
#include "def.h"
#include "lifegrd.h"
#include "nmtSlave.h"
#include "objdictdef.h"
#include "sync.h"
#include "pdo.h"
#include "sdo.h"
#include "can_open_timer.h"
#include "objacces.h"

/** Function that user app can overload
 * 
 */
static void initialisation(void);
static void preOperational(void);
static void operational(void);
static void stopped(void);

extern UNS8 bDeviceNodeId;
extern UNS8 *iam_a_slave;
extern const dict_cste dict_cstes;

/* State machine */
e_nodeState nodeState;
s_state_communication CurrentCommunicationState;

/* Prototypes for internals functions */
void switchCommunicationState(s_state_communication *newCommunicationState);

/*****************************************************************************/
e_nodeState getState(void)
{
	return nodeState;
}

/*=============================================================
* 函数名称：objdict_init
* 函数功能：对象字典初始化
* 参数个数：0
* 函数参数：
* 返回值：  
*           无
* 修改记录：
*===============================================================
* 日期                修改人             修改内容
* 2018-06-25          戴辉发             创建
==============================================================*/
void objdict_init(void)
{
}

/*****************************************************************************/
void canDispatch(Message *m)
{
	switch(m->cob_id.w >> 7)
	{
	case SYNC:
		if(CurrentCommunicationState.csSYNC)
			proceedSYNC(m);
		break;
	/* case TIME_STAMP: */
	case PDO1tx:
	case PDO1rx:
	case PDO2tx:
	case PDO2rx:
	case PDO3tx:
	case PDO3rx:
	case PDO4tx:
	case PDO4rx:
		if (CurrentCommunicationState.csPDO)
			proceedPDO(m);
		break;
	case SDOtx:
	case SDOrx:
		if (CurrentCommunicationState.csSDO)
			proceedSDO(m);
		break;
	case NODE_GUARD:
		if (CurrentCommunicationState.csHeartbeat)
			proceedNODE_GUARD(m);
		break;
	case NMT:
		if (*iam_a_slave)
		{
			proceedNMTstateChange(m);
		}
	}
}

#define None
#define StartOrStop(CommType, FuncStart, FuncStop) \
	if(newCommunicationState->CommType && !CurrentCommunicationState.CommType){\
		CurrentCommunicationState.CommType = 1;\
		FuncStart;\
	}\
	else if(!newCommunicationState->CommType && CurrentCommunicationState.CommType){\
		CurrentCommunicationState.CommType = 0;\
		FuncStop;\
	}
	
/*****************************************************************************/
void switchCommunicationState(s_state_communication *newCommunicationState)
{
	StartOrStop(csSDO, None, resetSDO())
	StartOrStop(csSYNC,	startSYNC(), stopSYNC())
	StartOrStop(csHeartbeat, heartbeatInit(), heartbeatStop())
	/* StartOrStop(Emergency,,) */
	StartOrStop(csPDO, startTPDOEvent(), stopTPDOEvent())
	StartOrStop(csBoot_Up, None, slaveSendBootUp())
}

/*****************************************************************************/
UNS8 setState(e_nodeState newState)
{
	while(newState != nodeState){
		switch( newState ){
		case Initialisation:
			{
				s_state_communication newCommunicationState = {1, 0, 0, 0, 0, 0};

				/* This will force a second loop for the state switch */
				nodeState = Initialisation;
				newState = Pre_operational;
				switchCommunicationState(&newCommunicationState);
				/* call user app related state func. */
				initialisation();
			}
			break;

		case Pre_operational:
			{
				s_state_communication newCommunicationState = {0, 1, 1, 1, 1, 0};

				nodeState = Pre_operational;
				newState = Pre_operational;
				switchCommunicationState(&newCommunicationState);
				preOperational();
			}
			break;

		case Operational:
			if(nodeState == Initialisation) return 0xFF;
			{
				s_state_communication newCommunicationState = {0, 1, 1, 1, 1, 1};

				nodeState = Operational;
				newState = Operational;
				switchCommunicationState(&newCommunicationState);
				operational();
			}
			break;

		case Stopped:
			if(nodeState == Initialisation) return 0xFF;
			{
				s_state_communication newCommunicationState = {0, 0, 0, 0, 1, 0};

				nodeState = Stopped;
				newState = Stopped;
				switchCommunicationState(&newCommunicationState);
				stopped();
			}
			break;

		default:
			return 0xFF;
		}/* end switch case */
	}

	return 0;
}

/*****************************************************************************/
void setNodeId(UNS8 nodeId)
{
	UNS32 pbData32;
	UNS16 i;
	UNS8  dataSize;

	// bDeviceNodeId is defined in the object dictionary.
	bDeviceNodeId = nodeId;

	dataSize = 4;
	// ** Initialize the server(s) SDO parameters
	// Remember that only one SDO server is allowed, defined at index 0x1200
	pbData32 = 0x600 + nodeId;
	setODentry(0x1200, 1, &pbData32, &dataSize, 0); // Subindex 1
	pbData32 = 0x580 + nodeId;
	setODentry(0x1200, 2, &pbData32, &dataSize, 0); // Subindex 2

	dataSize = 1;
	// Subindex 3 : node Id client. As we do not know the value, we put the node Id Server
	setODentry(0x1200, 3, &nodeId, &dataSize, 0);
	/* ** Initialize the server(s) SDO parameters */
	/* Remember that only one SDO server is allowed, defined at index 0x1200 */

	/* ** Initialize the client(s) SDO parameters  */

	/* Nothing to initialize (no default values required by the DS 401) */
	// ** Initialize the receive PDO communication parameters. Only for 0x1400 to 0x1403
	{
		UNS32 cobID[] = {0x200, 0x300, 0x400, 0x500};
		dataSize = 4;
		for (i = 0; (i < dict_cstes.max_count_of_PDO_receive)&&(i < 4); i++) {
			pbData32 = cobID[i] + nodeId;
			setODentry(0x1400 + i, 1, &pbData32, &dataSize, 0); // Subindex 1
		}
	}

	// ** Initialize the transmit PDO communication parameters. Only for 0x1800 to 0x1803
	{
		UNS32 cobID[] = {0x180, 0x280, 0x380, 0x480};

		dataSize = 4;

		for (i = 0; (i < dict_cstes.max_count_of_PDO_transmit)&&(i < 4); i++) {
			pbData32 = cobID[i] + nodeId;
			setODentry(0x1800 + i, 1, &pbData32, &dataSize, 0);// Subindex 1
		}
	}

	/* bDeviceNodeId is defined in the object dictionary. */
	bDeviceNodeId = nodeId;

	setState(Pre_operational);
	setState(Operational);
}

void initialisation(void)
{
}

void preOperational(void)
{
}

void operational(void)
{
}

void stopped(void)
{
}

