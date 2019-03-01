
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

#include "data.h"

#include <stdlib.h>

forest_t* new_forest()
{
  forest_t* f = (forest_t*) malloc(sizeof(forest_t));
  if (f) {
    f->fid = 0;
    f->name = 0;
    f->left = 0;
    f->right = 0;
    f->counts_raw = 0;
    f->counts = 0;
  }
  return f;
}

void destroy(forest_t *f)
{
  if (0==f) return;

  free(f->name);
  free(f->counts_raw);

  /* For sanity */
  f->name = 0;
  f->counts_raw = 0;
  f->counts = 0;
}


update_t* free_list = 0;

update_t* new_update(int fid, int level, int delta, update_t* next)
{
  update_t* node = 0;
  if (free_list) {
    node = free_list;
    free_list = free_list->next;
  } else {
    node = (update_t*) malloc(sizeof(update_t));
  }
  node->fid = fid;
  node->level = level;
  node->delta = delta;
  node->next = next;
  return node;
}

void kill_update(update_t* node)
{
  while (node) {
    update_t* nxt = node->next;
    node->next = free_list;
    free_list = node;
    node = nxt;
  }
}


void init_screen_t(screen_t* SI, int maxF)
{
  if (0==SI) return;
  SI->F = (forest_t**) malloc(maxF * sizeof(forest_t*));
  SI->maxf = maxF;
  SI->nf = 0;
  for (int i=0; i<maxF; i++) SI->F[i] = 0;
  SI->has_primed = 0;
  SI->max_level = 0;
  SI->rows = 0;
  SI->cols = 0;
  SI->forwidth = 0;
  SI->varwidth = 0;
  SI->playing = 0;
}

void done_screen_t(screen_t* SI)
{
  if (0==SI) return;
  for (int i=0; i<SI->maxf; i++) {
    destroy(SI->F[i]);
    free(SI->F[i]);
  }
  free(SI->F);
}

void add_forest(screen_t* SI, forest_t* f)
{
  if (0==SI) return;
  if (f->fid < 0) return;
  if (f->fid >= SI->maxf) return;
  if (0==SI->F[f->fid]) {
    SI->nf++;
    SI->forwidth = SI->cols / SI->nf;
  }
  destroy(SI->F[f->fid]);
  SI->F[f->fid] = f;
  if (f->left < 0) SI->has_primed = 1;
  if (f->right > SI->max_level) {
    SI->max_level = f->right;
    SI->varwidth = 1;
    int x = SI->max_level;
    while (x>9) {
      x /= 10;
      SI->varwidth++;
    }
  }
}

