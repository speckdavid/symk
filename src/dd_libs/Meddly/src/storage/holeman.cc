
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

#include "holeman.h"

// ******************************************************************
// *                                                                *
// *                        holeman  methods                        *
// *                                                                *
// ******************************************************************

MEDDLY::holeman::holeman(int smallestHole)
{
  parent = 0;
  num_holes = 0;
  max_holes = 0;
  num_untracked = 0;
  hole_slots = 0;
  max_hole_slots = 0;
  untracked_slots = 0;
  last_slot = 0;
  data = 0;
  size = 0;
  smallest = smallestHole;
}

// ******************************************************************

MEDDLY::holeman::~holeman()
{
  // don't delete our parent
  decMemAlloc(size*sizeof(node_handle));
  free(data);
}

// ******************************************************************

MEDDLY::node_address MEDDLY::holeman::chunkAfterHole(node_address addr) const
{
  MEDDLY_DCASSERT(data);
  MEDDLY_DCASSERT(data[addr]<0);
  MEDDLY_DCASSERT(data[addr-data[addr]-1] == data[addr]);
  return addr - data[addr];
}

// ******************************************************************
void MEDDLY::holeman::dumpInternalTail(output &) const
{
}

// ******************************************************************

void MEDDLY::holeman
::reportStats(output &s, const char* pad, unsigned flags) const
{
  if (flags & expert_forest::HOLE_MANAGER_STATS) {
    bool human_readable = flags & expert_forest::HUMAN_READABLE_MEMORY;
    s << pad << "    " << num_holes << " holes currently\n";
    if (num_untracked) {
      s << pad << "        (" << num_untracked << " untracked, " 
        << num_holes-num_untracked << " tracked)\n";
    }
    s << pad << "    " << max_holes << " max holes seen\n";

    unsigned long holemem = hole_slots * sizeof(node_handle);
    s << pad << "    ";
    s.put_mem(holemem, human_readable);
    s << " wasted in holes (total)\n";
    unsigned long untrackedmem = untracked_slots * sizeof(node_handle);
    if (untrackedmem) {
      s << pad << "        (";
      s.put_mem(untrackedmem, human_readable);
      s << " untracked, ";
      s.put_mem(holemem-untrackedmem, human_readable);
      s << " tracked)\n";
    }

    holemem = max_hole_slots * sizeof(node_handle);
    s << pad << "    ";
    s.put_mem(holemem, human_readable);
    s << pad << " max in holes\n";
  }
}

// ******************************************************************

void MEDDLY::holeman::clearHolesAndShrink(node_address new_last, bool shrink)
{
  last_slot = new_last;
  num_holes = 0;
  hole_slots = 0;
  num_untracked = 0;
  untracked_slots = 0;

  if (shrink && size > min_size && last_slot < size/2) {
    node_address new_size = size/2;
    while (new_size > min_size && new_size > last_slot * 3) { new_size /= 2; }
    resize(new_size);
  }
}

// ******************************************************************

void MEDDLY::holeman::resize(node_address new_slots)
{
  node_handle* new_data 
    = (node_handle*) realloc(data, new_slots * sizeof(node_handle));

  if (0 == new_data) {
    fprintf(stderr,
        "Error in allocating array of size %lu at %s, line %d\n",
        new_slots * sizeof(node_handle), __FILE__, __LINE__);
    throw error(error::INSUFFICIENT_MEMORY);
  }
  if (0 == data) new_data[0] = 0;
  if (new_slots > size) {
    incMemAlloc((new_slots - size) * sizeof(node_handle));
  } else {
    decMemAlloc((size - new_slots) * sizeof(node_handle));
  }
#ifdef MEMORY_TRACE
    printf("Resized data[]. Old size: %ld, New size: %ld, Last: %ld.\n", 
      size, new_slots, last
    );
#endif
  size = new_slots;
  if (data != new_data) {
    // update pointers
    data = new_data;
    parent->updateData(data);
  }
}

