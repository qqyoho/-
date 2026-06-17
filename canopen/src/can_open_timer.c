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

#include <applicfg.h>
#include "can_open_timer.h"
#include "stdint.h"

#define MAX_NB_TIMER                 32

/*  ---------  The timer table --------- */
s_timer_entry timers[MAX_NB_TIMER] = {{TIMER_FREE, NULL, NULL, 0, 0, 0},};

static volatile uint8_t g_can_open_timer = 0;

/* 10ms定时器 */
void c_open_10ms_timer_run(void)
{
	if (g_can_open_timer) g_can_open_timer --;
}

/* ---------  Use this to declare a new alarm --------- */
TIMER_HANDLE SetAlarm(UNS32 id, TimerCallback_t callback, TIMEVAL value, TIMEVAL period)
{
	TIMER_HANDLE i;
	TIMER_HANDLE row_number = TIMER_NONE;

	/* in order to decide new timer setting we have to run over all timer rows */
	for(i=0; i < MAX_NB_TIMER; i++)
	{
		s_timer_entry *row = (timers+i);

		if (callback && 	/* if something to store */
		   row->state == TIMER_FREE) /* and empty row */
		{	/* just store */
			row->callback = callback;
			row->id = id;
			row->val = value;
			row->back_val = value;
			row->interval = period;
			row->state = TIMER_ARMED;
			row_number = i;
			break;
		}
	}
	
	if (row_number != TIMER_NONE)
	/* if successfull **/
	{
		return row_number;
	}
	return TIMER_NONE;
}

/* ---------  Use this to remove an alarm --------- */
TIMER_HANDLE DelAlarm(TIMER_HANDLE handle)
{
	/* Quick and dirty. system timer will continue to be trigged, but no action will be preformed. */
	MSG_WAR(0x3320, "DelAlarm. handle = ", handle);

	/* in order to decide new timer setting we have to run over all timer rows */
	if(handle != TIMER_NONE && handle < MAX_NB_TIMER)
	{
		timers[handle].state = TIMER_FREE; 		
	}

	return TIMER_NONE;
}

/* ---------  TimeDispatch is called on each timer expiration --------- */
void TimeDispatch()
{
	TIMER_HANDLE i;

	if (g_can_open_timer) return;
	g_can_open_timer = 1;

	for(i = 0; i < MAX_NB_TIMER; i ++)
	{
		s_timer_entry *row = (timers + i);

		if (row->state & TIMER_ARMED) /* if row is active */
		{
			if ( row->val > 0 ) row->val --;
			if ( row->val == 0 )
			{
				if ( row->interval )
				{
					row->val = row->back_val;
				}
				else
				{
					row->state = TIMER_FREE;
				}
				if(row->callback)
					(*row->callback)(row->id);
			}
		}
	}
}
