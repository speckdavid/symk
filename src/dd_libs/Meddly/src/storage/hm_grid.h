
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


#ifndef HM_GRID_H
#define HM_GRID_H

#include "holeman.h"

namespace MEDDLY {
  class hm_grid;
};



/** Grid-based hole management.
    Scheme from early versions of this library.

    When nodes are deleted, the memory slots are marked as a "hole",
    using the following format.

          slot[0] : -numslots, the number of slots in the hole
            .
            .
            .
          slot[L] : -numslots, with L = numslots-1
        
    The first few slots of the hole are used for a hole management
    data structure, described below.
    Note that a hole is guaranteed to be at least 5 slots long
    (assuming a node of size 0, with no extra header info, is impossible).


    Hole management.
    ==============================================================
    There are two kinds of holes depending on their location in the grid:
    Index Holes and Non-index Holes.
    Rows in the grid correspond to holes of the same size.
    The left-most column of the grid is a (vertical) list,
    and these are the index holes.
    Nodes in the middle are not connected vertically,
    and these are the non-index holes.

    The hole grid structure:
    ------------------------
    (holes_bottom)
    holes_of_size_0 (index) -- (non_index) -- (non_index) -- NULL
    |
    holes_of_size_1 -- ""
    |
    :
    :
    (holes_top)

    
    Note that the grid only stores holes up to the largest hole
    requested.  Larger holes are stored in the "large holes list".

    Index holes are represented as follows:
    ---------------------------------------
    [0] -size (number of slots in hole)     
    [1] up
    [2] down 
    [3] next pointer (nodes of same size)
    [4..size-2] Unused
    :
    :
    [size-1] -size

    Non-index holes are represented as follows:
    [0] -size (number of slots in hole)     
    [1] flag (<0, indicates non-index node)
    [2] prev pointer (nodes of same size)
    [3] next pointer (nodes of same size)
    [4..size-2] Unused
    :
    :
    [size-1] -size

*/
class MEDDLY::hm_grid : public holeman {
  public:
    hm_grid();
    virtual ~hm_grid();

  public:
    virtual node_address requestChunk(int slots);
    virtual void recycleChunk(node_address addr, int slots);
    virtual void dumpInternalInfo(output &s) const;
    virtual void dumpHole(output &s, node_address a) const;
    virtual void reportStats(output &s, const char* pad, unsigned flags) const;
    virtual void clearHolesAndShrink(node_address new_last, bool shrink);

  private:
      inline node_handle* holeOf(node_handle addr) const {
        MEDDLY_DCASSERT(data);
        MEDDLY_CHECK_RANGE(1, addr, lastSlot()+1);
        MEDDLY_DCASSERT(data[addr] < 0);  // it's a hole
        return data + addr;
      }
      inline node_handle& h_up(node_handle off) const {
        return holeOf(off)[hole_up_index];
      }
      inline node_handle& h_down(node_handle off) const {
        return holeOf(off)[hole_down_index];
      }
      inline node_handle& h_prev(node_handle off) const {
        return holeOf(off)[hole_prev_index];
      }
      inline node_handle& h_next(node_handle off) const {
        return holeOf(off)[hole_next_index];
      }

      inline node_handle& Up(node_handle off)       { return h_up(off); }
      inline node_handle  Up(node_handle off) const { return h_up(off); }

      inline node_handle& Down(node_handle off)       { return h_down(off); }
      inline node_handle  Down(node_handle off) const { return h_down(off); }

      inline node_handle& Prev(node_handle off)       { return h_prev(off); }
      inline node_handle  Prev(node_handle off) const { return h_prev(off); }

      inline node_handle& Next(node_handle off)       { return h_next(off); }
      inline node_handle  Next(node_handle off) const { return h_next(off); }

      inline bool isIndexHole(node_handle p_offset) const {
          return (non_index_hole != h_up(p_offset));
      }
      inline bool isNotIndexHole(node_handle p_offset) const {
          return (non_index_hole == h_up(p_offset));
      }

      // get middle ptrs
      inline void getMiddle(node_handle p_offset, node_handle &prev, 
        node_handle &next) const
      {
         node_handle* hole = holeOf(p_offset);
         MEDDLY_DCASSERT(non_index_hole == hole[hole_up_index]);
         prev = hole[hole_prev_index];
         next = hole[hole_next_index];
      }

      // get index ptrs
      inline void getIndex(node_handle p_offset, node_handle &up,
        node_handle &down, node_handle &next) const 
      {
         node_handle* hole = holeOf(p_offset);
         up = hole[hole_up_index];
         MEDDLY_DCASSERT(up != non_index_hole);
         down = hole[hole_down_index];
         next = hole[hole_next_index];
      }

      // set a middle node
      inline void makeMiddle(node_handle p_offset, node_handle prev, 
        node_handle next) 
      {
         node_handle* hole = holeOf(p_offset);
         hole[hole_up_index] = non_index_hole;
         hole[hole_prev_index] = prev;
         hole[hole_next_index] = next;
      }

      // set an index node
      inline void makeIndex(node_handle p_offset, node_handle up, 
        node_handle down, node_handle next) 
      {
        node_handle* hole = holeOf(p_offset);
        hole[hole_up_index] = up;
        hole[hole_down_index] = down;
        hole[hole_next_index] = next;
        // update the pointers of the holes (index) above and below it
        if (up) {
          Down(up) = p_offset;
        } else {
          holes_top = p_offset;
        }
        if (down) {
          Up(down) = p_offset;
        } else {
          holes_bottom = p_offset;
        }
      }

      // convert a node from index to non-index
      inline void index2middle(node_handle p_offset, node_handle prev)
      {
        node_handle* hole = holeOf(p_offset);
        MEDDLY_DCASSERT(non_index_hole != hole[hole_up_index]);
        hole[hole_up_index] = non_index_hole;
        hole[hole_prev_index] = prev;
        // next doesn't change!
      }

      // convert a node from non-index to index
      inline void middle2index(node_handle p_offset, node_handle up, 
        node_handle down)
      {
        node_handle* hole = holeOf(p_offset);
        MEDDLY_DCASSERT(non_index_hole == hole[hole_up_index]);
        hole[hole_up_index] = up;
        hole[hole_down_index] = down;
        // update the pointers of the holes (index) above and below it
        if (up) {
          Down(up) = p_offset;
        } else {
          holes_top = p_offset;
        }
        if (down) {
          Up(down) = p_offset;
        } else {
          holes_bottom = p_offset;
        }
      }

      // add a hole to the grid or large hole list, as appropriate
      //    @return true if the hole was tracked
      bool insertHole(node_handle p_offset);

      // remove a hole from the grid
      void removeHole(node_handle p_offset);

  private:  // helpers for recycleChunk

      // Add to the end of a hole
      //  @param  hole    Existing hole, already in grid
      //  @param  extra   New hole immediately after, not in grid
      //  @param  slots   Size of the new hole
      inline void appendHole(node_handle hole, node_handle extra, int slots) 
      {
        // Sanity checks
        MEDDLY_DCASSERT(data[hole] < 0);
        MEDDLY_DCASSERT(data[hole] == data[extra-1]);
        MEDDLY_DCASSERT(hole - extra == data[hole]);

        if (-data[hole] > max_request) {
          // Hole is large, modify it in place :^)
          useHole(-data[hole]);
          slots += -data[hole];
          data[hole] = data[hole+slots-1] = -slots;
          newHole(slots);
          return;
        }

        // Still here?  Hole is small, meaning it is 
        // somewhere in the grid according to its size.
        // So, we need to (1) Remove the hole
        removeHole(hole);

        // (2) Increase its size
        slots += -data[hole];
        data[hole] = data[hole+slots-1] = -slots;

        // (3) Insert it again
        if (insertHole(hole)) {
          newHole(slots);
        } else {
          newUntracked(slots);
        }
      }

      // Add to the start of a hole
      //  @param  extra   New hole immediately before, not in grid
      //  @param  hole    Existing hole, already in grid
      inline void prependHole(node_handle extra, node_handle hole) {
        // With this data structure, this operation is identical to
        // (1) Remove
        removeHole(hole);

        // (2) Enlarge
        int slots = (hole - extra) -data[hole]; // total #slots now
        data[extra] = data[extra+slots-1] = -slots;

        // (3) Insert
        if (insertHole(extra)) {
          newHole(slots);
        } else {
          newUntracked(slots);
        }
      }

      // Group two holes with a new hole in the middle
      //  @param  left    Existing hole, already in the grid
      //  @param  mid     New hole in between
      //  @param  right   Existing hole, already in the grid
      inline void groupHoles(node_handle left, node_handle mid, 
        node_handle right)
      {
        // In this data structure, this operation is the same as:
        removeHole(right);
        appendHole(left, mid, (right-mid) - data[right]); 
      }

  private:
      static const int non_index_hole = -2;   // any negative will do
      // hole indexes
      static const int hole_up_index = 1;
      static const int hole_down_index = hole_up_index+1;
      static const int hole_prev_index = hole_down_index;
      static const int hole_next_index = hole_prev_index+1;

  private:
      /// Largest hole ever requested
      node_handle max_request;
      /// List of large holes
      node_handle large_holes;
      /// Pointer to top of holes grid
      node_handle holes_top;
      /// Pointer to bottom of holes grid
      node_handle holes_bottom;
};

#endif

