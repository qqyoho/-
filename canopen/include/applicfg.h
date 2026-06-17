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

#ifndef __APPLICFG__
#define __APPLICFG__

#include <string.h>
#include <stdio.h>

/// Define the architecture : little_endian or big_endian
// -----------------------------------------------------
// Test :
// UNS32 v = 0x1234ABCD;
// char *data = &v;
//
// Result for a little_endian architecture :
// data[0] = 0xCD;
// data[1] = 0xAB;
// data[2] = 0x34;
// data[3] = 0x12;
//
// Result for a big_endian architecture :
// data[0] = 0x12;
// data[1] = 0x34;
// data[2] = 0xAB;
// data[3] = 0xCD;

#if 0
/* CANOPEN_BIG_ENDIAN now defined in config.h*/
#ifndef CANOPEN_BIG_ENDIAN
#define CANOPEN_BIG_ENDIAN 0
#endif
#endif

// Integers
#define INTEGER8 signed char
#define INTEGER16 short
#define INTEGER32 long
 
// Unsigned integers
#define UNS8   unsigned char
#define UNS16  unsigned short
#define UNS32  unsigned long

// Whatever your microcontroller, the timer wont work if 
// TIMEVAL is not at least on 32 bits
#define TIMEVAL UNS32 

// The timer of the hcs12 counts from 0000 to 0xFFFF
#define TIMEVAL_MAX 0xFFFF

// The timer is incrementing every 4 us.
#define MS_TO_TIMEVAL(ms) (ms / 10)
#define US_TO_TIMEVAL(us) (us>>2)

#define MSG_ERR(num, str, val)
#define MSG_WAR(num, str, val)

#endif

