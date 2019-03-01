
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

#include "hm_array.h"


#define MERGE_AND_SPLIT_HOLES
// #define MEMORY_TRACE
// #define DEEP_MEMORY_TRACE


// ******************************************************************
// *                                                                *
// *                                                                *
// *                        hm_array methods                        *
// *                                                                *
// *                                                                *
// ******************************************************************

MEDDLY::hm_array::hm_array() : holeman(4)
{
  large_holes = 0;
  for (int i=0; i<LARGE_SIZE; i++) {
    small_holes[i] = 0;
  }
#ifdef MEASURE_LARGE_HOLE_STATS
  num_large_hole_traversals = 0;
  count_large_hole_visits = 0;
#endif
}

// ******************************************************************

MEDDLY::hm_array::~hm_array()
{
}

// ******************************************************************

MEDDLY::node_address MEDDLY::hm_array::requestChunk(int slots)
{
#ifdef MEMORY_TRACE
  printf("Requesting %d slots\n", slots);
#endif
  node_handle found = 0;
  // Try for an exact fit first
  if (slots < LARGE_SIZE) {
    if (small_holes[slots]) {
      found = small_holes[slots];
      listRemove(small_holes[slots], found);
      useHole(slots);
      MEDDLY_DCASSERT(data[found] = -slots);
    }
  }
  // Try the large hole list and check for a hole large enough
  if (!found) {
#ifdef MEASURE_LARGE_HOLE_STATS
    num_large_hole_traversals++;
#endif
    for (node_handle curr = large_holes; curr; curr = Next(curr)) {
#ifdef MEASURE_LARGE_HOLE_STATS
      count_large_hole_visits++;
#endif
      if (-data[curr] >= slots) {
        // we have a large enough hole, grab it
        found = curr;
        // remove the hole from the list
        listRemove(large_holes, found);
        useHole(-data[curr]);
        break;
      }
    }
  }

  if (found) {
    // We found a hole to recycle.
#ifdef MEMORY_TRACE
    printf("Removed hole %d\n", found);
#ifdef DEEP_MEMORY_TRACE
    getParent()->dumpInternal(stdout);
#else 
    dumpHole(stdout, found);
#endif
#endif
    // Sanity check:
    MEDDLY_DCASSERT(slots <= -data[found]);
    
    node_handle newhole = found + slots;
    node_handle newsize = -(data[found]) - slots;
    data[found] = -slots;
    if (newsize > 0) {
      // Save the leftovers - make a new hole!
      data[newhole] = -newsize;
      data[newhole + newsize - 1] = -newsize;
      insertHole(newhole);
    }
    return found;
  }
  
  // 
  // Still here?  We couldn't recycle a node.
  // 
  return allocFromEnd(slots);
}

// ******************************************************************

void MEDDLY::hm_array::recycleChunk(node_address addr, int slots)
{
#ifdef MEMORY_TRACE
  printf("Calling recycleChunk(%ld, %d)\n", long(addr), slots);
#endif

  decMemUsed(slots * sizeof(node_handle));

  data[addr] = data[addr+slots-1] = -slots;

  // Check for a hole to the left
#ifdef MERGE_AND_SPLIT_HOLES
  if (data[addr-1] < 0) {
    // Merge!
#ifdef MEMORY_TRACE
    printf("Left merging\n");
#endif
    // find the left hole address
    node_handle lefthole = addr + data[addr-1];
    MEDDLY_DCASSERT(data[lefthole] == data[addr-1]);

    // remove the left hole
    removeHole(lefthole);

    // merge with us
    slots += (-data[lefthole]);
    addr = lefthole;
    data[addr] = data[addr+slots-1] = -slots;
  }
#endif

  // if addr is the last hole, absorb into free part of array
  MEDDLY_DCASSERT(addr + slots - 1 <= lastSlot());
  if (addr+slots-1 == lastSlot()) {
    releaseToEnd(slots);
    return;
  }

  // Still here? Wasn't the last hole.

#ifdef MERGE_AND_SPLIT_HOLES
  // Check for a hole to the right
  if (data[addr+slots]<0) {
    // Merge!
#ifdef MEMORY_TRACE
    printf("Right merging\n");
#endif
    // find the right hole address
    node_handle righthole = addr+slots;

    // remove the right hole
    removeHole(righthole);
    
    // merge with us
    slots += (-data[righthole]);
    data[addr] = data[addr+slots-1] = -slots;
  }
#endif

  // Add hole to the proper list
  insertHole(addr);

#ifdef MEMORY_TRACE
  printf("Made Hole %ld\n", long(addr));
#ifdef DEEP_MEMORY_TRACE
  getParent()->dumpInternal(stdout);
#else
  dumpHole(stdout, addr);
#endif
#endif
}

// ******************************************************************

void MEDDLY::hm_array::dumpInternalInfo(output &s) const
{
  s << "Last slot used: " << long(lastSlot()) << "\n";
  s << "Total hole slots: " << holeSlots() << "\n";
  s << "small_holes: (";
  bool printed = false;
  for (int i=0; i<LARGE_SIZE; i++) if (small_holes[i]) {
    if (printed) s << ", ";
    s << i << ":" << small_holes[i];
    printed = true;
  }
  s << ")\n";
  s << "large_holes: " << long(large_holes) << "\n";
}

// ******************************************************************

void MEDDLY::hm_array::dumpHole(output &s, node_address a) const
{
  MEDDLY_DCASSERT(data);
  MEDDLY_CHECK_RANGE(1, a, lastSlot());
  long aN = chunkAfterHole(a)-1;
  s << "[" << long(data[a]) << ", p: " << long(data[a+1]) << ", n: " 
    << long(data[a+2]) << ", ..., " << long(data[aN]) << "]\n";
}

// ******************************************************************

void MEDDLY::hm_array
::reportStats(output &s, const char* pad, unsigned flags) const
{
  static unsigned HOLE_MANAGER =
    expert_forest::HOLE_MANAGER_STATS | expert_forest::HOLE_MANAGER_DETAILED;

  if (! (flags & HOLE_MANAGER)) return;

  s << pad << "Stats for array of lists hole management\n";

  holeman::reportStats(s, pad, flags);

#ifdef MEASURE_LARGE_HOLE_STATS
  if (flags & expert_forest::HOLE_MANAGER_STATS) {
    s << pad << "    #traversals large_holes: " << num_large_hole_traversals << "\n";
    if (num_large_hole_traversals) {
      s << pad << "    total traversal cost: " << count_large_hole_visits << "\n";
      double avg = count_large_hole_visits;
      avg /= num_large_hole_traversals;
      s << pad << "    Avg cost per traversal : " << avg << "\n";
    }
  }
#endif

  if (flags & expert_forest::HOLE_MANAGER_DETAILED) {
    s << pad << "    Length of non-empty chains:\n";
    for (int i=0; i<LARGE_SIZE; i++) {
      long L = listLength(small_holes[i]);
      if (L) {
        s << pad << "\tsize ";
        s.put(long(i), 3);
        s << ": " << L << "\n";
      }
    }
    long LL = listLength(large_holes);
    if (LL) s << pad << "\tlarge   : " << listLength(large_holes) << "\n";
  }

}

// ******************************************************************

void MEDDLY::hm_array::clearHolesAndShrink(node_address new_last, bool shrink)
{
  holeman::clearHolesAndShrink(new_last, shrink);

  // set up hole pointers and such
  large_holes = 0;
  for (int i=0; i<LARGE_SIZE; i++) small_holes[i] = 0;
}

