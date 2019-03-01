
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

#ifndef DATA_H
#define DATA_H

/*
  Struct for forest information
*/
typedef struct {
  int fid;
  char* name;
  int left;
  int right;
  int* counts_raw;
  int* counts;
} forest_t;

/*
  Allocate a new forest_t struct, and initialize it to zeroes.
*/
forest_t* new_forest();

/*
  Free the memory used by a forest_t struct.
  Does not free the pointer to the forest_t itself.
*/
void destroy(forest_t *f);


/*
  Struct for a list of count updates
*/
struct update {
  int fid;
  int level;
  int delta;
  struct update* next;
};

typedef struct update update_t;

/*
  Make a new update struct.
  (List of length 1).
*/
update_t* new_update(int fid, int level, int delta, update_t* next);

/*
  Destroy an update list.
*/
void kill_update(update_t*);


/*
  Struct for screen information
*/
typedef struct {
  forest_t** F;
  int nf;
  int maxf;
  char has_primed;
  int max_level;
  int rows;
  int cols;
  int forwidth;
  int varwidth;
  char playing;
} screen_t;

/*
  Initialize a screen_info struct
*/
void init_screen_t(screen_t* S, int maxF);

/*
  Destroy a screen info struct
*/
void done_screen_t(screen_t* S);

/*
  Add a forest to a screen info struct
*/
void add_forest(screen_t* S, forest_t* f);


#endif
