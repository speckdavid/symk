
// $Id$

/*
    Meddly: Multi-terminal and Edge-valued Decision Diagram LibrarY.
    Copyright (C) 2009, Iowa State University Research Foundation, Inc.

    This library is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published 
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/



/*
 * \brief A timer class.
 *
 * Timer is started when a timer object is initialized.
 * note_time() is used to note the current time.
 * get_last_interval() is the time between the last two instances when the
 * time was noted (in microseconds)
 *
 * */

#ifndef TIMERS_H
#define TIMERS_H

#include "defines.h"
#include <sys/time.h>
#include <time.h>

class timer 
{
	struct timeval curr_time, prev_time;
	struct timezone time_zone;
	long last_interval; 

public:
	timer()
	{
		gettimeofday(&curr_time, &time_zone);
		prev_time = curr_time;
	}

	inline void note_time()
	{
		gettimeofday(&curr_time, &time_zone);
		last_interval = (curr_time.tv_sec - prev_time.tv_sec) * 1000000;
		last_interval += curr_time.tv_usec - prev_time.tv_usec;
		prev_time = curr_time;
	}

	inline long get_last_interval()
	{
		return last_interval;
	}
};

#endif

