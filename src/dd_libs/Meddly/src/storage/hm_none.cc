
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



// TODO: Testing

#include "hm_none.h"


// #define MEMORY_TRACE


// ******************************************************************
// *                                                                *
// *                                                                *
// *                        hm_none  methods                        *
// *                                                                *
// *                                                                *
// ******************************************************************

MEDDLY::hm_none::hm_none() : holeman(2)
{
}

// ******************************************************************

MEDDLY::hm_none::~hm_none()
{
}

// ******************************************************************

MEDDLY::node_address MEDDLY::hm_none::requestChunk(int slots)
{
#ifdef MEMORY_TRACE
  printf("Requesting %d slots\n", slots);
#endif
  return allocFromEnd(slots);
}

// ******************************************************************

void MEDDLY::hm_none::recycleChunk(node_address addr, int slots)
{
#ifdef MEMORY_TRACE
  printf("Calling recycleChunk(%ld, %d)\n", long(addr), slots);
#endif

  decMemUsed(slots * sizeof(node_handle));
  newUntracked(slots);

  data[addr] = data[addr+slots-1] = -slots;

  // absorb as much as we can at the end
  while (data[lastSlot()]<0) {
    // there's a hole, grab it
    slots = -data[lastSlot()];
    addr = lastSlot() - slots + 1;
#ifdef MEMORY_TRACE
    printf("Absorbing end chunk\n\tlast %ld addr %ld size %d\n", 
      long(lastSlot()), long(addr), slots
    );
#endif
    releaseToEnd(slots);
    useUntracked(slots);
  }
}

// ******************************************************************

void MEDDLY::hm_none::dumpInternalInfo(output &s) const
{
  s << "Last slot used: " << long(lastSlot()) << "\n";
  s << "Total hole slots: " << holeSlots() << "\n";
}

// ******************************************************************

void MEDDLY::hm_none::dumpHole(output &s, node_address a) const
{
  MEDDLY_DCASSERT(data);
  MEDDLY_CHECK_RANGE(1, a, lastSlot());
  long aN = chunkAfterHole(a)-1;
  s << '[' << long(data[a]) << ", ..., " << long(data[aN]) << "]\n";
}

// ******************************************************************

void MEDDLY::hm_none
::reportStats(output &s, const char* pad, unsigned flags) const
{
  static unsigned HOLE_MANAGER =
    expert_forest::HOLE_MANAGER_STATS | expert_forest::HOLE_MANAGER_DETAILED;

  if (! (flags & HOLE_MANAGER)) return;

  s << pad << "Stats for do nothing hole management\n";

  holeman::reportStats(s, pad, flags);
}

// ******************************************************************

void MEDDLY::hm_none::clearHolesAndShrink(node_address new_last, bool shrink)
{
  holeman::clearHolesAndShrink(new_last, shrink);
}

