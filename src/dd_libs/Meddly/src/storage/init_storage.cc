
// $Id: init_builtin.cc 700 2016-07-07 21:06:50Z asminer $

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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "../defines.h"
#include "init_storage.h"

#include "old_scheme.h"
#include "simple.h"
#include "compact.h"

namespace MEDDLY {
  const node_storage_style* CLASSIC_STORAGE = 0;

  const node_storage_style* SIMPLE_GRID = 0;
  const node_storage_style* SIMPLE_ARRAY = 0;
  const node_storage_style* SIMPLE_HEAP = 0;
  const node_storage_style* SIMPLE_NONE = 0;

  const node_storage_style* COMPACT_GRID = 0;
};




MEDDLY::storage_initializer::storage_initializer(initializer_list *p)
 : initializer_list(p)
{
  classic = 0;

  simple_grid = 0;
  simple_array = 0;
  simple_heap = 0;
  simple_none = 0;

  compact_grid = 0;
}

void MEDDLY::storage_initializer::setup()
{
  CLASSIC_STORAGE = (classic = new old_node_storage_style);

  SIMPLE_GRID  = (simple_grid  = new simple_grid_style);
  SIMPLE_ARRAY = (simple_array = new simple_array_style);
  SIMPLE_HEAP  = (simple_heap  = new simple_heap_style);
  SIMPLE_NONE  = (simple_none  = new simple_none_style);

  COMPACT_GRID = (compact_grid = new compact_grid_style);
}

void MEDDLY::storage_initializer::cleanup()
{
  delete classic;
  CLASSIC_STORAGE = (classic = 0);

  delete simple_grid;
  delete simple_array;
  delete simple_heap;
  delete simple_none;

  SIMPLE_GRID  = (simple_grid  = 0);
  SIMPLE_ARRAY = (simple_array = 0);
  SIMPLE_HEAP  = (simple_heap  = 0);
  SIMPLE_NONE  = (simple_none  = 0);


  delete compact_grid;
  COMPACT_GRID = (compact_grid = 0);
}

