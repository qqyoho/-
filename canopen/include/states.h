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

#ifndef __states_h__
#define __states_h__

#include <applicfg.h>
#include "data.h"

/* The nodes states 
 * -----------------
 * values are choosen so, that they can be sent directly
 * for heartbeat messages...
 * Must be coded on 7 bits only
 * */
/* Should not be modified */
enum enum_nodeState {
  Initialisation  = 0x00, 
  Disconnected    = 0x01,
  Connecting      = 0x02,
  Preparing       = 0x02,
  Stopped         = 0x04,
  Operational     = 0x05,
  Pre_operational = 0x7F,
  Unknown_state   = 0x0F
};

typedef enum enum_nodeState e_nodeState;

typedef struct
{
	UNS8 csBoot_Up;
	UNS8 csSDO;
	UNS8 csEmergency;
	UNS8 csSYNC;
	UNS8 csHeartbeat;
	UNS8 csPDO;
} s_state_communication;

/************************* prototypes ******************************/

/** Called by driver/app when receiving messages
*/
void canDispatch(Message *m);

/** Returns the state of the node 
*/
e_nodeState getState(void);

/** Change the state of the node 
*/
UNS8 setState(e_nodeState newState);

/** Define the node ID. Initialize the object dictionary
*/
void setNodeId(UNS8 nodeId);

/** Some stuff to do when the node enter in reset mode
 *
 */
/* void initResetMode (CO_Data* d); */


#endif

