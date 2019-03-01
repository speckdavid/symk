
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


#ifndef HM_HEAP_H
#define HM_HEAP_H

#include "holeman.h"

namespace MEDDLY {
  class hm_heap;
};


/** Grid-based hole management with heaps.

    We have a doubly-linked list of heaps of holes.
    Each heap contains holes of the same size.
    Holes are ordered by address in the heaps,
    and the policy is that we use the smallest address hole 
    that is large enough to satisfy a request.
    Note that the index node has lower address than
    all index nodes in the heap it points to.

    Index node representation:
    ---------------------------------------
    [0] -size (number of slots in hole)     
    [1] up (to larger holes)
    [2] down (to smaller holes)
    [3] root (pointer to heap)
    [4] -heapsize
    :
    :
    [size-1] -size
    ---------------------------------------


    Heap node representation
    ---------------------------------------
    [0] -size (number of slots in hole)     
    [1] left child
    [2] right child
    [3] parent (because we can)
    [4] ID (gives position in the heap)
    :
    :
    [size-1] -size
    ---------------------------------------

  
    Heaps:
    Because we cannot have an array implementation of each heap,
    we use an ID to specify where each node is in the heap, where
    the root node has ID 1, and a generic node with ID i has a parent
    with ID i/2 and children with IDs 2i and 2i+1:

                            1
                  2                   3 
             4         5         6         7
          08   09   10   11   12   13   14   15
*/
class MEDDLY::hm_heap : public holeman {
  public:
    hm_heap();
    virtual ~hm_heap();

  public:
    virtual node_address requestChunk(int slots);
    virtual void recycleChunk(node_address addr, int slots);
    virtual void dumpInternalInfo(output &s) const;
    virtual void dumpHole(output &s, node_address a) const;
    virtual void reportStats(output &s, const char* pad, unsigned flags) const;
    virtual void clearHolesAndShrink(node_address new_last, bool shrink);

  private:
    // add a hole to the grid or large hole list, as appropriate
    //  @return   true if this hole is tracked
    bool insertHole(node_handle p_offset);

    // remove a hole from the grid or large hole list, as appropriate
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
      inline void prependHole(node_handle extra, node_handle hole)
      {
        int slots = (hole - extra) -data[hole]; // total #slots now

        if (-data[hole] > max_request) {
          // Hole is large, modify it in place :^)
          useHole(-data[hole]);
          data[extra] = data[extra+slots-1] = -slots;
          newHole(slots);

          // shift the node data over
          shiftNode(extra, hole);
          return;
        }

        // Small hole - do the usual

        // (1) Remove
        removeHole(hole);

        // (2) Enlarge
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
        // Do our best to modify things in place
        if ((-data[right] > max_request) && (-data[left] <= max_request)) {
          // left is small, right is large
          // merge everything into the right hole
          removeHole(left);
          prependHole(left, right);
        } else {
          // Merge everything into the left hole
          // (note - merging into the left is faster,
          //  so that's why we do this most of the time)
          removeHole(right);
          appendHole(left, mid, (right-mid) - data[right]); 
        }
      }

  private:
    static const int left_index = 1;
    static const int right_index = 2;
    static const int parent_index = 3;
    static const int ID_index = 4;

    static const int up_index = 1;
    static const int down_index = 2;
    static const int root_index = 3;
    static const int size_index = 4;

  private:
      inline node_handle* holeOf(node_handle addr) const {
        MEDDLY_DCASSERT(data);
        MEDDLY_CHECK_RANGE(1, addr, lastSlot()+1);
        MEDDLY_DCASSERT(data[addr] < 0);  // it's a hole
        return data + addr;
      }
      inline bool isIndexHole(node_handle addr) const {
        return holeOf(addr)[size_index] <= 0;
      }
      inline bool isNotIndexHole(node_handle addr) const {
        return holeOf(addr)[ID_index] > 0;
      }

      inline node_handle& rawLeft(node_handle addr) const {
        MEDDLY_DCASSERT(isNotIndexHole(addr));
        return holeOf(addr)[left_index];
      }
      inline node_handle& rawRight(node_handle addr) const {
        MEDDLY_DCASSERT(isNotIndexHole(addr));
        return holeOf(addr)[right_index];
      }
      inline node_handle& rawParent(node_handle addr) const {
        MEDDLY_DCASSERT(isNotIndexHole(addr));
        return holeOf(addr)[parent_index];
      }
      inline node_handle& rawID(node_handle addr) const {
        MEDDLY_DCASSERT(isNotIndexHole(addr));
        return holeOf(addr)[ID_index];
      }

      inline node_handle Left(node_handle addr) const {
        return rawLeft(addr); 
      }
      inline node_handle Right(node_handle addr) const {
        return rawRight(addr); 
      }
      inline node_handle Parent(node_handle addr) const {
        return rawParent(addr); 
      }
      inline node_handle ID(node_handle addr) const {
        return rawID(addr); 
      }

      inline void setLeft(node_handle addr, node_handle left) {
        rawLeft(addr) = left;
        if (left) rawParent(left) = addr;
      }
      inline void setRight(node_handle addr, node_handle right) {
        rawRight(addr) = right;
        if (right) rawParent(right) = addr;
      }

      inline node_handle& rawUp(node_handle addr) const {
        MEDDLY_DCASSERT(isIndexHole(addr));
        return holeOf(addr)[up_index];
      }
      inline node_handle& rawDown(node_handle addr) const {
        MEDDLY_DCASSERT(isIndexHole(addr));
        return holeOf(addr)[down_index];
      }
      inline node_handle& rawRoot(node_handle addr) const {
        MEDDLY_DCASSERT(isIndexHole(addr));
        return holeOf(addr)[root_index];
      }
      inline node_handle& rawSize(node_handle addr) const {
        MEDDLY_DCASSERT(isIndexHole(addr));
        return holeOf(addr)[size_index];
      }

      inline node_handle Up(node_handle addr) const   { return rawUp(addr); }
      inline node_handle Down(node_handle addr) const { return rawDown(addr); }
      inline node_handle Root(node_handle addr) const { return rawRoot(addr); }
      inline node_handle Size(node_handle addr) const { return rawSize(addr); }

      inline node_handle& Up(node_handle addr)    { return rawUp(addr); }
      inline node_handle& Down(node_handle addr)  { return rawDown(addr); }
      inline node_handle& Root(node_handle addr)  { return rawRoot(addr); }
      inline node_handle& Size(node_handle addr)  { return rawSize(addr); }

      // get index ptrs
      inline void getIndex(node_handle p_offset, node_handle &up,
        node_handle &down, node_handle &heap, node_handle &size) const 
      {
        node_handle* hole = holeOf(p_offset);
        up = hole[up_index];
        down = hole[down_index];
        heap = hole[root_index];
        size = hole[size_index];
        MEDDLY_DCASSERT(size<=0);
      }

      // get all heap pointers at once
      inline void getHeapNode(node_handle p_offset, node_handle& ID,
        node_handle& parent, node_handle& left, node_handle& right) const
      {
        node_handle* node = holeOf(p_offset);
        left = node[left_index];
        right = node[right_index];
        parent = node[parent_index];
        ID = node[ID_index];
        MEDDLY_DCASSERT(node[ID_index] > 0);
      }

      // set an index node
      inline void makeIndex(node_handle p_offset, node_handle up, 
        node_handle down, node_handle heap, node_handle size) 
      {
        MEDDLY_DCASSERT(size<=0);
        node_handle* hole = holeOf(p_offset);
        hole[up_index] = up;
        hole[down_index] = down;
        hole[root_index] = heap;
        hole[size_index] = size;
        // nice - set the above and below pointers to us
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

      // set all heap pointers (except parent) at once
      inline void makeHeapNode(node_handle addr, node_handle ID, 
        node_handle left, node_handle right) 
      {
        node_handle* node = holeOf(addr);
        node[left_index] = left;
        node[right_index] = right;
        node[ID_index] = ID;
        MEDDLY_DCASSERT(node[ID_index] > 0);
        if (left) rawParent(left) = addr;
        if (right) rawParent(right) = addr;
      }

      // shift a node
      inline void shiftNode(node_handle newaddr, node_handle oldaddr) {
          node_handle left, right, parent, ID;
          getHeapNode(oldaddr, ID, parent, left, right);
          makeHeapNode(newaddr, ID, left, right);

          // update our parent
          if (parent) {
            if (ID % 2) {
              MEDDLY_DCASSERT(Right(parent) == oldaddr);
              setRight(parent, newaddr);
            } else {
              MEDDLY_DCASSERT(Left(parent) == oldaddr);
              setLeft(parent, newaddr);
            }
          } else {
            MEDDLY_DCASSERT(large_holes == oldaddr);
            large_holes = newaddr;
            rawParent(newaddr) = 0;
          }
      }

  private:

      // find the last non-null node on the path to the given ID.
      inline node_handle takePathToID(node_handle root, unsigned long ID) const
      {
        if (0==root) return 0;
        // ID bit pattern gives the path :^)
        // determine which bit to start from...
        unsigned long two2b = 0x1;
        int bit = 0;
        for (; two2b <= ID; two2b <<=1) { bit++; }
        two2b >>= 1;
        // now, traverse
        for (node_handle n=root; n; ) {
          two2b >>= 1;
          if (0==--bit) return n;   // we're at the right position
          node_handle child = (ID & two2b) ? Right(n) : Left(n);
          if (!child) return n;   // return before we become null
          n = child;
        }
        // we should never get here, but do something sane just in case
        return 0;
      }

      // add node to the given heap
      void addToHeap(node_handle& root, node_handle& size, node_handle p);

      // remove a node known to be in this heap
      void removeFromHeap(node_handle& root, node_handle& size, node_handle p);

      // repair heap upwards
      void upHeap(node_handle& root, node_handle pp, node_handle p);

      // repair heap downwards
      node_handle downHeap(node_handle ID, 
        node_handle left, node_handle right, node_handle replace);

      // convert a heap into a singly-linked list (using the "parent" pointer)
      void heapToList(node_handle root);

#ifdef DEVELOPMENT_CODE
      void verifyHeap(node_handle ptr, node_handle node) const;
#endif

  private:  // data
      /// Largest hole ever requested
      node_handle max_request;
      /// Heap of large holes
      node_handle large_holes;
      /// Size of large holes heap (negative, for consistency)
      node_handle large_holes_size;
      /// Pointer to top of holes grid
      node_handle holes_top;
      /// Pointer to bottom of holes grid
      node_handle holes_bottom;
};

#endif

