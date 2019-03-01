
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

#include "simple.h"

#include "hm_grid.h"
#include "hm_array.h"
#include "hm_heap.h"
#include "hm_none.h"

// #define DEBUG_ENCODING
// #define DEBUG_COMPACTION
// #define DEBUG_SLOW
// #define MEMORY_TRACE
// #define DEEP_MEMORY_TRACE

inline char slotsForBytes(int bytes) 
{
  int sl = bytes / sizeof(MEDDLY::node_handle);
  if (bytes % sizeof(MEDDLY::node_handle)) sl++;
  return sl;
}


// ******************************************************************
// *                                                                *
// *                                                                *
// *                     simple_storage methods                     *
// *                                                                *
// *                                                                *
// ******************************************************************


MEDDLY::simple_storage::simple_storage(expert_forest* f, holeman* hm, const char* sN)
 : node_storage(f)
{
  holeManager = hm;
  storageName = sN;
  data = 0;

  edgeSlots = slotsForBytes(f->edgeBytes());
  unhashedSlots = slotsForBytes(f->unhashedHeaderBytes());
  hashedSlots = slotsForBytes(f->hashedHeaderBytes());
  MEDDLY_DCASSERT(holeManager);
  holeManager->setParent(this);
}

MEDDLY::simple_storage::~simple_storage()
{
  delete holeManager;
}

void MEDDLY::simple_storage::collectGarbage(bool shrink)
{
  //
  // Should we even bother?
  //
  node_handle wasted = holeManager->holeSlots();
  if (0==data || 0==wasted) return;
  if (wasted <= getParent()->getPolicies().compact_min) {
    return;
  }
  if (wasted <  getParent()->getPolicies().compact_max) {

    // If percentage of wasted slots is below trigger, then don't compact
    if (100 * wasted < 
        holeManager->lastSlot() * getParent()->getPolicies().compact_frac) 
      return;

  }

#ifdef DEBUG_SLOW
  fprintf(stderr, "Compacting forest level\n");
#endif
#ifdef MEMORY_TRACE
  printf("Compacting\n");
#endif
#ifdef DEBUG_COMPACTION
  printf("Before compaction:\n");
  dumpInternal(stdout);
  printf("\n");
#endif

  //
  // Scan the whole array of data, copying over itself and skipping holes.
  // 
  node_handle* node_ptr = (data + 1);  // since we leave [0] empty
  node_handle* end_ptr =  (data + holeManager->lastSlot() + 1);
  node_handle* curr_ptr = node_ptr;

  while (node_ptr < end_ptr) {
    if (*node_ptr < 0) {
      //
      // This is a hole, skip it
      // 
      node_ptr = data + holeManager->chunkAfterHole(node_ptr - data);
      continue;
    } 
    //
    // A real node, move it
    //
    MEDDLY_DCASSERT(!getParent()->isPessimistic() || *node_ptr != 0);
    
    long old_off = node_ptr - data;
    long new_off = curr_ptr - data;

    // copy the node, except for the tail
    int datalen = slotsForNode(sizeOf(old_off)) - 1;
    if (node_ptr != curr_ptr) {
      memmove(curr_ptr, node_ptr, datalen * sizeof(node_handle));
    }
    node_ptr += datalen;
    curr_ptr += datalen;
    //
    // Skip any padding
    //
    if (*node_ptr < 0) {
      node_ptr -= *node_ptr;  
    }
    //
    // Copy trailer, the node number
    //
    *curr_ptr = *node_ptr;
    moveNodeOffset(*curr_ptr, old_off, new_off);
    curr_ptr++;
    node_ptr++;

  } // while
  MEDDLY_DCASSERT(node_ptr == end_ptr);

  holeManager->clearHolesAndShrink( (curr_ptr - 1 - data), shrink );
  MEDDLY_DCASSERT(0==holeManager->holeSlots());

  incCompactions(); 

#ifdef DEBUG_COMPACTION
  printf("After compaction:\n");
  dumpInternal(stdout);
  printf("\n");
#endif
}

void MEDDLY::simple_storage
::reportStats(output &s, const char* pad, unsigned flags) const
{
  static unsigned STORAGE = 
    expert_forest::STORAGE_STATS | expert_forest::STORAGE_DETAILED;

  if (flags & STORAGE) {
    s << pad << "Stats for " << storageName << "\n";

    // anything for us?
  }

  holeManager->reportStats(s, pad, flags);

#ifdef DEVELOPMENT_CODE
  verifyStats();
#endif
}

/*
void MEDDLY::simple_storage::showNode(output &s, node_address addr, bool verb) const
{
  if (sizeOf(addr) < 0) {
    // Sparse node
    int nnz = -sizeOf(addr);
    if (verb) s << " nnz : " << long(nnz);
    s << " down: (";
    for (int z=0; z<nnz; z++) {
      if (z) s << ", ";
      s << long(SI(addr)[z]) << ":";
      if (edgeSlots) {
        s.put('<');
        getParent()->showEdgeValue(s, SEP(addr, z));
        s << ", ";
      } 
      node_handle d = SD(addr)[z];
      if (getParent()->isTerminalNode(d)) {
        getParent()->showTerminal(s, d);
      } else {
        s.put(long(d));
      }
      if (edgeSlots) s.put('>');
    } // for z
    s.put(')');
  } else {
    // Full node
    int size = sizeOf(addr);
    if (verb) s << " size: " << long(size);
    s << " down: [";
    for (int i=0; i<size; i++) {
      if (i) s.put('|');
      if (edgeSlots) {
        s.put('<');
        getParent()->showEdgeValue(s, FEP(addr, i));
        s.put(", ");
      } 
      node_handle d = FD(addr)[i];
      if (getParent()->isTerminalNode(d)) {
        getParent()->showTerminal(s, d);
      } else {
        s.put(long(d));
      }
      if (edgeSlots) s.put('>');
    } // for i
    s.put(']');
  }

  // show extra header stuff
  if (unhashedSlots) {
    getParent()->showUnhashedHeader(s, UH(addr));
  }
  if (hashedSlots) {
    getParent()->showHashedHeader(s, HH(addr));
  }
}

void MEDDLY::simple_storage
::writeNode(output &s, node_address addr, const node_handle* map) const
{
  s << long(sizeOf(addr)) << "\n";
  if (sizeOf(addr) < 0) {
    // Sparse node
    int nnz = -sizeOf(addr);
    // write indexes
    s.put('\t');
    for (int z=0; z<nnz; z++) {
      s << " " << long(SI(addr)[z]);
    }
    s << "\n\t";
    // write down pointers
    for (int z=0; z<nnz; z++) {
      s.put(' ');
      node_handle d = SD(addr)[z];
      if (getParent()->isTerminalNode(d)) {
        getParent()->writeTerminal(s, d);
      } else {
        if (map) d = map[d];
        s.put(long(d));
      }
    }
    // write edges
    if (edgeSlots) {
      s << "\n\t";
      for (int z=0; z<nnz; z++) {
        s.put(' ');
        getParent()->showEdgeValue(s, SEP(addr, z));
      }
    } 
    s.put('\n');
  } else {
    // Full node
    int size = sizeOf(addr);
    s.put('\t');
    // write down pointers
    for (int i=0; i<size; i++) {
      s.put(' ');
      node_handle d = FD(addr)[i];
      if (getParent()->isTerminalNode(d)) {
        getParent()->writeTerminal(s, d);
      } else {
        if (map) d = map[d];
        s.put(long(d));
      }
    }
    // write edges
    if (edgeSlots) {
      s << "\n\t";
      for (int i=0; i<size; i++) {
        s.put(' ');
        getParent()->showEdgeValue(s, FEP(addr, i));
      }
    } 
    s.put('\n');
  }

  // write extra header stuff
  // this goes LAST so we can read it into a built node
  if (unhashedSlots) {
    getParent()->writeUnhashedHeader(s, UH(addr));
  }
  if (hashedSlots) {
    getParent()->writeHashedHeader(s, HH(addr));
  }

}
*/



MEDDLY::node_address MEDDLY::simple_storage
::makeNode(node_handle p, const unpacked_node &nb, node_storage_flags opt)
{
#ifdef DEBUG_ENCODING
  printf("simple_storage making node\n        temp:  ");
  nb.show(stdout, true);
#endif
  node_handle tv = getParent()->getTransparentNode();

  //
  // Easy case - sparse nodes disabled
  //
  if (0==(forest::policies::ALLOW_SPARSE_STORAGE & opt)) {
    if (nb.isSparse()) {
      int truncsize = -1;
      for (int z=0; z<nb.getNNZs(); z++) {
        truncsize = MAX(truncsize, nb.i(z));
      }
      return (truncsize<0) ? tv : makeFullNode(p, truncsize+1, nb);
    } else {
      for (int i=nb.getSize()-1; i>=0; i--) {
        if (nb.d(i)!=tv) {
          return makeFullNode(p, i+1, nb);
        }
      }
      return tv;
    }
  }

  //
  // Easy case - full nodes disabled
  //
  if (0==(forest::policies::ALLOW_FULL_STORAGE & opt)) {
    if (nb.isSparse()) {
      return makeSparseNode(p, nb.getNNZs(), nb);
    } else {
      int nnz = 0;
      for (int i=0; i<nb.getSize(); i++) {
        if (nb.d(i)!=tv) {
        	nnz++;
        }
      }
      return makeSparseNode(p, nnz, nb);
    }
  }

  //
  // Full and sparse are allowed, determine which is more compact
  //
  int truncsize = -1;
  int nnz;
  if (nb.isSparse()) {
    nnz = nb.getNNZs();
    if(nnz==0) {
      return tv;
    }
    for (int z=0; z<nnz; z++) {
      truncsize = MAX(truncsize, nb.i(z));
    }
//    if (truncsize<0) {
//      return tv;
//    }
    truncsize++;
  } else {
    nnz = 0;
    for (int i=0; i<nb.getSize(); i++) {
      if (nb.d(i)!=tv) {
        nnz++;
        truncsize = i;
      }
    }
    if(nnz==0) {
      return tv;
    }
    truncsize++;
  }

  return (slotsForNode(-nnz) < slotsForNode(truncsize)) ? makeSparseNode(p, nnz, nb) : makeFullNode(p, truncsize, nb);
}

void MEDDLY::simple_storage::unlinkDownAndRecycle(node_address addr)
{
  //
  // Unlink down pointers
  //
  const node_handle* down;
  int size = sizeOf(addr);
  if (size < 0) {
    size = -size;
    down = SD(addr);
  } else {
    down = FD(addr);
  }
  for (int i=0; i<size; i++) {
    getParent()->unlinkNode(down[i]);
  }

  //
  // Recycle
  //
  holeManager->recycleChunk(addr, activeNodeActualSlots(addr));
}

bool MEDDLY::simple_storage
::areDuplicates(node_address addr, const unpacked_node &n) const
{
  if (n.HHbytes()) {
    if (memcmp(HH(addr), n.HHptr(), n.HHbytes())) return false;
  }

  if (n.UHbytes()) {
    if (memcmp(UH(addr), n.UHptr(), n.UHbytes())) return false;
  }

  node_handle tv=getParent()->getTransparentNode();
  int size = sizeOf(addr);
  if (size<0) {
    //
    // Node is sparse
    //
    int nnz = -size;
    const node_handle* down = SD(addr);
    const node_handle* index = SI(addr);
    if (n.isFull()) {
      // check that down matches
      int i = 0;
      for (int z=0; z<nnz; z++) {
        if (index[z] >= n.getSize()) return false;
        for (; i<index[z]; i++) {
          if (n.d(i)!=tv) {
            return false;
          }
        }
        if (n.d(i) != down[z]) return false;
        i++;
      }
      for (; i<n.getSize(); i++) {
        if (n.d(i)!=tv) {
          return false;
        }
      }
      // check that edges match
      if (n.hasEdges()) {
        for (int z=0; z<nnz; z++) {
          if (!getParent()->areEdgeValuesEqual( 
                  SEP(addr, z),   n.eptr(index[z])
              )) return false;
        } // for z
      }
      // must be equal
      return true;
    }
    else {
      // n is sparse
      if (n.getNNZs() != nnz) return false;
      // check that down matches
      for (int z=0; z<nnz; z++) {
        if (index[z] != n.i(z)) return false;
        if (down[z] != n.d(z))  return false;
      }
      // check that edges match
      if (n.hasEdges()) {
        for (int z=0; z<nnz; z++) {
          if (!getParent()->areEdgeValuesEqual(SEP(addr, z), n.eptr(z))) {
            return false;
          }
        }
      }
      // must be equal
      return true;
    }
  }
  else {
    //
    // Node is truncated full
    //
    const node_handle* down = FD(addr);
    if (n.isFull()) {
      if (size > n.getSize()) return false;
      // check down
      int i;
      for (i=0; i<size; i++) {
        if (down[i] != n.d(i)) return false;
      }
      for (; i<n.getSize(); i++) {
        if (n.d(i)!=tv) {
          return false;
        }
      }
      // check edges
      if (n.hasEdges()) {
        for (int i=0; i<size; i++) if (down[i]) {
          if (!getParent()->areEdgeValuesEqual(FEP(addr, i), n.eptr(i))) {
            return false;
          }
        }
      }
      // must be equal
      return true;
    }
    else {
      // n is sparse
      int i = 0;
      // check down
      for (int z=0; z<n.getNNZs(); z++) {
        if (n.i(z) >= size) return false;
        for (; i<n.i(z); i++) {
          if (down[i]!=tv) {
            return false;
          }
        }
        if (n.d(z) != down[i]) return false;
        i++;
      }
      if (i<size) return false; // there WILL be a non-zero down
      // check edges
      if (n.hasEdges()) {
        for (int z=0; z<n.getNNZs(); z++) {
          if (!getParent()->areEdgeValuesEqual(FEP(addr, n.i(z)),  n.eptr(z))) {
            return false;
          }
        } // for z
      }
      // must be equal
      return true;
    }
  }
}

void MEDDLY::simple_storage
::fillUnpacked(unpacked_node &nr, node_address addr, unpacked_node::storage_style st2) const
{
#ifdef DEBUG_ENCODING
  printf("simple_storage filling reader\n    internal: ");
  dumpInternalNode(stdout, addr, 0x03);
  printf("        node: ");
  showNode(stdout, addr, true);
#endif

  const node_handle tv = getParent()->getTransparentNode();
  
  // Copy extra header

  if (hashedSlots) {
    memcpy(nr.HHdata(), HH(addr), getParent()->hashedHeaderBytes());
  }

  if (unhashedSlots) {
    memcpy(nr.UHdata(), UH(addr), getParent()->unhashedHeaderBytes());
  }

  // Copy everything else

  int size = sizeOf(addr);

  /*
      Set the unpacked node storage style based on settings
  */
  switch (st2) {
      case unpacked_node::FULL_NODE:     nr.bind_as_full(true);    break;
      case unpacked_node::SPARSE_NODE:   nr.bind_as_full(false);   break;
      case unpacked_node::AS_STORED:     nr.bind_as_full(size>=0); break;

      default:            assert(0);
  };

  if (size < 0) {
    //
    // Node is sparse
    //
    int nnz = -size;
    const node_handle* down = SD(addr);
    const node_handle* index = SI(addr);

    if (nr.isFull()) {
      for (int i=0; i<nr.getSize(); i++) {
        nr.d_ref(i) = tv;
        if (nr.hasEdges()) {
          memset(nr.eptr_write(i), 0, nr.edgeBytes());
        }
      }
      for (int z=0; z<nnz; z++) {
        int i = index[z];
        nr.d_ref(i) = down[z];
        if (nr.hasEdges()) {
          memcpy(nr.eptr_write(i), SEP(addr, z), nr.edgeBytes());
        }
      }
#ifdef DEBUG_ENCODING
      printf("\n        temp:  ");
      nr.show(stdout, getParent(), true);
      printf("\n");
#endif
      return;
    }

    // nr is sparse

    for (int z=0; z<nnz; z++) {
      nr.d_ref(z) = down[z];
      nr.i_ref(z) = index[z];
      if (nr.hasEdges()) {
        memcpy(nr.eptr_write(z), SEP(addr, z), nr.edgeBytes());
      }
    } // for z
    nr.shrinkSparse(nnz);
#ifdef DEBUG_ENCODING
    printf("\n        temp:  ");
    nr.show(stdout, getParent(), true);
    printf("\n");
#endif
    return;
  } 

  //
  // Node is full
  //
  const node_handle* down = FD(addr);
  if (nr.isFull()) {
    int i;
    for (i=0; i<size; i++) {
      nr.d_ref(i) = down[i];
      if (nr.hasEdges()) {
        memcpy(nr.eptr_write(i), FEP(addr, i), nr.edgeBytes());
      }
    } 
    if (unpacked_node::AS_STORED == st2) {
      nr.shrinkFull(size);
    } else {
      for (; i<nr.getSize(); i++) {
        nr.d_ref(i) = tv;
        if (nr.hasEdges()) {
          memset(nr.eptr_write(i), 0, nr.edgeBytes());
        }
      }
    }
#ifdef DEBUG_ENCODING
    printf("\n        temp:  ");
    nr.show(stdout, getParent(), true);
    printf("\n");
#endif
    return;
  }

  // nr is sparse
  int z = 0;
  for (int i=0; i<size; i++) if (down[i]) {
    nr.d_ref(z) = down[i];
    nr.i_ref(z) = i;
    if (nr.hasEdges()) {
      memcpy(nr.eptr_write(z), FEP(addr, i), nr.edgeBytes());
    }
    z++;
  } // for i
  nr.shrinkSparse(z);
#ifdef DEBUG_ENCODING
  printf("\n        temp:  ");
  nr.show(stdout, getParent(), true);
  printf("\n");
#endif
}


unsigned MEDDLY::simple_storage::hashNode(int level, node_address addr) const
{
  hash_stream s;
  s.start(0);

  // Do the hashed header part, if any

  if (hashedSlots) {
    s.push(HH(addr), getParent()->hashedHeaderBytes());
  }

  //
  // Hash the node itself
  
  int size = sizeOf(addr);
  if (size < 0) {
    // Node is sparse
    int nnz = -size;
    node_handle* down = SD(addr);
    node_handle* index = SI(addr);
    if (getParent()->areEdgeValuesHashed()) {
      for (int z=0; z<nnz; z++) {
        s.push(index[z], down[z], ((int*)SEP(addr, z))[0]);
      } // for z
    } else {
      for (int z=0; z<nnz; z++) {
        s.push(index[z], down[z]);
      } // for z
    }
  } else {
    // Node is full
    node_handle* down = FD(addr);
    node_handle tv=getParent()->getTransparentNode();
    if (getParent()->areEdgeValuesHashed()) {
      for (int i=0; i<size; i++) {
    	if (down[i]!=tv) {
          s.push(i, down[i], ((int*)FEP(addr, i))[0]);
    	}
      } // for z
    } else {
      for (int i=0; i<size; i++) {
    	if (down[i]!=tv) {
          s.push(i, down[i]);
    	}
      } // for z
    }
  }

  return s.finish();
}


int MEDDLY::simple_storage
::getSingletonIndex(node_address addr, node_handle &down) const
{
  int size = sizeOf(addr);
  if (size<0) {
    // sparse node --- easy
    if (size != -1) return -1;
    down = SD(addr)[0];
    return SI(addr)[0];
  } 
  
  // full node
  const node_handle* dnptr = FD(addr);
  for (int i=0; i<size; i++) {
    if (0==dnptr[i]) continue;
    if (i+1 != size) return -1;
    down = dnptr[i];
    return i;
  }
  return -1;
}


MEDDLY::node_handle 
MEDDLY::simple_storage
::getDownPtr(node_address addr, int index) const
{
  if (index<0) throw error(error::INVALID_VARIABLE);
  int size = sizeOf(addr);
  if (size<0) {
    int z = findSparseIndex(addr, index);
    if (z<0) return 0;
    return SD(addr)[z];
  } else {
    if (index < size) {
      return FD(addr)[index];
    } else {
      return 0;
    }
  }
}

void MEDDLY::simple_storage
::getDownPtr(node_address addr, int index, int& ev, node_handle& dn) const
{
  if (index<0) throw error(error::INVALID_VARIABLE);
  if (sizeOf(addr)<0) {
    int z = findSparseIndex(addr, index);
    if (z<0) {
      dn = 0;
      ev = 0;
    } else {
      dn = SD(addr)[z];
      ev = ((int*)SEP(addr, z))[0];
    }
  } else {
    if (index < sizeOf(addr)) {
      dn = FD(addr)[index];
      ev = ((int*)FEP(addr, index))[0];
    } else {
      dn = 0;
      ev = 0;
    }
  }
}

void MEDDLY::simple_storage
::getDownPtr(node_address addr, int index, float& ev, node_handle& dn) const
{
  if (index<0) throw error(error::INVALID_VARIABLE);
  if (sizeOf(addr)<0) {
    int z = findSparseIndex(addr, index);
    if (z<0) {
      dn = 0;
      ev = 0;
    } else {
      dn = SD(addr)[z];
      ev = ((float*)SEP(addr, z))[0];
    }
  } else {
    if (index < sizeOf(addr)) {
      dn = FD(addr)[index];
      ev = ((float*)FEP(addr, index))[0];
    } else {
      dn = 0;
      ev = 0;
    }
  }
}


const void* MEDDLY::simple_storage
::getUnhashedHeaderOf(node_address addr) const
{
  return UH(addr);
}

const void* MEDDLY::simple_storage
::getHashedHeaderOf(node_address addr) const
{
  return HH(addr);
}


//
//
// Protected
//
//

void MEDDLY::simple_storage::updateData(node_handle* d)
{
  data = d;
  updateCountArray(data + count_index);
  updateNextArray(data + next_index);
}

int MEDDLY::simple_storage::smallestNode() const
{
  return slotsForNode(0);
}

void MEDDLY::simple_storage::dumpInternalInfo(output &s) const
{
  holeManager->dumpInternalInfo(s);
}

void MEDDLY::simple_storage::dumpInternalTail(output &s) const
{
  holeManager->dumpInternalTail(s);
}



MEDDLY::node_address 
MEDDLY::simple_storage
::dumpInternalNode(output &s, node_address a, unsigned flags) const
{
  if (a<=0) return 0;
  int awidth = digits(getParent()->getLastNode());
  if (a > holeManager->lastSlot()) {
    s.put(long(a), awidth);
    s << " : free slots\n";
    return 0;
  }
  MEDDLY_DCASSERT(data);
  if (data[a]<0) { 
    // hole
    if (flags & 0x02) {
      s.put(long(a), awidth);
      s << " : ";
      holeManager->dumpHole(s, a);
    }
    a = holeManager->chunkAfterHole(a);
  } else {
    // proper node
    if (flags & 0x01) {
      s.put(long(a), awidth);
      s << " : ";
      int nElements = activeNodeActualSlots(a);
      s << "[" << long(data[a]);
      for (int i=1; i<nElements; i++) {
        s.put('|');
        s.put(long(data[a+i]));
      }
      s << "]\n";
    }
    a += activeNodeActualSlots(a);
  }
  return a;
}








//
//
// Private
//
//



MEDDLY::node_handle MEDDLY::simple_storage
::makeFullNode(node_handle p, int size, const unpacked_node &nb)
{
#if 0
  node_address addr = allocNode(size, p, false);
  MEDDLY_DCASSERT(1==getCountOf(addr));
  node_handle* down = FD(addr);
  if (edgeSlots) {
      MEDDLY_DCASSERT(nb.hasEdges());
      char* edge = (char*) FE(addr);
      int edge_bytes = edgeSlots * sizeof(node_handle);
      if (nb.isSparse()) {
        memset(down, 0, size * sizeof(node_handle));
        memset(edge, 0, size * edge_bytes);
        for (int z=0; z<nb.getNNZs(); z++) {
          int i = nb.i(z);
          MEDDLY_CHECK_RANGE(0, i, size);
          down[i] = nb.d(z);
          memcpy(edge + i * edge_bytes, nb.eptr(z), edge_bytes);
        }
      } else {
        for (int i=0; i<size; i++) down[i] = nb.d(i);
        // kinda hacky
        memcpy(edge, nb.eptr(0), size * edge_bytes);
      }
  } else {
      MEDDLY_DCASSERT(!nb.hasEdges());
      if (nb.isSparse()) {
        memset(down, 0, size * sizeof(node_handle));
        for (int z=0; z<nb.getNNZs(); z++) {
          MEDDLY_CHECK_RANGE(0, nb.i(z), size);
          down[nb.i(z)] = nb.d(z);
        }
      } else {
        for (int i=0; i<size; i++) down[i] = nb.d(i);
      }
  }
#else
  node_address addr = allocNode(size, p, true);
  MEDDLY_DCASSERT(1==getCountOf(addr));
  node_handle* down = FD(addr);
  if (edgeSlots) {
      MEDDLY_DCASSERT(nb.hasEdges());
      char* edge = (char*) FE(addr);
      int edge_bytes = edgeSlots * sizeof(node_handle);
      if (nb.isSparse()) {
        for (int z=0; z<nb.getNNZs(); z++) {
          int i = nb.i(z);
          MEDDLY_CHECK_RANGE(0, i, size);
          down[i] = nb.d(z);
          memcpy(edge + i * edge_bytes, nb.eptr(z), edge_bytes);
        }
      } else {
        for (int i=0; i<size; i++) down[i] = nb.d(i);
        // kinda hacky
        memcpy(edge, nb.eptr(0), size * edge_bytes);
      }
  } else {
      MEDDLY_DCASSERT(!nb.hasEdges());
      if (nb.isSparse()) {
        for (int z=0; z<nb.getNNZs(); z++) {
          MEDDLY_CHECK_RANGE(0, nb.i(z), size);
          down[nb.i(z)] = nb.d(z);
        }
      } else {
        for (int i=0; i<size; i++) down[i] = nb.d(i);
      }
  }
#endif
  copyExtraHeader(addr, nb);
#ifdef DEBUG_ENCODING
  printf("\n        made: ");
  showNode(stdout, addr, true);
  printf("\n    internal: ");
  dumpInternalNode(stdout, addr, 0x03);
#endif
  return addr;
}

MEDDLY::node_handle MEDDLY::simple_storage
::makeSparseNode(node_handle p, int size, const unpacked_node &nb)
{
  node_address addr = allocNode(-size, p, false);
  MEDDLY_DCASSERT(1==getCountOf(addr));
  node_handle* index = SI(addr);
  node_handle* down  = SD(addr);
  if (nb.hasEdges()) {
    MEDDLY_DCASSERT(nb.hasEdges());
    char* edge = (char*) SE(addr);
    int edge_bytes = edgeSlots * sizeof(node_handle);
    if (nb.isSparse()) {
      for (int z=0; z<size; z++) {
        down[z] = nb.d(z);
        index[z] = nb.i(z);
      }
      // kinda hacky
      memcpy(edge, nb.eptr(0), size * edge_bytes);
#ifdef DEVELOPMENT_CODE
      // check if the sparse node is sorted
      for (int z=1; z<size; z++) {
        MEDDLY_DCASSERT(index[z-1] < index[z]);
      }
#endif
    } else {
      int z = 0;
      for (int i=0; i<nb.getSize(); i++) if (nb.d(i)) {
        MEDDLY_CHECK_RANGE(0, z, size);
        down[z] = nb.d(i);
        index[z] = i;
        memcpy(edge + z * edge_bytes, nb.eptr(i), edge_bytes);
        z++;
      }
    }
  } else {
    MEDDLY_DCASSERT(!nb.hasEdges());
    if (nb.isSparse()) {
      for (int z=0; z<size; z++) {
        down[z] = nb.d(z);
        index[z] = nb.i(z);
      }
#ifdef DEVELOPMENT_CODE
      // check if the sparse node is sorted
      for (int z=1; z<size; z++) {
        MEDDLY_DCASSERT(index[z-1] < index[z]);
      }
#endif
    } else {
      int z = 0;
      node_handle tv = getParent()->getTransparentNode();
      for (int i=0; i<nb.getSize(); i++) {
        if (nb.d(i)!=tv) {
          MEDDLY_CHECK_RANGE(0, z, size);
          down[z] = nb.d(i);
          index[z] = i;
          z++;
        }
      }
    }
  }
  copyExtraHeader(addr, nb);
#ifdef DEBUG_ENCODING
  printf("\n        made: ");
  showNode(stdout, addr, true);
  printf("\n    internal: ");
  dumpInternalNode(stdout, addr, 0x03);
#endif
  return addr;
}


void MEDDLY::simple_storage
::copyExtraHeader(node_address addr, const unpacked_node &nb)
{
  // copy extra header info, if any
  if (unhashedSlots) {
    memcpy(UH(addr), nb.UHptr(), nb.UHbytes());
  }
  if (hashedSlots) {
    memcpy(HH(addr), nb.HHptr(), nb.HHbytes());
  }
}



MEDDLY::node_handle 
MEDDLY::simple_storage::allocNode(int sz, node_handle tail, bool clear)
{
  int slots = slotsForNode(sz);
  MEDDLY_DCASSERT(slots >= extraSlots + unhashedSlots + hashedSlots);

  node_handle off = holeManager->requestChunk(slots);
  node_handle got = -data[off];
  incMemUsed(got * sizeof(node_handle));
  MEDDLY_DCASSERT(got >= slots);
  if (clear) {
//	memset(data+off, 0, slots*sizeof(node_handle));
	node_handle tv = getParent()->getTransparentNode();
	for(int i=0; i<slots; i++){
		data[off+i]=tv;
	}
  }
  setCountOf(off, 1);                     // #incoming
  setNextOf(off, temp_node_value);        // mark as a temp node
  setSizeOf(off, sz);                     // size
  data[off+slots-1] = slots - got;        // negative padding
  data[off+got-1] = tail;                 // tail entry
#ifdef MEMORY_TRACE
  printf("Allocated new node, asked %d, got %d, position %d (size %d)\n", slots, got, off, sz);
#ifdef DEEP_MEMORY_TRACE
  dumpInternal(stdout);
#else
  dumpInternal(stdout, off);
#endif
#endif
  return off;
}


#ifdef DEVELOPMENT_CODE
void MEDDLY::simple_storage::verifyStats() const
{
  int holes = 0;
  node_address hole_count = 0;
  for (node_address a=1; a<=holeManager->lastSlot(); ) {
    if (data[a]<0) {
      // hole
      node_address anext = holeManager->chunkAfterHole(a);
      hole_count += (anext - a);
      holes++;
      a = anext;
      continue;
    }
    // not hole
    a += activeNodeActualSlots(a);
  }
  // done scan, compare stats

  if ((hole_count == holeManager->holeSlots()) &&
     (holes == holeManager->numHoles())
     )   return;

  printf("Counted holes: %d stat: %d\n", 
    holes, holeManager->numHoles()
  );
  
  printf("Counted hole slots: %ld stat: %ld\n", 
    hole_count, holeManager->holeSlots()
  );

  FILE_output myout(stdout);
  dumpInternal(myout, 0x03);
  
  MEDDLY_DCASSERT(0);
}
#endif

// ******************************************************************
// *                                                                *
// *                                                                *
// *                   simple_grid_style  methods                   *
// *                                                                *
// *                                                                *
// ******************************************************************


MEDDLY::simple_grid_style::simple_grid_style()
{
}

MEDDLY::simple_grid_style::~simple_grid_style()
{
}

MEDDLY::node_storage* MEDDLY::simple_grid_style
::createForForest(expert_forest* f) const
{
  return new simple_storage(f, new hm_grid, "simple node storage with grid for holes");
}

// ******************************************************************
// *                                                                *
// *                                                                *
// *                   simple_array_style methods                   *
// *                                                                *
// *                                                                *
// ******************************************************************


MEDDLY::simple_array_style::simple_array_style()
{
}

MEDDLY::simple_array_style::~simple_array_style()
{
}

MEDDLY::node_storage* MEDDLY::simple_array_style
::createForForest(expert_forest* f) const
{
  return new simple_storage(f, new hm_array, "simple node storage with array of lists for holes");
}

// ******************************************************************
// *                                                                *
// *                                                                *
// *                   simple_heap_style  methods                   *
// *                                                                *
// *                                                                *
// ******************************************************************


MEDDLY::simple_heap_style::simple_heap_style()
{
}

MEDDLY::simple_heap_style::~simple_heap_style()
{
}

MEDDLY::node_storage* MEDDLY::simple_heap_style
::createForForest(expert_forest* f) const
{
  return new simple_storage(f, new hm_heap, "simple node storage with heaps for holes");
}

// ******************************************************************
// *                                                                *
// *                                                                *
// *                   simple_none_style  methods                   *
// *                                                                *
// *                                                                *
// ******************************************************************


MEDDLY::simple_none_style::simple_none_style()
{
}

MEDDLY::simple_none_style::~simple_none_style()
{
}

MEDDLY::node_storage* MEDDLY::simple_none_style
::createForForest(expert_forest* f) const
{
  return new simple_storage(f, new hm_none, "simple node storage with no hole management");
}

