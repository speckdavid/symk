
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



#ifndef OLD_SCHEME_H
#define OLD_SCHEME_H

#include "../defines.h"
#include "../hash_stream.h"
#include "holeman.h"

#define INTEGRATED_HOLE_MANAGER

namespace MEDDLY {
  // node storage mechanism used for versions < 0.10 of the library
  class old_node_storage_style;
  class old_node_storage;
};


/**
    Factory to build an old_node_storage object.
*/
class MEDDLY::old_node_storage_style : public node_storage_style {
  public:
    old_node_storage_style();
    virtual ~old_node_storage_style();
    virtual node_storage* createForForest(expert_forest* f) const;
};


/** Original node storage mechanism in a forest.
    The hole manager is integrated into this class.

    Limits: offsets must be the same as node_handles, 
    because the data structure uses those for hole data.

    Details of node storage are left to the derived forests.
    However, every active node is stored in the following format.

      common   {  slot[0] : incoming count, >= 0.
      header --{  slot[1] : next pointer in unique table or special value.
               {  slot[2] : size.  >=0 for full storage, <0 for sparse.

      unhashed    {       : slots used for any extra information
      header    --{       : as needed on a forest-by-forest basis.
      (optional)  {       : Info does NOT affect node uniqueness.

      hashed      {       : slots used for any extra information
      header    --{       : as needed on a forest-by-forest basis.
      (optional)  {       : Info DOES affect node uniqueness.

                  {       : Downward pointers.
                  {       : If full storage, there are size pointers
      down -------{       : and entry i gives downward pointer i.
                  {       : If sparse storage, there are -size pointers
                  {       : and entry i gives a pointer but the index
                  {       : corresponds to index[i], below.
          
                  {       : Index entries.
                  {       : Unused for full storage.
      index ------{       : If sparse storage, entry i gives the
      (sparse)    {       : index for outgoing edge i, and there are
                  {       : -size entries.

                  {       : Edge values.
      edge        {       : If full storage, there are size * edgeSize
      values -----{       : slots; otherwise there are -size * edgeSize
                  {       : slots.  Derived forests are responsible
                  {       : for packing information into these slots.

                { -padlen : Any node padding to allow for future
                {         : expansion, or for memory management purposes
                {         : (e.g., memory hold is larger than requested).
      padding --{         : padlen is number of padded slots.  If the
                {         : first entry after the node proper is negative,
                {         : then it specifies the number of long padding
                {         : slots; otherwise, there is no padding.

      tail    --{ slot[L] : The forest node number,
                            guaranteed to be non-negative.


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
class MEDDLY::old_node_storage : public node_storage {
  // required interface
  public:
    old_node_storage(expert_forest* f);
    virtual ~old_node_storage();

    virtual void collectGarbage(bool shrink);
    virtual void reportStats(output &s, const char* pad, unsigned flags) const;

    /*
    virtual void showNode(output &s, node_address addr, bool verb) const;
    virtual void writeNode(output &s, node_address addr, const node_handle* map)
    const;
    */

    virtual node_address makeNode(node_handle p, const unpacked_node &nb, 
        node_storage_flags opt);

    virtual void unlinkDownAndRecycle(node_address addr);

    virtual bool areDuplicates(node_address addr, const unpacked_node &nr) const;
    virtual void fillUnpacked(unpacked_node &ur, node_address addr, unpacked_node::storage_style) const;
    virtual unsigned hashNode(int level, node_address addr) const;
    virtual int getSingletonIndex(node_address addr, node_handle &down) const;
    virtual node_handle getDownPtr(node_address addr, int index) const;
    virtual void getDownPtr(node_address addr, int ind, int& ev, node_handle& dn) const;
    virtual void getDownPtr(node_address addr, int ind, float& ev, node_handle& dn) const;
    virtual const void* getUnhashedHeaderOf(node_address addr) const;
    virtual const void* getHashedHeaderOf(node_address addr) const;

  protected:
    virtual void updateData(node_handle* d);
    virtual int smallestNode() const;
    virtual void dumpInternalInfo(output &) const;
    virtual node_address 
    dumpInternalNode(output &, node_address addr, unsigned flags) const;
    virtual void dumpInternalTail(output &) const;


  private:
      static const int min_size = 1024;

      static const int non_index_hole = -2;
      static const long temp_node_value = -5;

      // header indexes (relative to chunk start)
      static const int count_index = 0;
      static const int next_index = 1;    
      static const int size_index = 2;
      static const int headerSlots = size_index+1;

      // Counts for extra slots
      static const int tailSlots = 1;
      static const int extraSlots = headerSlots + tailSlots;

      // hole indexes
      static const int hole_up_index = 1;
      static const int hole_down_index = hole_up_index+1;
      static const int hole_prev_index = hole_down_index;
      static const int hole_next_index = hole_prev_index+1;

      /// data array; could be just a copy
      node_handle* data;
      /// Size of data array.
      node_handle size;
      /// Last used data slot.  Also total number of longs "allocated"
      node_handle last;

  // Holes grid info
  private:
      /// Largest hole ever requested
      node_handle max_request;
      /// List of large holes
      node_handle large_holes;
      /// Pointer to top of holes grid
      node_handle holes_top;
      /// Pointer to bottom of holes grid
      node_handle holes_bottom;
      /// Total ints in holes
      node_handle hole_slots;
      /// Total ints in fragments
      node_handle fragment_slots;

      /// for verifying hole_slots
      static node_handle verify_hole_slots;

  // header sizes; vary by forest.
  private:
      /// Number of slots required for each outgoing edge's value (can be 0).
      char edgeSlots;
      /// Number of slots for extra unhashed data (typically 0).
      char unhashedSlots;
      /// Number of slots for extra hashed data (typically 0).
      char hashedSlots;


  // --------------------------------------------------------
  // |  helpers for inlines.
  private:
      inline node_handle* chunkOf(node_handle addr) const {
        MEDDLY_DCASSERT(data);
        MEDDLY_CHECK_RANGE(1, addr, last+1);
        MEDDLY_DCASSERT(data[addr] >= 0);  // it's not a hole
        return data + addr;
      }
      inline node_handle* holeOf(node_handle addr) const {
        MEDDLY_DCASSERT(data);
        MEDDLY_CHECK_RANGE(1, addr, last+1);
        MEDDLY_DCASSERT(data[addr] < 0);  // it's a hole
        return data + addr;
      }
      inline node_handle& rawSizeOf(node_handle addr) const {
        return chunkOf(addr)[size_index];
      }
      inline node_handle  sizeOf(node_handle addr) const      { return rawSizeOf(addr); }
      inline void setSizeOf(node_handle addr, node_handle sz) { rawSizeOf(addr) = sz; }
      inline node_handle* UH(node_handle addr) const {
          return chunkOf(addr) + headerSlots;
      }
      inline node_handle* HH(node_handle addr) const {
          return UH(addr) + unhashedSlots;
      }
      // full down, as a pointer
      inline node_handle* FD(node_handle addr) const {
          MEDDLY_DCASSERT(rawSizeOf(addr)>0);
          return HH(addr) + hashedSlots;
      }
      // full edges, as a pointer
      inline node_handle* FE(node_handle addr) const {
          return FD(addr) + rawSizeOf(addr);
      }
      // a particular full edge, as a pointer
      inline void* FEP(node_handle addr, int p) const {
        return FE(addr) + p * edgeSlots;
      }
      // sparse down, as a pointer
      inline node_handle* SD(node_handle addr) const {
          MEDDLY_DCASSERT(rawSizeOf(addr)<0);
          return HH(addr) + hashedSlots;
      }
      // sparse indexes, as a pointer
      inline node_handle* SI(node_handle addr) const {
          return SD(addr) - rawSizeOf(addr);  // sparse size is negative
      }
      // sparse edges, as a pointer
      inline node_handle* SE(node_handle addr) const {
          return SD(addr) - 2*rawSizeOf(addr);  // sparse size is negative
      }
      // a particular sparse edge, as a pointer
      inline void* SEP(node_handle addr, int p) const {
        return SE(addr) + p * edgeSlots;
      }
      // binary search for an index
      inline int findSparseIndex(node_handle addr, int i) const {
        int low = 0;
        int nnz = -rawSizeOf(addr);
        MEDDLY_DCASSERT(nnz>=0);
        int high = nnz;
        node_handle* index = SI(addr);
        while (low < high) {
          int z = (low+high)/2;
          MEDDLY_CHECK_RANGE(0, z, nnz);
          if (index[z] == i) return z;
          if (index[z] < i) low = z + 1;
          else              high = z;
        }
        return -1;
      }


  // --------------------------------------------------------
  // |  inlines.
  private:
      /// How many int slots would be required for a node with given size.
      ///   @param  sz  negative for sparse storage, otherwise full.
      inline int slotsForNode(int sz) const {
          int nodeSlots = (sz<0) ? (2+edgeSlots) * (-sz) : (1+edgeSlots) * sz;
          return extraSlots + unhashedSlots + hashedSlots + nodeSlots;
      }


  // --------------------------------------------------------
  // |  Misc. helpers.
  private:

      /** Create a new node, stored as truncated full.
          Space is allocated for the node, and data is copied.
            @param  p     Node handle number.
            @param  size  Number of downward pointers.
            @param  nb    Node data is copied from here.
            @return       The "address" of the new node.
      */
      node_handle makeFullNode(node_handle p, int size, const unpacked_node &nb);

      /** Create a new node, stored sparsely.
          Space is allocated for the node, and data is copied.
            @param  p     Node handle number.
            @param  size  Number of nonzero downward pointers.
            @param  nb    Node data is copied from here.
            @return       The "address" of the new node.
      */
      node_handle makeSparseNode(node_handle p, int size, const unpacked_node &nb);

      void copyExtraHeader(node_address addr, const unpacked_node &nb);

  // --------------------------------------------------------
  // |  Hole management helpers.
  private:
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

      inline node_handle& holeUp(node_handle off)       { return h_up(off); }
      inline node_handle  holeUp(node_handle off) const { return h_up(off); }

      inline node_handle& holeDown(node_handle off)       { return h_down(off); }
      inline node_handle  holeDown(node_handle off) const { return h_down(off); }

      inline node_handle& holePrev(node_handle off)       { return h_prev(off); }
      inline node_handle  holePrev(node_handle off) const { return h_prev(off); }

      inline node_handle& holeNext(node_handle off)       { return h_next(off); }
      inline node_handle  holeNext(node_handle off) const { return h_next(off); }

      inline bool isHoleNonIndex(node_handle p_offset) const {
          return (non_index_hole == h_up(p_offset));
      }

      // returns offset to the hole found in level
      node_handle getHole(int slots);

      // makes a hole of size == slots, at the specified offset
      void makeHole(node_handle p_offset, int slots);

      // add a hole to the hole grid
      void gridInsert(node_handle p_offset);

      // remove a non-index hole from the hole grid
      void midRemove(node_handle p_offset);

      // remove an index hole from the hole grid
      void indexRemove(node_handle p_offset);

      // resize the data array.
      void resize(node_handle new_slots);

      /** Allocate enough slots to store a node with given size.
          Also, stores the node size in the node.
            @param  sz      negative for sparse storage, otherwise full.
            @param  tail    Node id
            @param  clear   Should the node be zeroed.
            @return         Offset in the data array.
      */
      node_handle allocNode(int sz, node_handle tail, bool clear);

      
      /// Find actual number of slots used for this active node.
      inline int activeNodeActualSlots(node_handle addr) const {
          int end = addr + slotsForNode(sizeOf(addr))-1;
          // account for any padding
          if (data[end] < 0) {
            end -= data[end];
          }
          return end - addr + 1;
      }

      /// Find actual number of slots used for this active node.
      inline 
      int activeNodeActualSlots(node_handle addr, node_handle &pad) const {
          int end = addr + slotsForNode(sizeOf(addr))-1;
          // account for any padding
          if (data[end] < 0) {
            pad = -data[end];
            end += pad;
          } else {
            pad = 0;
          }
          return end - addr + 1;
      }

}; 

#endif

