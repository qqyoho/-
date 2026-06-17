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
#include "lifegrd.h"
#include "can_open_timer.h"
#include "states.h"
#include "objdictdef.h"

extern UNS8 bDeviceNodeId;

/* NMT-heartbeat */
UNS8  toggle;
UNS8  ConsumerHeartbeatCount;
UNS32 *ConsumerHeartbeatEntries;
TIMER_HANDLE *ConsumerHeartBeatTimers;
UNS16 ProducerHeartBeatTime;
TIMER_HANDLE ProducerHeartBeatTimer;
heartbeatError_t heartbeatError;
e_nodeState NMTable[NMT_MAX_NODE_ID]; 

/* Prototypes for internals functions */
void ConsumerHearbeatAlarm(UNS32 id);
void ProducerHearbeatAlarm(UNS32 id);

/*****************************************************************************/
e_nodeState getNodeState(UNS8 nodeId)
{
	return NMTable[nodeId];
}

/*****************************************************************************/
/* The Consumer Timer Callback */
void ConsumerHearbeatAlarm(UNS32 id)
{
	/*MSG_WAR(0x00, "ConsumerHearbeatAlarm", 0x00);*/

	/* call heartbeat error with NodeId */
	(heartbeatError)((UNS8)( ((ConsumerHeartbeatEntries[id]) & (UNS32)0x00FF0000) >> (UNS8)16 ));
}

/*****************************************************************************/
void proceedNODE_GUARD(Message* m )
{
	UNS8 nodeId = (UNS8)GET_NODE_ID((*m));

	if((m->rtr == 1) ) /* Notice that only the master can have sent this node guarding request */
	{ /* Receiving a NMT NodeGuarding (request of the state by the master) */
		/*  only answer to the NMT NodeGuarding request, the master is not checked (not implemented) */
		if (nodeId == bDeviceNodeId)
		{
			Message msg;
			msg.cob_id.w = bDeviceNodeId + 0x700;
			msg.len = (UNS8)0x01;
			msg.rtr = 0;
			msg.data[0] = getState(); 
			if (toggle)
			{
				msg.data[0] |= 0x80 ;
				toggle = 0 ;
			}
			else
				toggle = 1 ; 
			/* send the nodeguard response. */
			can_std_transmit(msg.cob_id.w, msg.data, msg.rtr, msg.len);
		}  
	}
	else
    { /* Not a request CAN */
		MSG_WAR(0x3110, "Received NMT nodeId : ", nodeId);
		/* the slave's state receievd is stored in the NMTable */
		/* The state is stored on 7 bit */
		NMTable[nodeId] = (e_nodeState) ((*m).data[0] & 0x7F) ;

		/* Boot-Up frame reception */
		if ( NMTable[nodeId] == Initialisation )
		{
			/* The device send the boot-up message (Initialisation) */
			/* to indicate the master that it is entered in pre_operational mode */
			/* Because the  device enter automaticaly in pre_operational mode, */
			/* the pre_operational mode is stored */
			/* NMTable[bus_id][nodeId] = Pre_operational; */
			MSG_WAR(0x3100, "The NMT is a bootup from node : ", nodeId);
		}

		if(NMTable[nodeId] != Unknown_state) {
			UNS8 index, ConsummerHeartBeat_nodeId ;

			for( index = (UNS8)0x00; index < ConsumerHeartbeatCount; index++ )
			{
				ConsummerHeartBeat_nodeId = (UNS8)( ((ConsumerHeartbeatEntries[index]) & (UNS32)0x00FF0000) >> (UNS8)16 );
				if ( nodeId == ConsummerHeartBeat_nodeId )
				{
					TIMEVAL time = ( (ConsumerHeartbeatEntries[index]) & (UNS32)0x0000FFFF ) ;
					/* Renew alarm for next heartbeat. */
					DelAlarm(ConsumerHeartBeatTimers[index]);
					ConsumerHeartBeatTimers[index] = SetAlarm(index, &ConsumerHearbeatAlarm, MS_TO_TIMEVAL(time), 0);
				}
			}
		}
	}
}

/*****************************************************************************/
/* The Consumer Timer Callback */
void ProducerHearbeatAlarm(UNS32 id)
{
	if(ProducerHeartBeatTime)
	{
		Message msg;
		/* Time expired, the heartbeat must be sent immediately
		 * generate the correct node-id: this is done by the offset 1792
		 * (decimal) and additionaly
		 * the node-id of this device.
		 */
		msg.cob_id.w = bDeviceNodeId + 0x700;
		msg.len = (UNS8)0x01;
		msg.rtr = 0;
		msg.data[0] = getState(); /* No toggle for heartbeat !*/
		/* send the heartbeat */
  		can_std_transmit(msg.cob_id.w, msg.data, msg.rtr, msg.len);
	}
	else{
		ProducerHeartBeatTimer = DelAlarm(ProducerHeartBeatTimer);
	}
}

/*****************************************************************************/
void heartbeatInit(void)
{
    UNS8 index; /* Index to scan the table of heartbeat consumers */

    toggle = 0;

    for( index = (UNS8)0x00; index < ConsumerHeartbeatCount; index++ )
    {
		TIMEVAL time = (UNS16) (ConsumerHeartbeatEntries[index] & (UNS32)0x0000FFFF) ;
		/* MSG_WAR(0x3121, "should_time : ", should_time ) ; */
		if ( time )
		{
			ConsumerHeartBeatTimers[index] = SetAlarm(index, &ConsumerHearbeatAlarm, MS_TO_TIMEVAL(time), 0);
		}
    }

    if (ProducerHeartBeatTime)
    {
    	TIMEVAL time = ProducerHeartBeatTime;
    	ProducerHeartBeatTimer = SetAlarm(0, &ProducerHearbeatAlarm, MS_TO_TIMEVAL(time), MS_TO_TIMEVAL(time));
    }
}

/*****************************************************************************/
void heartbeatStop(void)
{
    UNS8 index;
    for( index = (UNS8)0x00; index < ConsumerHeartbeatCount; index++ )
    {
        ConsumerHeartBeatTimers[index + 1] = DelAlarm(ConsumerHeartBeatTimers[index + 1]);;
    }

    ProducerHeartBeatTimer = DelAlarm(ProducerHeartBeatTimer);;
}

void _heartbeatError(UNS8 heartbeatID)
{
}

