
/* $Id$ */

/*
    termview: terminal viewing utility for Meddly trace data.
    Copyright (C) 2015, Iowa State University Research Foundation, Inc.

    This library is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by 
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along 
    with this software.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SCREEN_H
#define SCREEN_H

#include "data.h"

/**
  Initialize the screen.
    @return   1 on success, 0 on error
*/
int open_screen(screen_t* S);

/**
  Close the screen.
*/
void close_screen();

/**
  Update keystroke area ofscreen.
  Call whenever a key is pressed.
*/
void update_keys(screen_t* S);

/**
  Re-draw forest windows.
  Call this when a forest is added.
*/
void update_windows(screen_t* S);

/**
  Update screen with phase information.
    @param  S     Screen info
    @param  fid   Raw forest id
    @param  str   String - next phase of computation.
*/
void update_p(screen_t* S, int fid, const char* str);

/**
  Get a keystroke.
    @param  block   Should we wait until a key is pressed?

    @return   0, if no key was pressed
              the character pressed, otherwise.
*/
int keystroke(char block);


/**
  Update screen due to an 'a' record.
    @param  S     Screen information.  Will be changed.
    @param  list  List of updates to apply.
*/
void update_a(screen_t* S, update_t* list);

#endif
