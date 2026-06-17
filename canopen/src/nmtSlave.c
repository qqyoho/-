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

#include "nmtSlave.h"
#include "states.h"
#include "def.h"
#include "can.h"

extern UNS8 bDeviceNodeId;

/*******************************************************************)*********/
/* put the slave in the state wanted by the master */	
void proceedNMTstateChange(Message *m)
{
	if( getState() == Pre_operational ||
		getState() == Operational ||
		getState() == Stopped ) {

		/* Check if this NMT-message is for this node */
		/* byte 1 = 0 : all the nodes are concerned (broadcast) */
		if( ( (*m).data[1] == 0 ) || ( (*m).data[1] == bDeviceNodeId ) ){
			switch( (*m).data[0]){ /* command specifier (cs) */			
			case NMT_Start_Node:
				if ( (getState() == Pre_operational) || (getState() == Stopped) )
					setState(Operational);
				break; 

			case NMT_Stop_Node:
				if (getState() == Pre_operational ||
					getState() == Operational )
					setState(Stopped);
				break;

			case NMT_Enter_PreOperational:
				if (getState() == Operational || getState() == Stopped )
					setState(Pre_operational);
				break;

			case NMT_Reset_Node:
				setState(Initialisation);
				break;

			case NMT_Reset_Comunication:
				setState(Initialisation);
				break;
			}/* end switch */
		}/* end if( ( (*m).data[1] == 0 ) || ( (*m).data[1] == bDeviceNodeId ) ) */
	}
}

/*****************************************************************************/
UNS8 slaveSendBootUp(void)
{
	Message m;

	/* message configuration */
	m.cob_id.w = NODE_GUARD << 7 | bDeviceNodeId;
	m.rtr = NOT_A_REQUEST;
	m.len = 1;
	m.data[0] = 0x00;

	return can_std_transmit(m.cob_id.w, m.data, m.rtr, m.len);
}

