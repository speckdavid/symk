
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



#include "hm_grid.h"

#include <map>

// #define MEMORY_TRACE
// #define DEEP_MEMORY_TRACE


// ******************************************************************
// *                                                                *
// *                                                                *
// *                        hm_grid  methods                        *
// *                                                                *
// *                                                                *
// ******************************************************************

MEDDLY::hm_grid::hm_grid() : holeman(5)
{
  max_request = 0;
  large_holes = 0;
  holes_top = 0;
  holes_bottom = 0;
}

MEDDLY::hm_grid::~hm_grid()
{
}

MEDDLY::node_address MEDDLY::hm_grid::requestChunk(int slots)
{
#ifdef MEMORY_TRACE
  printf("Requesting %d slots\n", slots);
#endif
#ifdef DEEP_MEMORY_TRACE
  printf("-------------------------------------------\n", slots);
  dumpInternal(stdout, 0x03);
#endif

  if (slots > max_request) {
#ifdef MEMORY_TRACE
    printf("New max request; shifting holes from large list to grid\n");
#endif
    max_request = slots;
    // Traverse the large hole list, and re-insert
    // all the holes, in case some are now smaller
    // than max_request.
    node_handle curr = large_holes;
    large_holes = 0;
    for (; curr; ) {
      node_handle next = Next(curr);
      insertHole(curr);
      curr = next;
    }
  }

  // find us a good hole
  node_handle found = 0;

  // First, try for a hole exactly of this size
  // by traversing the index nodes in the hole grid
  node_handle chain = 0;
  node_handle curr = holes_bottom;
  while (curr) {
    if (slots == -(data[curr])) {
      if (Next(curr)) {
        found = Next(curr);
      } else {
        found = curr;
      }
      break;
    }
    if (slots < -(data[curr])) {
      // no exact match possible
      curr = 0;
      break;
    }
    // move up the hole grid
    curr = Up(curr);
    chain++;
  }

  // If that failed, try the large hole list
  if (!found && large_holes) {
    // should be large enough
    found = large_holes;
  }

  // If that failed, try the largest hole in the grid
  if (!found && holes_top) if (slots < -data[holes_top]) {
    if (Next(holes_top)) {
      found = Next(holes_top);
    } else {
      found = holes_top;
    }
  }

  if (found) {
    // We have a hole to recycle
    // Sanity check:
    MEDDLY_DCASSERT(slots <= -data[found]);

    // Remove the hole
    removeHole(found);

    node_handle newhole = found + slots;
    node_handle newsize = -(data[found]) - slots;
    data[found] = -slots;
    if (newsize > 0) {
      // Save the leftovers - make a new hole!
      data[newhole] = -newsize;
      data[newhole + newsize - 1] = -newsize;
      if (insertHole(newhole)) {
        newHole(newsize);
      } else {
        newUntracked(newsize);
      }
    }
#ifdef MEMORY_TRACE
    printf("\trecycled %d slots, addr %ld\n", -data[found], long(found));
#endif
    return found;
  }
  
  // 
  // Still here?  We couldn't recycle a node.
  // 
  found = allocFromEnd(slots);
  return found;
}

// ******************************************************************

void MEDDLY::hm_grid::recycleChunk(node_address addr, int slots)
{
#ifdef MEMORY_TRACE
  printf("Calling recycleChunk(%ld, %d)\n", addr, slots);
#ifdef DEEP_MEMORY_TRACE
  printf("-------------------------------------------\n", slots);
  dumpInternal(stdout, 0x03);
#else
  printf("Node %ld: ", addr);
  dumpInternalNode(stdout, addr, 0x03);
#endif
#endif

  decMemUsed(slots * sizeof(node_handle));

  // Can we absorb this hole at the end?
  if (addr+slots-1 == lastSlot()) {
    // YES!
    releaseToEnd(slots);
    // Check for a hole to our left
    if (data[lastSlot()]<0) {
      // it's a hole; absorb it at the end too
      slots = -data[lastSlot()];
      addr = lastSlot() - slots + 1;
      removeHole(addr);
      releaseToEnd(slots);
    }
    return;
  }

  //
  // Still here.  The hole is not at the end.
  // See if we are adjacent to other holes,
  // and if so, merge with those.
  //

  if (data[addr-1] < 0) {
    // There's a hole to our immediate left.

    if (data[addr+slots] < 0) {
      // Hole to the left and right
      groupHoles(addr + data[addr-1], addr, addr + slots);
#ifdef MEMORY_TRACE
      printf("Grouped holes %ld, %ld, %ld\n", 
        addr + data[addr-1], addr, addr + slots
      );
#endif
    } else {
      // Hole to the left only
      appendHole(addr + data[addr-1], addr, slots);
#ifdef MEMORY_TRACE
      printf("Merged %ld with new hole %ld\n", addr + data[addr-1], addr);
#endif
    }
  } else {
    // No hole to our immediate left.

    if (data[addr+slots] < 0) {
      // Hole to the right only
      prependHole(addr, addr + slots);
#ifdef MEMORY_TRACE
      printf("New hole %ld, merged wth %ld\n", addr, addr + slots);
#endif
    } else {
      // No nearby holes
      data[addr] = data[addr+slots-1] = -slots;
      // Add hole to grid
      if (insertHole(addr)) {
        newHole(slots);
      } else {
        newUntracked(slots);
      }
#ifdef MEMORY_TRACE
      printf("Made Hole %ld\n", addr);
#endif
    }
  }
}

// ******************************************************************

void MEDDLY::hm_grid::dumpInternalInfo(output &s) const
{
  s << "Last slot used: " << long(lastSlot()) << "\n";
  s << "Total hole slots: " <<  holeSlots() << "\n";
  s << "large_holes: " << long(large_holes) << "\n";
  s << "Grid: top = " << long(holes_top) << " bottom = " << long(holes_bottom) << "\n";
}

// ******************************************************************

void MEDDLY::hm_grid::dumpHole(output &s, node_address a) const
{
  MEDDLY_DCASSERT(data);
  MEDDLY_CHECK_RANGE(1, a, lastSlot());
  s << '[' << long(data[a]);
  if (-data[a] <  smallestChunk()) {
    // not tracked
    for (int i=1; i<-data[a]; i++) {
      s << ", " << long(data[a+i]);
    }
    s << "]\n";
  } else {
    // tracked hole
    if (isIndexHole(a)) {
      node_handle up, down, next;
      getIndex(a, up, down, next);
      s << ", u: " << long(up) << ", d: " << long(down) << ", n: " << long(next) << ", ";
    } else {
      node_handle next, prev;
      getMiddle(a, prev, next);
      s << ", non-index, p: " << long(prev) << ", n: " << long(next) << ", ";
    }
    long aN = chunkAfterHole(a)-1;
    s << "..., " << long(data[aN]) << "]\n";
  }
}

// ******************************************************************

void MEDDLY::hm_grid
::reportStats(output &s, const char* pad, unsigned flags) const
{
  static unsigned HOLE_MANAGER =
    expert_forest::HOLE_MANAGER_STATS | expert_forest::HOLE_MANAGER_DETAILED;

  if (! (flags & HOLE_MANAGER)) return;

  s << pad << "Stats for grid hole management\n";

  holeman::reportStats(s, pad, flags);

  if (! (flags & expert_forest::HOLE_MANAGER_DETAILED)) return;

  // Compute chain length histogram
  std::map<int, int> chainLengths;
  for (int curr = holes_bottom; curr; curr = Up(curr)) {
    int currHoleOffset = curr;
    int count = 0;
    // traverse this chain to get its length
    for (count = 0; currHoleOffset; count++) {
      currHoleOffset = Next(currHoleOffset);
    }
    int currHoleSize = -data[curr];
    chainLengths[currHoleSize] += count;
  }
  // add the large hole list
  int count = 0;
  for (int curr = large_holes; curr; curr = Next(curr)) {
    count++;
  }
  chainLengths[-1] += count;

  // Display the histogram

  s << pad << "    Hole Chains (size, count):\n";
  for (std::map<int, int>::iterator iter = chainLengths.begin();
      iter != chainLengths.end(); ++iter)
    {
      if (iter->first<0) {
        s << pad << "\tlarge: " << iter->second << "\n";
      } else {
        s << pad << "\t";
        s.put(long(iter->first), 5);
        s << ": " << iter->second << "\n";
      }
    }
  s << pad << "    End of Hole Chains\n";
}

// ******************************************************************

void MEDDLY::hm_grid::clearHolesAndShrink(node_address new_last, bool shrink)
{
  holeman::clearHolesAndShrink(new_last, shrink);

  // set up hole pointers and such
  holes_top = holes_bottom = 0;
  large_holes = 0;
}

// ******************************************************************
// *                                                                *
// *                        private  helpers                        *
// *                                                                *
// ******************************************************************

bool MEDDLY::hm_grid::insertHole(node_handle p_offset)
{
#ifdef MEMORY_TRACE
  printf("insertHole(%ld)\n", long(p_offset));
#endif

  // sanity check to make sure that the first and last slots in this hole
  // have the same value, i.e. -(# of slots in the hole)
  MEDDLY_DCASSERT(data[p_offset] == data[p_offset - data[p_offset] - 1]);

  // If the hole is too small, don't bother to track it
  if (-data[p_offset] < smallestChunk()) {
#ifdef MEMORY_TRACE
    printf("\thole size %d, too small to track\n", -data[p_offset]);
#endif
    return false;
    // This memory can still be reclaimed 
    // when it is merged with neighboring holes :^)
  }

  // Check if we belong in the grid, or the large hole list
  if (-data[p_offset] > max_request) {
#ifdef MEMORY_TRACE
    printf("\tAdding to large_holes: %d\n", large_holes);
#endif
    // add to the large hole list
    makeMiddle(p_offset, 0, large_holes);
    if (large_holes) Prev(large_holes) = p_offset;
    large_holes = p_offset;
    return true;
  }

  // special case: empty
  if (0 == holes_bottom) {
#ifdef MEMORY_TRACE
    printf("\tAdding to empty grid\n");
#endif
    // index hole
    makeIndex(p_offset, 0, 0, 0);
    return true;
  }

  // special case: at top
  if (data[p_offset] < data[holes_top]) {
#ifdef MEMORY_TRACE
    printf("\tAdding new chain at top\n");
#endif
    // index hole
    makeIndex(p_offset, 0, holes_top, 0);
    return true;
  }

  // find our vertical position in the grid
  node_handle above = holes_bottom;
  node_handle below = 0;
  while (data[p_offset] < data[above]) {
    below = above;
    above = Up(below);
    MEDDLY_DCASSERT(Down(above) == below);
    MEDDLY_DCASSERT(above);  
  }
  if (data[p_offset] == data[above]) {
#ifdef MEMORY_TRACE
    printf("\tAdding to chain\n");
#endif
    // Found, add this to chain
    // making a non-index hole
    node_handle right = Next(above);
    makeMiddle(p_offset, above, right);
    if (right) Prev(right) = p_offset;
    Next(above) = p_offset;
    return true;
  }
#ifdef MEMORY_TRACE
  printf("\tAdding new chain\n");
#endif
  // we should have above < p_offset < below  (remember, -sizes)
  // create an index hole since there were no holes of this size
  makeIndex(p_offset, above, below, 0);
  return true;
}

// ******************************************************************

void MEDDLY::hm_grid::removeHole(node_handle p_offset)
{
  MEDDLY_DCASSERT(data);
  MEDDLY_DCASSERT(data[p_offset] < 0);

  if (-data[p_offset] < smallestChunk()) {
    useUntracked(-data[p_offset]);
    return; // not tracked
  }

  useHole(-data[p_offset]);

  if (isIndexHole(p_offset)) {
      //
      // Index node
      //
#ifdef MEMORY_TRACE
      printf("indexRemove(%ld)\n", long(p_offset));
#endif
      node_handle above, below, right;
      getIndex(p_offset, above, below, right);

      if (right) {
        // convert right into an index node
        middle2index(right, above, below);

      } else {
        // remove this row completely

        // update the pointers of the holes (index) above and below it
        if (above) {
          Down(above) = below;
        } else {
          holes_top = below;
        }

        if (below) {
          Up(below) = above;
        } else {
          holes_bottom = above;
        }
      }
#ifdef MEMORY_TRACE
      printf("Removed Index hole %ld\n", long(p_offset));
#ifdef DEEP_MEMORY_TRACE
      dumpInternal(stdout, 0x03);
#else 
      dumpInternalNode(stdout, p_offset, 0x03);
#endif
#endif
      //
      // Done with index node removal
      //
  } else {
      //
      // Not an index node
      //
#ifdef MEMORY_TRACE
      printf("midRemove(%ld)\n", long(p_offset));
#endif
      // Remove a "middle" node, either in the grid
      // or in the large hole list.
      //
      node_handle left, right;
      getMiddle(p_offset, left, right);

      if (left) {
        Next(left) = right;
      } else {
        // MUST be head of the large hole list
        MEDDLY_DCASSERT(large_holes == p_offset);
        large_holes = right;
      }
      if (right) Prev(right) = left;

#ifdef MEMORY_TRACE
      printf("Removed Non-Index hole %ld\n", long(p_offset));
#ifdef DEEP_MEMORY_TRACE
      dumpInternal(stdout, 0x03);
#else 
      dumpInternalNode(stdout, p_offset, 0x03);
#endif
#endif
  }
}

// ******************************************************************

