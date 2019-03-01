
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


#ifndef HM_ARRAY_H
#define HM_ARRAY_H

#include "holeman.h"

namespace MEDDLY {
  class hm_array;
};

#define MEASURE_LARGE_HOLE_STATS

/** Array-based hole management.

    Small holes are stored by size in an array of lists.
    Large holes are stored in a single list.
    Lists are doubly-linked so we may remove an arbitrary node.

    Holes are represented as follows:
    ---------------------------------------
    [0] -size (number of slots in hole)     
    [1] prev pointer
    [2] next pointer
    :
    :
    [size-1] -size

*/
class MEDDLY::hm_array : public holeman {
  public:
    hm_array();
    virtual ~hm_array();

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
      inline node_handle& h_prev(node_handle off) const {
        return holeOf(off)[hole_prev_index];
      }
      inline node_handle& h_next(node_handle off) const {
        return holeOf(off)[hole_next_index];
      }

      inline node_handle& Prev(node_handle off)       { return h_prev(off); }
      inline node_handle  Prev(node_handle off) const { return h_prev(off); }

      inline node_handle& Next(node_handle off)       { return h_next(off); }
      inline node_handle  Next(node_handle off) const { return h_next(off); }

      // Insert a node
      inline void listInsert(node_handle& list, node_handle node) {
        Next(node) = list;
        Prev(node) = 0;
        if (list) {
          Prev(list) = node;
        } 
        list = node; 
      }

      // Remove a node
      inline void listRemove(node_handle& list, node_handle node) {
        if (Prev(node)) {
          Next(Prev(node)) = Next(node);
        } else {
          list = Next(node);
        }
        if (Next(node)) {
          Prev(Next(node)) = Prev(node);
        }
      }

      // Determine list length
      inline long listLength(node_handle list) const {
        long count = 0;
        for (; list; list = Next(list)) count++;
        return count;
      }

  protected:
      // "front end" hole creation
      inline void insertHole(node_handle hole) {
        MEDDLY_DCASSERT(data);
        // Don't track holes smaller than smallestChunk():
        if (-data[hole] < smallestChunk()) {
          newUntracked(-data[hole]);
          return;
        }
        newHole(-data[hole]);
        if (-data[hole] < LARGE_SIZE) {
          listInsert(small_holes[-data[hole]], hole);
        } else {
          listInsert(large_holes, hole);
        }
      }
      // "front end" hole grab
      inline void removeHole(node_handle hole) {
        MEDDLY_DCASSERT(data);
        // Don't track holes smaller than smallestChunk():
        if (-data[hole] < smallestChunk()) {
          useUntracked(-data[hole]);
          return;
        }
        useHole(-data[hole]);
        if (-data[hole] < LARGE_SIZE) {
          listRemove(small_holes[-data[hole]], hole);
        } else {
          listRemove(large_holes, hole);
        }
      }


  private:
      // hole indexes
      static const int hole_prev_index = 1;
      static const int hole_next_index = 2;

      // when does a hole become "large"
      static const int LARGE_SIZE = 128;
      // static const int LARGE_SIZE = 1;  // extreme - everything is "large"

  private:
      /// List of large holes
      node_handle large_holes;
      /// Lists of holes by size
      node_handle small_holes[LARGE_SIZE];

#ifdef MEASURE_LARGE_HOLE_STATS
      long num_large_hole_traversals;
      long count_large_hole_visits;
#endif
};

#endif

