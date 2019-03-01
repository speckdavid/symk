
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


#ifndef COMPACT_STORAGE_H
#define COMPACT_STORAGE_H

// TODO: Testing

#include "../defines.h"
#include "../hash_stream.h"
#include "holeman.h"
#include "bytepack.h"

namespace MEDDLY {
  class compact_storage;

  class compact_grid_style;
};

// ******************************************************************
// *                                                                *
// *                                                                *
// *                     compact_storage  class                     *
// *                                                                *
// *                                                                *
// ******************************************************************

/** New design for node storage in a forest.

    Limits: node size fits in an "int".

    Guarantees: nodes are "node_handle aligned" except for
                portions that are "compacted".

    ------------------------------------------------------------
    Format, "outer", viewed as an array of node_handles:
    ------------------------------------------------------------

        slot[0]:    incoming count, will be >= 0
        slot[1]:    next pointer in unique table, or special value

          ... memory chunk, described below, with padding
          at the end as necessary since the chunk is spread
          over an array of node_handles ...

        slot[M]:    If there is any padding, this will be
                    -padding count (number of extra slots to ignore)

        slot[N]:    tail (node handle number), will be >= 0

        
        Memory chunk:

          int   size  :   node size.  >=0 for truncated full, <0 for sparse
          byte  style :   how to compact the remaining arrays
                  3 bits - (size of pointers in bytes) - 1
                      000: 1 byte pointers
                      001: 2 byte pointers
                      ...
                      111: 8 byte pointers (longs on 64-bit arch.)

                  2 bits - (size of indexes in bytes) - 1
                      00: 1 byte indexes
                      ..
                      11: 4 byte indexes (ints; max node size anyway)

                  3 bits - unused, reserve for edge value compaction?

          unhashed header info: raw bytes
          hashed header info: raw bytes

          compacted downward pointer array (size * #bytes per pointer)
          compacted index array if sparse (size * #bytes per index)
          compacted edge value array (size * #bytes per edge)

          ignored bytes until we reach node_handle boundary


    ------------------------------------------------------------
    Format of "holes", viewed as an array of node_handles
    ------------------------------------------------------------

        slot[0]:    -size of hole,   <0

        slot[1]:  }
        slot[2]:  } used for hole management
        slot[3]:  }

        slot[N]:    -size of hole,   <0

    ------------------------------------------------------------
    
    Minimum chunk size is 5 slots.

*/
class MEDDLY::compact_storage : public node_storage {
  // required interface
  public:
    compact_storage(expert_forest* f, holeman* hm, const char* sN);
    virtual ~compact_storage();

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
    virtual void fillUnpacked(unpacked_node &nr, node_address addr, unpacked_node::storage_style) const;
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

  // --------------------------------------------------------
  // |  Misc. helpers.
  private:
      /** Create a new node, stored as truncated full.
          Space is allocated for the node, and data is copied.
            @param  p       Node handle number.
            @param  size    Number of downward pointers.
            @param  pbytes  Number of bytes to use for each pointer.
            @param  nb      Node data is copied from here.
            @return         The "address" of the new node.
      */
      node_address makeFullNode(node_handle p, int size, 
          int pbytes, const unpacked_node &nb);

      /** Create a new node, stored sparsely.
          Space is allocated for the node, and data is copied.
            @param  p       Node handle number.
            @param  size    Number of nonzero downward pointers.
            @param  pbytes  Number of bytes to use for each pointer.
            @param  ibytes  Number of bytes to use for each index.
            @param  nb      Node data is copied from here.
            @return         The "address" of the new node.
      */
      node_address makeSparseNode(node_handle p, int size, 
          int pbytes, int ibytes, const unpacked_node &nb);


      /** Allocate a new chunk of requested size or larger.
            @param  slots   number of integer slots requested
            @param  tail    Node id
            @param  clear   Should the node be zeroed.
            @return         Offset in the data array.
      */
      node_address allocNode(int slots, node_handle tail, bool clear);

  // --------------------------------------------------------
  // |  inlines.
  private:
      inline static int bytesToSlots(int bytes) {
        // works for bytes > 0
        return 1 + ((bytes-1) / sizeof(node_handle));
      }

      /// How many integer slots would be required for a node with given size.
      ///   @param  sz  negative for sparse storage, otherwise full.
      ///   @param  pb  bytes per pointer
      ///   @param  ib  bytes per index
      inline int slotsForNode(int sz, int pb, int ib) const {
          int nodebytes = (sz<0)  ? (edgeBytes+pb+ib) * (-sz) 
                                  : (edgeBytes+pb) * sz;
          nodebytes += unhashedBytes + hashedBytes 
                        + sizeof(int)   // the integer size
                        + 1;            // the style byte
          return extraSlots + bytesToSlots(nodebytes);
      }


      /// Find the actual number of slots used for this active node
      inline int activeNodeActualSlots(node_address a) const {
        int pb, ib;
        getStyleOf(a, pb, ib);
        int end = a + slotsForNode(sizeOf(a), pb, ib)-1;
        // account for any padding
        if (data[end] < 0) {
          end -= data[end];
        }
        return end - a + 1;
      }

      inline void copyExtraHeader(node_address addr, const unpacked_node &nb) {
        // copy extra header info, if any
        if (unhashedBytes) {
          memcpy(UH(addr), nb.UHptr(), nb.UHbytes());
        }
        if (hashedBytes) {
          memcpy(HH(addr), nb.HHptr(), nb.HHbytes());
        }
      }


      //--------------------------------------------------------------
      // Helpers - building full from full
      //--------------------------------------------------------------

      template <int pbytes>
      inline node_address 
      copyFullIntoFull(const unpacked_node &nb, int size, node_address addr)
      {
        MEDDLY_DCASSERT(nb.isFull());
        unsigned char* down = fullDown(addr);
        for (int i=0; i<size; i++) {
          downToData<pbytes>(nb.d(i), down);
          down += pbytes;
        }
        if (edgeBytes) {
          memcpy(fullEdge(addr), nb.eptr(0), size * edgeBytes);
        } 
        return addr;
      }

      inline node_address copyFullIntoFull(int pbytes, 
        const unpacked_node &nb, int size, node_address addr)
      {
        switch (pbytes) {
            case 1:   return  copyFullIntoFull<1>(nb, size, addr);
            case 2:   return  copyFullIntoFull<2>(nb, size, addr);
            case 3:   return  copyFullIntoFull<3>(nb, size, addr);
            case 4:   return  copyFullIntoFull<4>(nb, size, addr);
            case 5:   return  copyFullIntoFull<5>(nb, size, addr);
            case 6:   return  copyFullIntoFull<6>(nb, size, addr);
            case 7:   return  copyFullIntoFull<7>(nb, size, addr);
            case 8:   return  copyFullIntoFull<8>(nb, size, addr);
            default:
                MEDDLY_DCASSERT(0);
                throw error(error::MISCELLANEOUS);
        }
      }

      //--------------------------------------------------------------
      // Helpers - building full from sparse
      //--------------------------------------------------------------

      template <int pbytes>
      inline node_address
      copySparseIntoFull(const unpacked_node &nb, int size, node_address addr)
      {
        MEDDLY_DCASSERT(nb.isSparse());
        unsigned char* down = fullDown(addr);
        if (edgeBytes) {
          unsigned char* edge = fullEdge(addr);
          for (int z=0; z<nb.getNNZs(); z++) {
            int i = nb.i(z);
            MEDDLY_CHECK_RANGE(0, i, size);
            downToData<pbytes>(nb.d(z), down + i*pbytes);
            memcpy(edge + i*edgeBytes, nb.eptr(z), edgeBytes);
          } // for z
        } else {
          for (int z=0; z<nb.getNNZs(); z++) {
            int i = nb.i(z);
            MEDDLY_CHECK_RANGE(0, i, size);
            downToData<pbytes>(nb.d(z), down + i*pbytes);
          } // for z
        }
        return addr;
      }

      inline node_address copySparseIntoFull(int pbytes, 
        const unpacked_node &nb, int size, node_address addr)
      {
        switch (pbytes) {
            case 1:   return  copySparseIntoFull<1>(nb, size, addr);
            case 2:   return  copySparseIntoFull<2>(nb, size, addr);
            case 3:   return  copySparseIntoFull<3>(nb, size, addr);
            case 4:   return  copySparseIntoFull<4>(nb, size, addr);
            case 5:   return  copySparseIntoFull<5>(nb, size, addr);
            case 6:   return  copySparseIntoFull<6>(nb, size, addr);
            case 7:   return  copySparseIntoFull<7>(nb, size, addr);
            case 8:   return  copySparseIntoFull<8>(nb, size, addr);
            default:
                MEDDLY_DCASSERT(0);
                throw error(error::MISCELLANEOUS);
        }
      }

      //--------------------------------------------------------------
      // Helpers - building sparse from sparse
      //--------------------------------------------------------------

      template <int pbytes, int ibytes>
      inline node_address 
      copySparseIntoSparse(const unpacked_node &nb, int size, node_address addr)
      {
        MEDDLY_DCASSERT(nb.isSparse());
        unsigned char* down = sparseDown(addr);
        unsigned char* index = sparseIndex(addr);
        for (int z=0; z<size; z++) {
          downToData<pbytes>(nb.d(z), down);
          down += pbytes;
          rawToData<ibytes>(nb.i(z), index);
          index += ibytes;
        }
        if (edgeBytes) {
          memcpy(sparseEdge(addr), nb.eptr(0), size * edgeBytes);
        } 
        return addr;
      }

      template <int pbytes>
      inline node_address
      copySparseIntoSparse(int ibytes, const unpacked_node &nb, 
        int size, node_address addr)
      {
        switch (ibytes) {
            case 1:   return  copySparseIntoSparse<pbytes, 1>(nb, size, addr);
            case 2:   return  copySparseIntoSparse<pbytes, 2>(nb, size, addr);
            case 3:   return  copySparseIntoSparse<pbytes, 3>(nb, size, addr);
            case 4:   return  copySparseIntoSparse<pbytes, 4>(nb, size, addr);
            default:
                MEDDLY_DCASSERT(0);
                throw error(error::MISCELLANEOUS);
        }
      }

      inline node_address
      copySparseIntoSparse(int pbytes, int ibytes, const unpacked_node &nb, 
        int size, node_address addr)
      {
        switch (pbytes) {
            case 1:   return  copySparseIntoSparse<1>(ibytes, nb, size, addr);
            case 2:   return  copySparseIntoSparse<2>(ibytes, nb, size, addr);
            case 3:   return  copySparseIntoSparse<3>(ibytes, nb, size, addr);
            case 4:   return  copySparseIntoSparse<4>(ibytes, nb, size, addr);
            case 5:   return  copySparseIntoSparse<5>(ibytes, nb, size, addr);
            case 6:   return  copySparseIntoSparse<6>(ibytes, nb, size, addr);
            case 7:   return  copySparseIntoSparse<7>(ibytes, nb, size, addr);
            case 8:   return  copySparseIntoSparse<8>(ibytes, nb, size, addr);
            default:
                MEDDLY_DCASSERT(0);
                throw error(error::MISCELLANEOUS);
        }
      }


      //--------------------------------------------------------------
      // Helpers - building sparse from full
      //--------------------------------------------------------------

      template <int pbytes, int ibytes>
      inline node_address 
      copyFullIntoSparse(const unpacked_node &nb, node_address addr)
      {
        MEDDLY_DCASSERT(nb.isFull());
        unsigned char* down = sparseDown(addr);
        unsigned char* index = sparseIndex(addr);
        if (edgeBytes) {
          unsigned char* edge = sparseEdge(addr);
          for (int i=0; i<nb.getSize(); i++) if (nb.d(i)) {
            downToData<pbytes>(nb.d(i), down);
            down += pbytes;
            rawToData<ibytes>(i, index);
            index += ibytes;
            memcpy(edge, nb.eptr(i), edgeBytes);
            edge += edgeBytes;
          }
        } else {
          for (int i=0; i<nb.getSize(); i++) if (nb.d(i)) {
            downToData<pbytes>(nb.d(i), down);
            down += pbytes;
            rawToData<ibytes>(i, index);
            index += ibytes;
          }
        }
        return addr;
      }


      template <int pbytes>
      inline node_address
      copyFullIntoSparse(int ibytes, const unpacked_node &nb, node_address addr)
      {
        switch (ibytes) {
            case 1:   return  copyFullIntoSparse<pbytes, 1>(nb, addr);
            case 2:   return  copyFullIntoSparse<pbytes, 2>(nb, addr);
            case 3:   return  copyFullIntoSparse<pbytes, 3>(nb, addr);
            case 4:   return  copyFullIntoSparse<pbytes, 4>(nb, addr);
            default:
                MEDDLY_DCASSERT(0);
                throw error(error::MISCELLANEOUS);
        }
      }

      inline node_address
      copyFullIntoSparse(int pbytes, int ibytes, const unpacked_node &nb, 
        node_address addr)
      {
        switch (pbytes) {
            case 1:   return  copyFullIntoSparse<1>(ibytes, nb, addr);
            case 2:   return  copyFullIntoSparse<2>(ibytes, nb, addr);
            case 3:   return  copyFullIntoSparse<3>(ibytes, nb, addr);
            case 4:   return  copyFullIntoSparse<4>(ibytes, nb, addr);
            case 5:   return  copyFullIntoSparse<5>(ibytes, nb, addr);
            case 6:   return  copyFullIntoSparse<6>(ibytes, nb, addr);
            case 7:   return  copyFullIntoSparse<7>(ibytes, nb, addr);
            case 8:   return  copyFullIntoSparse<8>(ibytes, nb, addr);
            default:
                MEDDLY_DCASSERT(0);
                throw error(error::MISCELLANEOUS);
        }
      }


      //--------------------------------------------------------------
      // Helpers - unlink down pointers
      //--------------------------------------------------------------
      template <int pbytes>
      inline void unlinkDown(unsigned char* down, int size) {
        for (int i=0; i<size; i++) {
          node_handle downval;
          dataToDown<pbytes>(down, downval);
          down += pbytes;
          getParent()->unlinkNode(downval);
        }
      }

      inline void unlinkDown(int pbytes, unsigned char* down, int size) {
          switch (pbytes) {
              case 1:   return  unlinkDown<1>(down, size);
              case 2:   return  unlinkDown<2>(down, size);
              case 3:   return  unlinkDown<3>(down, size);
              case 4:   return  unlinkDown<4>(down, size);
              case 5:   return  unlinkDown<5>(down, size);
              case 6:   return  unlinkDown<6>(down, size);
              case 7:   return  unlinkDown<7>(down, size);
              case 8:   return  unlinkDown<8>(down, size);
              default:
                  MEDDLY_DCASSERT(0);
                  throw error(error::MISCELLANEOUS);
          }
      }

      //--------------------------------------------------------------
      //--------------------------------------------------------------
      // Helpers for areDuplicates
      //--------------------------------------------------------------
      //--------------------------------------------------------------
      template <class nodetype>
      inline bool areDupsTempl(node_address addr, const nodetype &n) const 
      {
        if (n.HHbytes()) {
          if (memcmp(HH(addr), n.HHptr(), n.HHbytes())) return false;
        }

        if (n.UHbytes()) {
          if (memcmp(UH(addr), n.UHptr(), n.UHbytes())) return false;
        }
        
        int size = sizeOf(addr);
        if (size<0) {
          // Node is sparse
          if (n.isFull()) {
            return sparseEqualsFull(addr, -size, n);
          } else {
            return sparseEqualsSparse(addr, -size, n);
          }
        } else {
          // Node is full
          if (n.isFull()) {
            return fullEqualsFull(addr, size, n);
          } else {
            return fullEqualsSparse(addr, size, n);
          }
        }
      }

      //--------------------------------------------------------------
      // Helpers - sparse = full
      //--------------------------------------------------------------

      template <int pbytes, int ibytes, class nodetype>
      inline bool 
      sparseEqualsFull(node_address addr, int nnzs, const nodetype &n) const 
      {
        MEDDLY_DCASSERT(n.isFull());
        MEDDLY_DCASSERT(sizeOf(addr) < 0);
        MEDDLY_DCASSERT(sizeOf(addr) == -nnzs);

        const unsigned char* down = sparseDown(addr);
        const unsigned char* index = sparseIndex(addr);
        int i = 0;
        if (n.hasEdges()) {
          MEDDLY_DCASSERT(edgeBytes == n.edgeBytes());
          const unsigned char* edge = sparseEdge(addr);
          for (int z=0; z<nnzs; z++) {
            int indz;
            dataToUnsigned<ibytes>(index, indz);
            index += ibytes;
            if (indz >= n.getSize()) return false;
            for (; i<indz; i++) if (n.d(i)) return false;
            node_handle dv;
            dataToDown<pbytes>(down, dv);
            down += pbytes;
            if (n.d(i) != dv) return false;
            if (!getParent()->areEdgeValuesEqual(edge, n.eptr(indz))) 
              return false;
            edge += edgeBytes;
            i++;
          } // for z
        } else {
          for (int z=0; z<nnzs; z++) {
            int indz;
            dataToUnsigned<ibytes>(index, indz);
            index += ibytes;
            if (indz >= n.getSize()) return false;
            for (; i<indz; i++) if (n.d(i)) return false;
            node_handle dv;
            dataToDown<pbytes>(down, dv);
            down += pbytes;
            if (n.d(i) != dv) return false;
            i++;
          } // for z
        }

        for (; i<n.getSize(); i++) if (n.d(i)) return false;
        return true;
      }

      template <int pbytes, class nodetype>
      inline bool sparseEqualsFull(int ibytes, node_address addr,
        int nnzs, const nodetype &n) const
      {
        switch (ibytes) {
            case 1:   return  sparseEqualsFull<pbytes, 1>(addr, nnzs, n);
            case 2:   return  sparseEqualsFull<pbytes, 2>(addr, nnzs, n);
            case 3:   return  sparseEqualsFull<pbytes, 3>(addr, nnzs, n);
            case 4:   return  sparseEqualsFull<pbytes, 4>(addr, nnzs, n);
            default:
                MEDDLY_DCASSERT(0);
                throw error(error::MISCELLANEOUS);
        }
      }

      template <class nodetype>
      inline bool 
      sparseEqualsFull(node_address addr, int nnzs, const nodetype &n) const 
      {
        int pbytes, ibytes;
        getStyleOf(addr, pbytes, ibytes);
        switch (pbytes) {
            case 1:   return sparseEqualsFull<1>(ibytes, addr, nnzs, n);
            case 2:   return sparseEqualsFull<2>(ibytes, addr, nnzs, n);
            case 3:   return sparseEqualsFull<3>(ibytes, addr, nnzs, n);
            case 4:   return sparseEqualsFull<4>(ibytes, addr, nnzs, n);
            case 5:   return sparseEqualsFull<5>(ibytes, addr, nnzs, n);
            case 6:   return sparseEqualsFull<6>(ibytes, addr, nnzs, n);
            case 7:   return sparseEqualsFull<7>(ibytes, addr, nnzs, n);
            case 8:   return sparseEqualsFull<8>(ibytes, addr, nnzs, n);
            default:
                MEDDLY_DCASSERT(0);
                throw error(error::MISCELLANEOUS);
        }
      }

      //--------------------------------------------------------------
      // Helpers - sparse = sparse
      //--------------------------------------------------------------

      template <int pbytes, int ibytes, class nodetype>
      inline bool 
      sparseEqualsSparse(node_address addr, int nnzs, const nodetype &n) const 
      {
        MEDDLY_DCASSERT(n.isSparse());
        MEDDLY_DCASSERT(sizeOf(addr) < 0);
        MEDDLY_DCASSERT(sizeOf(addr) == -nnzs);

        const unsigned char* down = sparseDown(addr);
        const unsigned char* index = sparseIndex(addr);
        if (n.hasEdges()) {
          MEDDLY_DCASSERT(edgeBytes == n.edgeBytes());
          const unsigned char* edge = sparseEdge(addr);
          for (int z=0; z<nnzs; z++) {
            node_handle dv;
            dataToDown<pbytes>(down, dv);
            down += pbytes;
            if (dv != n.d(z)) return false;
            MEDDLY_DCASSERT(dv);
            int iv;
            dataToUnsigned<ibytes>(index, iv);
            index += ibytes;
            if (iv != n.i(z)) return false;
            if (!getParent()->areEdgeValuesEqual(edge, n.eptr(z))) {
              return false;
            }
            edge += edgeBytes;
          } // for z
        } else {
          for (int z=0; z<nnzs; z++) {
            node_handle dv;
            dataToDown<pbytes>(down, dv);
            down += pbytes;
            if (dv != n.d(z)) return false;
            MEDDLY_DCASSERT(dv);
            int iv;
            dataToUnsigned<ibytes>(index, iv);
            index += ibytes;
            if (iv != n.i(z)) return false;
          } // for z
        }
        return true;
      }

      template <int pbytes, class nodetype>
      inline bool sparseEqualsSparse(int ibytes, node_address addr,
        int nnzs, const nodetype &n) const
      {
        switch (ibytes) {
            case 1:   return  sparseEqualsSparse<pbytes, 1>(addr, nnzs, n);
            case 2:   return  sparseEqualsSparse<pbytes, 2>(addr, nnzs, n);
            case 3:   return  sparseEqualsSparse<pbytes, 3>(addr, nnzs, n);
            case 4:   return  sparseEqualsSparse<pbytes, 4>(addr, nnzs, n);
            default:
                MEDDLY_DCASSERT(0);
                throw error(error::MISCELLANEOUS);
        }
      }

      template <class nodetype>
      inline bool 
      sparseEqualsSparse(node_address addr, int nnzs, const nodetype &n) const 
      {
        if (n.getNNZs() != nnzs) return false;

        int pbytes, ibytes;
        getStyleOf(addr, pbytes, ibytes);
        switch (pbytes) {
            case 1:   return sparseEqualsSparse<1>(ibytes, addr, nnzs, n);
            case 2:   return sparseEqualsSparse<2>(ibytes, addr, nnzs, n);
            case 3:   return sparseEqualsSparse<3>(ibytes, addr, nnzs, n);
            case 4:   return sparseEqualsSparse<4>(ibytes, addr, nnzs, n);
            case 5:   return sparseEqualsSparse<5>(ibytes, addr, nnzs, n);
            case 6:   return sparseEqualsSparse<6>(ibytes, addr, nnzs, n);
            case 7:   return sparseEqualsSparse<7>(ibytes, addr, nnzs, n);
            case 8:   return sparseEqualsSparse<8>(ibytes, addr, nnzs, n);
            default:
                MEDDLY_DCASSERT(0);
                throw error(error::MISCELLANEOUS);
        }
      }

      //--------------------------------------------------------------
      // Helpers - full = full
      //--------------------------------------------------------------

      template <int pbytes, class nodetype>
      inline bool 
      fullEqualsFull(node_address addr, int size, const nodetype &n) const 
      {
        MEDDLY_DCASSERT(n.isFull());
        MEDDLY_DCASSERT(sizeOf(addr) >= 0);
        MEDDLY_DCASSERT(sizeOf(addr) == size);

        const unsigned char* down = fullDown(addr);
        int i;
        if (n.hasEdges()) {
          MEDDLY_DCASSERT(edgeBytes == n.edgeBytes());
          const unsigned char* edge = fullEdge(addr);
          for (i=0; i<size; i++) {
            node_handle dv;
            dataToDown<pbytes>(down, dv);
            down += pbytes;
            if (dv != n.d(i)) return false;
            if (dv) {
              if (!getParent()->areEdgeValuesEqual(edge, n.eptr(i))) {
                return false;
              }
            }
            edge += edgeBytes;
          } // for i
        } else {
          for (i=0; i<size; i++) {
            node_handle dv;
            dataToDown<pbytes>(down, dv);
            down += pbytes;
            if (dv != n.d(i)) return false;
          } // for i
        }
        // check any extra in n
        for (; i<n.getSize(); i++) {
          if (n.d(i)) return false;
        }
        return true;
      }

      template <class nodetype>
      inline bool 
      fullEqualsFull(node_address addr, int size, const nodetype &n) const 
      {
          if (size > n.getSize()) return false;

          switch (pointerBytesOf(addr)) {
              case 1:   return fullEqualsFull<1>(addr, size, n);
              case 2:   return fullEqualsFull<2>(addr, size, n);
              case 3:   return fullEqualsFull<3>(addr, size, n);
              case 4:   return fullEqualsFull<4>(addr, size, n);
              case 5:   return fullEqualsFull<5>(addr, size, n);
              case 6:   return fullEqualsFull<6>(addr, size, n);
              case 7:   return fullEqualsFull<7>(addr, size, n);
              case 8:   return fullEqualsFull<8>(addr, size, n);
              default:
                  MEDDLY_DCASSERT(0);
                  throw error(error::MISCELLANEOUS);
          }
      }

      //--------------------------------------------------------------
      // Helpers - full = sparse
      //--------------------------------------------------------------

      template <int pbytes, class nodetype>
      inline bool 
      fullEqualsSparse(node_address addr, int size, const nodetype &n) const 
      {
        MEDDLY_DCASSERT(n.isSparse());
        MEDDLY_DCASSERT(sizeOf(addr) >= 0);
        MEDDLY_DCASSERT(sizeOf(addr) == size);
        const unsigned char* down = fullDown(addr);
        // check down
        int i = 0;
        for (int z=0; z<n.getNNZs(); z++) {
          node_handle dv;
          if (n.i(z) >= size) return false;
          for (; i<n.i(z); i++) {
            dataToDown<pbytes>(down, dv);
            down += pbytes;
            if (dv) return false;
          }
          dataToDown<pbytes>(down, dv);
          down += pbytes;
          if (n.d(z) != dv) return false;
          i++;
        } // for z
        if (i<size) return false;
        // check edges
        if (n.hasEdges()) {
          MEDDLY_DCASSERT(edgeBytes == n.edgeBytes());
          const unsigned char* edge = fullEdge(addr);
          for (int z=0; z<n.getNNZs(); z++) {
            if (!getParent()->areEdgeValuesEqual(
              edge + n.i(z) * edgeBytes, n.eptr(z)
            )) return false;
          }
        }
        return true;
      }

      template <class nodetype>
      inline bool 
      fullEqualsSparse(node_address addr, int size, const nodetype &n) const 
      {
          switch (pointerBytesOf(addr)) {
              case 1:   return fullEqualsSparse<1>(addr, size, n);
              case 2:   return fullEqualsSparse<2>(addr, size, n);
              case 3:   return fullEqualsSparse<3>(addr, size, n);
              case 4:   return fullEqualsSparse<4>(addr, size, n);
              case 5:   return fullEqualsSparse<5>(addr, size, n);
              case 6:   return fullEqualsSparse<6>(addr, size, n);
              case 7:   return fullEqualsSparse<7>(addr, size, n);
              case 8:   return fullEqualsSparse<8>(addr, size, n);
              default:
                  MEDDLY_DCASSERT(0);
                  throw error(error::MISCELLANEOUS);
          }
      }


      //--------------------------------------------------------------
      //--------------------------------------------------------------
      // Helpers for fillReader
      //--------------------------------------------------------------
      //--------------------------------------------------------------

      //--------------------------------------------------------------
      // Helpers - read full from full
      //--------------------------------------------------------------
      template <int pbytes>
      inline void 
      readFullFromFull(node_address addr, int size, unpacked_node &nr, bool shrink) const
      {
        MEDDLY_DCASSERT(nr.isFull());
        MEDDLY_DCASSERT(sizeOf(addr) >= 0);
        MEDDLY_DCASSERT(sizeOf(addr) == size);
        MEDDLY_DCASSERT(edgeBytes == nr.edgeBytes());
        unsigned char* down = fullDown(addr);
        unsigned char* edge = fullEdge(addr);
        int i;
        for (i=0; i<size; i++) {
          dataToDown<pbytes>(down, nr.d_ref(i));
          down += pbytes;
          if (edgeBytes) {
            memcpy(nr.eptr_write(i), edge, edgeBytes);
            edge += edgeBytes;
          }
        } 
        if (shrink) {
          nr.shrinkFull(size);
        } else {
          for (; i<nr.getSize(); i++) {
            nr.d_ref(i) = 0;
            if (edgeBytes) {
              memset(nr.eptr_write(i), 0, edgeBytes);
            }
          }
        }
      }

      inline void readFullFromFull(int pbytes, node_address addr, int size, 
        unpacked_node &nr, bool shrink) const
      {
          switch (pbytes) {
              case 1:   return readFullFromFull<1>(addr, size, nr, shrink);
              case 2:   return readFullFromFull<2>(addr, size, nr, shrink);
              case 3:   return readFullFromFull<3>(addr, size, nr, shrink);
              case 4:   return readFullFromFull<4>(addr, size, nr, shrink);
              case 5:   return readFullFromFull<5>(addr, size, nr, shrink);
              case 6:   return readFullFromFull<6>(addr, size, nr, shrink);
              case 7:   return readFullFromFull<7>(addr, size, nr, shrink);
              case 8:   return readFullFromFull<8>(addr, size, nr, shrink);
              default:
                  MEDDLY_DCASSERT(0);
                  throw error(error::MISCELLANEOUS);
          }
      }

      //--------------------------------------------------------------
      // Helpers - read sparse from full
      //--------------------------------------------------------------
      template <int pbytes>
      inline void 
      readSparseFromFull(node_address addr, int size, unpacked_node &nr) const
      {
        MEDDLY_DCASSERT(nr.isSparse());
        MEDDLY_DCASSERT(sizeOf(addr) >= 0);
        MEDDLY_DCASSERT(sizeOf(addr) == size);
        unsigned char* down = fullDown(addr);

        int z = 0;
        if (nr.hasEdges()) {
          unsigned char* edge = fullEdge(addr);
          for (int i=0; i<size; i++) {
            dataToDown<pbytes>(down, nr.d_ref(z));
            down += pbytes;
            if (nr.d(z)) {
              nr.i_ref(z) = i;
              memcpy(nr.eptr_write(z), edge, edgeBytes);
              z++;
            }
            edge += edgeBytes;
          } // for i
        } else {
          for (int i=0; i<size; i++) {
            dataToDown<pbytes>(down, nr.d_ref(z));
            down += pbytes;
            if (nr.d(z)) {
              nr.i_ref(z) = i;
              z++;
            }
          } // for i
        }
        nr.shrinkSparse(z);
      }

      inline void readSparseFromFull(int pbytes, node_address addr, int size, 
        unpacked_node &nr) const
      {
          switch (pbytes) {
              case 1:   return readSparseFromFull<1>(addr, size, nr);
              case 2:   return readSparseFromFull<2>(addr, size, nr);
              case 3:   return readSparseFromFull<3>(addr, size, nr);
              case 4:   return readSparseFromFull<4>(addr, size, nr);
              case 5:   return readSparseFromFull<5>(addr, size, nr);
              case 6:   return readSparseFromFull<6>(addr, size, nr);
              case 7:   return readSparseFromFull<7>(addr, size, nr);
              case 8:   return readSparseFromFull<8>(addr, size, nr);
              default:
                  MEDDLY_DCASSERT(0);
                  throw error(error::MISCELLANEOUS);
          }
      }

      //--------------------------------------------------------------
      // Helpers - read full from sparse
      //--------------------------------------------------------------
      template <int pbytes, int ibytes>
      inline void 
      readFullFromSparse(node_address addr, int nnzs, unpacked_node &nr) const
      {
        MEDDLY_DCASSERT(nr.isFull());
        MEDDLY_DCASSERT(sizeOf(addr) < 0);
        MEDDLY_DCASSERT(sizeOf(addr) == -nnzs);
        MEDDLY_DCASSERT(nr.edgeBytes() == edgeBytes);

        for (int i=0; i<nr.getSize(); i++) {
          nr.d_ref(i) = 0;
          if (nr.hasEdges()) {
            memset(nr.eptr_write(i), 0, nr.edgeBytes());
          }
        }

        unsigned char* down = sparseDown(addr);
        unsigned char* index = sparseIndex(addr);
        unsigned char* edge = sparseEdge(addr);

        for (int z=0; z<nnzs; z++) {
          int i;
          dataToUnsigned<ibytes>(index, i);
          index += ibytes;
          dataToDown<pbytes>(down, nr.d_ref(i));
          down += pbytes;
          if (nr.hasEdges()) {
            memcpy(nr.eptr_write(i), edge, edgeBytes);
            edge += edgeBytes;
          }
        }
        
      }

      template <int pbytes>
      inline void readFullFromSparse(int ibytes, node_address addr, 
        int nnzs, unpacked_node &nr) const
      {
        switch (ibytes) {
            case 1:   return  readFullFromSparse<pbytes, 1>(addr, nnzs, nr);
            case 2:   return  readFullFromSparse<pbytes, 2>(addr, nnzs, nr);
            case 3:   return  readFullFromSparse<pbytes, 3>(addr, nnzs, nr);
            case 4:   return  readFullFromSparse<pbytes, 4>(addr, nnzs, nr);
            default:
                MEDDLY_DCASSERT(0);
                throw error(error::MISCELLANEOUS);
        }
      }

      inline void readFullFromSparse(int pbytes, int ibytes, 
        node_address addr, int nnzs, unpacked_node &nr) const
      {
          switch (pbytes) {
              case 1:   return readFullFromSparse<1>(ibytes, addr, nnzs, nr);
              case 2:   return readFullFromSparse<2>(ibytes, addr, nnzs, nr);
              case 3:   return readFullFromSparse<3>(ibytes, addr, nnzs, nr);
              case 4:   return readFullFromSparse<4>(ibytes, addr, nnzs, nr);
              case 5:   return readFullFromSparse<5>(ibytes, addr, nnzs, nr);
              case 6:   return readFullFromSparse<6>(ibytes, addr, nnzs, nr);
              case 7:   return readFullFromSparse<7>(ibytes, addr, nnzs, nr);
              case 8:   return readFullFromSparse<8>(ibytes, addr, nnzs, nr);
              default:
                  MEDDLY_DCASSERT(0);
                  throw error(error::MISCELLANEOUS);
          }
      }


      //--------------------------------------------------------------
      // Helpers - read sparse from sparse
      //--------------------------------------------------------------
      template <int pbytes, int ibytes>
      inline void 
      readSparseFromSparse(node_address addr, int nnzs, unpacked_node &nr) const
      {
        MEDDLY_DCASSERT(nr.isSparse());
        MEDDLY_DCASSERT(sizeOf(addr) < 0);
        MEDDLY_DCASSERT(sizeOf(addr) == -nnzs);
        MEDDLY_DCASSERT(nr.edgeBytes() == edgeBytes);

        unsigned char* down = sparseDown(addr);
        unsigned char* index = sparseIndex(addr);

        for (int z=0; z<nnzs; z++) {
          dataToDown<pbytes>(down, nr.d_ref(z));
          down += pbytes;
          dataToUnsigned<ibytes>(index, nr.i_ref(z));
          index += ibytes;
        }

        if (nr.hasEdges()) {
          MEDDLY_DCASSERT(edgeBytes>0);
          unsigned char* edge = sparseEdge(addr);
          for (int z=0; z<nnzs; z++) {
            memcpy(nr.eptr_write(z), edge, edgeBytes);
            edge += edgeBytes;
          } 
        }

        nr.shrinkSparse(nnzs);
      }

      template <int pbytes>
      inline void readSparseFromSparse(int ibytes, node_address addr, 
        int nnzs, unpacked_node &nr) const
      {
        switch (ibytes) {
            case 1:   return  readSparseFromSparse<pbytes, 1>(addr, nnzs, nr);
            case 2:   return  readSparseFromSparse<pbytes, 2>(addr, nnzs, nr);
            case 3:   return  readSparseFromSparse<pbytes, 3>(addr, nnzs, nr);
            case 4:   return  readSparseFromSparse<pbytes, 4>(addr, nnzs, nr);
            default:
                MEDDLY_DCASSERT(0);
                throw error(error::MISCELLANEOUS);
        }
      }

      inline void readSparseFromSparse(int pbytes, int ibytes, 
        node_address addr, int nnzs, unpacked_node &nr) const
      {
          switch (pbytes) {
              case 1:   return readSparseFromSparse<1>(ibytes, addr, nnzs, nr);
              case 2:   return readSparseFromSparse<2>(ibytes, addr, nnzs, nr);
              case 3:   return readSparseFromSparse<3>(ibytes, addr, nnzs, nr);
              case 4:   return readSparseFromSparse<4>(ibytes, addr, nnzs, nr);
              case 5:   return readSparseFromSparse<5>(ibytes, addr, nnzs, nr);
              case 6:   return readSparseFromSparse<6>(ibytes, addr, nnzs, nr);
              case 7:   return readSparseFromSparse<7>(ibytes, addr, nnzs, nr);
              case 8:   return readSparseFromSparse<8>(ibytes, addr, nnzs, nr);
              default:
                  MEDDLY_DCASSERT(0);
                  throw error(error::MISCELLANEOUS);
          }
      }

      //--------------------------------------------------------------
      // Helpers - hashSparse
      //--------------------------------------------------------------
      template <int pbytes, int ibytes>
      inline void hashSparse(hash_stream& s, node_address addr, int nnzs) const
      {
        const unsigned char* down = sparseDown(addr);
        const unsigned char* index = sparseIndex(addr);
        if (getParent()->areEdgeValuesHashed()) {
          MEDDLY_DCASSERT(edgeBytes>0);
          const unsigned char* edge = sparseEdge(addr);
          for (int z=0; z<nnzs; z++) {
            node_handle dv;
            dataToDown<pbytes>(down, dv);
            down += pbytes;
            int iv;
            dataToUnsigned<ibytes>(index, iv);
            index += ibytes;
            s.push(iv, dv, ((int*)edge)[0]);
            edge += edgeBytes;
          } // for z
        } else {
          for (int z=0; z<nnzs; z++) {
            node_handle dv;
            dataToDown<pbytes>(down, dv);
            down += pbytes;
            int iv;
            dataToUnsigned<ibytes>(index, iv);
            index += ibytes;
            s.push(iv, dv);
          } // for z
        }
      }

      template <int pbytes>
      inline void hashSparse(int ibytes, hash_stream& s, node_address addr, 
        int nnzs) const
      {
        switch (ibytes) {
            case 1:   return  hashSparse<pbytes, 1>(s, addr, nnzs);
            case 2:   return  hashSparse<pbytes, 2>(s, addr, nnzs);
            case 3:   return  hashSparse<pbytes, 3>(s, addr, nnzs);
            case 4:   return  hashSparse<pbytes, 4>(s, addr, nnzs);
            default:
                MEDDLY_DCASSERT(0);
                throw error(error::MISCELLANEOUS);
        }
      }
      
      inline void hashSparse(hash_stream& s, node_address addr, int nnzs) const
      {
        int pbytes, ibytes;
        getStyleOf(addr, pbytes, ibytes);
        switch (pbytes) {
            case 1:   return hashSparse<1>(ibytes, s, addr, nnzs);
            case 2:   return hashSparse<2>(ibytes, s, addr, nnzs);
            case 3:   return hashSparse<3>(ibytes, s, addr, nnzs);
            case 4:   return hashSparse<4>(ibytes, s, addr, nnzs);
            case 5:   return hashSparse<5>(ibytes, s, addr, nnzs);
            case 6:   return hashSparse<6>(ibytes, s, addr, nnzs);
            case 7:   return hashSparse<7>(ibytes, s, addr, nnzs);
            case 8:   return hashSparse<8>(ibytes, s, addr, nnzs);
            default:
                MEDDLY_DCASSERT(0);
                throw error(error::MISCELLANEOUS);
        }
      }

      //--------------------------------------------------------------
      // Helpers - hashFull
      //--------------------------------------------------------------

      template <int pbytes>
      inline void hashFull(hash_stream& s, node_address addr, int size) const
      {
        const unsigned char* down = fullDown(addr);
        if (getParent()->areEdgeValuesHashed()) {
          MEDDLY_DCASSERT(edgeBytes>0);
          const unsigned char* edge = fullEdge(addr);
          for (int i=0; i<size; i++) {
            node_handle dv;
            dataToDown<pbytes>(down, dv);
            down += pbytes;
            if (dv) {
              s.push(i, dv, ((int*)edge)[0]);
            }
            edge += edgeBytes;
          } // for z
        } else {
          for (int i=0; i<size; i++) {
            node_handle dv;
            dataToDown<pbytes>(down, dv);
            down += pbytes;
            if (dv) {
              s.push(i, dv);
            }
          } // for z
        }
      }

      inline void hashFull(hash_stream& s, node_address addr, int size) const
      {
        switch (pointerBytesOf(addr)) {
            case 1:   return hashFull<1>(s, addr, size);
            case 2:   return hashFull<2>(s, addr, size);
            case 3:   return hashFull<3>(s, addr, size);
            case 4:   return hashFull<4>(s, addr, size);
            case 5:   return hashFull<5>(s, addr, size);
            case 6:   return hashFull<6>(s, addr, size);
            case 7:   return hashFull<7>(s, addr, size);
            case 8:   return hashFull<8>(s, addr, size);
            default:
                MEDDLY_DCASSERT(0);
                throw error(error::MISCELLANEOUS);
        }
      }

      //--------------------------------------------------------------
      // Helpers - getSingletonIndex
      //--------------------------------------------------------------
      template <int pbytes>
      inline int getSingletonFull(node_address addr, int size, 
        node_handle &dv) const
      {
        MEDDLY_DCASSERT(sizeOf(addr) >= 0);
        MEDDLY_DCASSERT(sizeOf(addr) == size);
      
        const unsigned char* down = fullDown(addr);
        for (int i=0; i<size; i++) {
          dataToDown<pbytes>(down, dv);
          down += pbytes;
          if (0==dv) continue;
          if (i+1 != size) return -1;
          return i;
        }
        return -1;
      }

      inline int getSingletonFull(int pbytes, node_address addr, int size, 
        node_handle &down) const
      {
          switch (pbytes) {
              case 1:   return getSingletonFull<1>(addr, size, down);
              case 2:   return getSingletonFull<2>(addr, size, down);
              case 3:   return getSingletonFull<3>(addr, size, down);
              case 4:   return getSingletonFull<4>(addr, size, down);
              case 5:   return getSingletonFull<5>(addr, size, down);
              case 6:   return getSingletonFull<6>(addr, size, down);
              case 7:   return getSingletonFull<7>(addr, size, down);
              case 8:   return getSingletonFull<8>(addr, size, down);
              default:
                  MEDDLY_DCASSERT(0);
                  throw error(error::MISCELLANEOUS);
          }
      }

      //--------------------------------------------------------------
      // Helpers - findSparseIndex
      //--------------------------------------------------------------
      template <int ibytes>
      inline int findSparseIndex(node_handle addr, int nnz, int i) const 
      {
        MEDDLY_DCASSERT(nnz>=0);
        int low = 0;
        int high = nnz;
        const unsigned char* index = sparseIndex(addr);
        while (low < high) {
          int z = (low+high)/2;
          MEDDLY_CHECK_RANGE(0, z, nnz);
          int indz;
          dataToUnsigned<ibytes>(index + z*ibytes, indz);
          if (indz == i)  return z;
          if (indz < i)   low = z + 1;
          else            high = z;
        }
        return -1;
      }

      inline int findSparseIndex(int ibytes, node_handle addr, int nnz, int i)
      const
      {
          switch (ibytes) {
              case 1:   return findSparseIndex<1>(addr, nnz, i);
              case 2:   return findSparseIndex<2>(addr, nnz, i);
              case 3:   return findSparseIndex<3>(addr, nnz, i);
              case 4:   return findSparseIndex<4>(addr, nnz, i);
              default:
                  MEDDLY_DCASSERT(0);
                  throw error(error::MISCELLANEOUS);
          }
      }

  //
  // node data inlines
  //
  private:
      inline unsigned char* chunkOf(node_address addr) const {
        MEDDLY_DCASSERT(data);
        MEDDLY_DCASSERT(memchunk);
        MEDDLY_DCASSERT(holeManager);
        MEDDLY_CHECK_RANGE(1, addr, holeManager->lastSlot()+1);
        MEDDLY_DCASSERT(data[addr] >= 0); // not a hole
        return (unsigned char*) (memchunk+addr);
      }
      inline int& rawSizeOf(node_address addr) const {
        return ((int*)chunkOf(addr))[0];
      }
      inline int sizeOf(node_address addr) const { 
        return rawSizeOf(addr); 
      }
      inline void setSizeOf(node_address addr, int sz) { 
        rawSizeOf(addr) = sz; 
      }

      inline unsigned char& rawStyleOf(node_address addr) const {
        return (chunkOf(addr) + sizeof(int))[0];
      }

      inline void setStyleOf(node_address addr, int ptrb, int idxb) {
        rawStyleOf(addr) = ((ptrb-1) & 0x07) | (((idxb-1) & 0x03) << 3);
      }

      inline void getStyleOf(node_address addr, int& ptrb, int& idxb) const {
        unsigned char x = rawStyleOf(addr);
        ptrb = 1+ (x & 0x07);
        idxb = 1+ (( x >> 3) & 0x03);
      }

      inline int pointerBytesOf(node_address addr) const {
        return 1+ ( rawStyleOf(addr) & 0x07 );
      }

      inline int indexBytesOf(node_address addr) const {
        return 1+ ( (rawStyleOf(addr) >> 3) & 0x03 );
      }

      inline unsigned char* UH(node_address addr) const {
        return chunkOf(addr) + sizeof(int) + 1;
      }

      inline unsigned char* HH(node_address addr) const {
        return UH(addr) + unhashedBytes;
      }
      
      inline unsigned char* fullDown(node_address addr) const {
        MEDDLY_DCASSERT(sizeOf(addr) >= 0);
        return HH(addr) + hashedBytes;
      }
      inline unsigned char* fullEdge(node_address addr) const {
        return fullDown(addr) + sizeOf(addr) * pointerBytesOf(addr);
      }

      inline unsigned char* sparseDown(node_address addr) const {
        MEDDLY_DCASSERT(sizeOf(addr) < 0);
        return HH(addr) + hashedBytes;
      }
      inline unsigned char* sparseIndex(node_address addr) const {
        return sparseDown(addr) - sizeOf(addr) * pointerBytesOf(addr);
      }
      inline unsigned char* sparseEdge(node_address addr) const {
        return sparseIndex(addr) - sizeOf(addr) * indexBytesOf(addr);
      }


  private:
      // header indexes (relative to chunk start)
      static const int count_index = 0;
      static const int next_index = 1;    
      static const int mem_index = 2;

      static const int extraSlots = 3;
  private:
    /// what we are, for display purposes
    const char* storageName;

    /// the data array
    node_handle* data;
    /// for convenience - memory chunk by address
    node_handle* memchunk;

    holeman* holeManager;

  // header sizes; vary by forest.
  private:
      /// Number of bytes required for each outgoing edge's value (can be 0).
      char edgeBytes;
      /// Number of bytes for extra unhashed data (typically 0).
      char unhashedBytes;
      /// Number of bytes for extra hashed data (typically 0).
      char hashedBytes;


};

// ******************************************************************
// *                                                                *
// *                                                                *
// *                    compact_grid_style class                    *
// *                                                                *
// *                                                                *
// ******************************************************************

/** Compact storage using the original grid mechanism for holes.
*/

class MEDDLY::compact_grid_style : public node_storage_style {
  public:
    compact_grid_style();
    virtual ~compact_grid_style();
    virtual node_storage* createForForest(expert_forest* f) const;
};


#endif  // include guard

