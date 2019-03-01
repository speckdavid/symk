
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

#include "defines.h"
#include "hash_stream.h"

// ******************************************************************
// *                                                                *
// *                                                                *
// *                     unpacked_node  methods                     *
// *                                                                *
// *                                                                *
// ******************************************************************

MEDDLY::unpacked_node::unpacked_node()
{
  parent = 0;
  extra_unhashed = 0;
  ext_uh_alloc = 0;
  ext_uh_size = 0;
  extra_hashed = 0;
  ext_h_alloc = 0;
  ext_h_size = 0;
  down = 0;
  index = 0;
  edge = 0;
  alloc = 0;
  ealloc = 0;
  size = 0;
  nnzs = 0;
  level = 0;
#ifdef DEVELOPMENT_CODE
  has_hash = false;
#endif
}

MEDDLY::unpacked_node::~unpacked_node()
{
  clear();
}

void MEDDLY::unpacked_node::clear()
{
  free(extra_unhashed);
  free(extra_hashed);
  free(down);
  free(index);
  free(edge);
  down = 0;
  index = 0;
  edge = 0;
  alloc = 0;
  ealloc = 0;
  size = 0;
  nnzs = 0;
  level = 0;
}

/*
  Initializers 
*/

void MEDDLY::unpacked_node::initRedundant(const expert_forest *f, int k, 
  node_handle node, bool full)
{
  MEDDLY_DCASSERT(f);
  MEDDLY_DCASSERT(0==f->edgeBytes());
  int nsize = f->getLevelSize(k);
  bind_to_forest(f, k, nsize, full);
  for (int i=0; i<nsize; i++) {
    down[i] = node;
  }
  if (!full) {
    for (int i=0; i<nsize; i++) index[i] = i;
    nnzs = nsize;
  }
}

void MEDDLY::unpacked_node::initRedundant(const expert_forest *f, int k, 
  float ev, node_handle node, bool full)
{
  MEDDLY_DCASSERT(f);
  MEDDLY_DCASSERT(sizeof(float)==f->edgeBytes());
  int nsize = f->getLevelSize(k);
  bind_to_forest(f, k, nsize, full);
  for (int i=0; i<nsize; i++) {
    down[i] = node;
    ((float*)edge)[i] = ev;
  }
  if (!full) {
    for (int i=0; i<nsize; i++) index[i] = i;
    nnzs = nsize;
  }
}

void MEDDLY::unpacked_node::initRedundant(const expert_forest *f, int k, 
  int ev, node_handle node, bool full)
{
  MEDDLY_DCASSERT(f);
  MEDDLY_DCASSERT(sizeof(int)==f->edgeBytes());
  int nsize = f->getLevelSize(k);
  bind_to_forest(f, k, nsize, full);
  for (int i=0; i<nsize; i++) {
    down[i] = node;
    ((int*)edge)[i] = ev;
  }
  if (!full) {
    for (int i=0; i<nsize; i++) index[i] = i;
    nnzs = nsize;
  }
}

void MEDDLY::unpacked_node::initIdentity(const expert_forest *f, int k, 
  int i, node_handle node, bool full)
{
  MEDDLY_DCASSERT(f);
  MEDDLY_DCASSERT(0==f->edgeBytes());
  int nsize = f->getLevelSize(k);
  if (full) {
    bind_to_forest(f, k, nsize, full);
    memset(down, 0, nsize * sizeof(node_handle));
    down[i] = node;
  } else {
    bind_to_forest(f, k, 1, full);
    nnzs = 1;
    down[0] = node;
    index[0] = i;
  }
}

void MEDDLY::unpacked_node::initIdentity(const expert_forest *f, int k, 
  int i, int ev, node_handle node, bool full)
{
  MEDDLY_DCASSERT(f);
  MEDDLY_DCASSERT(sizeof(int)==f->edgeBytes());
  int nsize = f->getLevelSize(k);
  if (full) {
    bind_to_forest(f, k, nsize, full);
    memset(down, 0, nsize * sizeof(node_handle));
    memset(edge, 0, nsize * sizeof(int));
    down[i] = node;
    ((int*)edge)[i] = ev;
  } else {
    bind_to_forest(f, k, 1, full);
    nnzs = 1;
    down[0] = node;
    ((int*)edge)[0] = ev;
    index[0] = i;
  }
}

void MEDDLY::unpacked_node::initIdentity(const expert_forest *f, int k, 
  int i, float ev, node_handle node, bool full)
{
  MEDDLY_DCASSERT(f);
  MEDDLY_DCASSERT(sizeof(float)==f->edgeBytes());
  int nsize = f->getLevelSize(k);
  if (full) {
    bind_to_forest(f, k, nsize, full);
    memset(down, 0, nsize * sizeof(node_handle));
    memset(edge, 0, nsize * sizeof(float));
    down[i] = node;
    ((float*)edge)[i] = ev;
  } else {
    bind_to_forest(f, k, 1, full);
    nnzs = 1;
    down[0] = node;
    ((float*)edge)[0] = ev;
    index[0] = i;
  }
}

/*
  Usage
*/

void MEDDLY::unpacked_node::show(output &s, bool details) const
{
  int stop;
  if (isSparse()) {
    if (details) s << "nnzs: " << long(nnzs) << " ";
    s << "down: (";
    stop = nnzs;
  } else {
    if (details) s << "size: " << long(size) << " ";
    s << "down: [";
    stop = size;
  }

  for (int z=0; z<stop; z++) {
    if (isSparse()) {
      if (z) s << ", ";
      s << long(i(z)) << ":";
    } else {
      if (z) s.put('|');
    }
    if (parent->edgeBytes()) {
      s.put('<');
      parent->showEdgeValue(s, eptr(z));
      s.put(", ");
    }
    if (parent->isTerminalNode(d(z))) {
      parent->showTerminal(s, d(z));
    } else {
      s.put(long(d(z)));
    }
    if (parent->edgeBytes()) s.put('>');
  }

  if (isSparse()) {
    s.put(')');
  } else {
    s.put(']');
  }

  // show extra header stuff
  if (ext_uh_size) {
    parent->showUnhashedHeader(s, extra_unhashed);
  }
  if (ext_h_size) {
    parent->showHashedHeader(s, extra_hashed);
  }
}

void MEDDLY::unpacked_node::write(output &s, const node_handle* map) const
{
  int stop;
  if (isSparse()) {
    s.put(long(-nnzs));
    stop = nnzs;
  } else {
    s.put(long(size));
    stop = size;
  }

  //
  // write indexes (sparse only)
  //
  if (isSparse()) {
    s.put('\n');
    s.put('\t');
    for (int z=0; z<nnzs; z++) {
      s.put(' ');
      s.put(long(i(z)));
    }
  }

  //
  // write down pointers
  //
  s.put('\n');
  s.put('\t');
  for (int z=0; z<stop; z++) {
    s.put(' ');
    if (parent->isTerminalNode(d(z))) {
      parent->writeTerminal(s, d(z));
    } else {
      s.put(long( map ? map[d(z)] : d(z) ));
    }
  }

  // 
  // write edges
  //
  if (parent->edgeBytes()) {
    s.put('\n');
    s.put('\t');
    for (int z=0; z<stop; z++) {
      s.put(' ');
      parent->showEdgeValue(s, eptr(z));
    }
  }
  s.put('\n');


  // write extra header stuff
  // this goes LAST so we can read it into a built node
  if (ext_uh_size) {
    parent->writeUnhashedHeader(s, extra_unhashed);
  }
  if (ext_h_size) {
    parent->writeHashedHeader(s, extra_hashed);
  }

}

void MEDDLY::unpacked_node
::resize(int ns)
{
  size = ns;
  nnzs = ns;
  if (size > alloc) {
    int nalloc = ((ns/8)+1)*8;
    MEDDLY_DCASSERT(nalloc > ns);
    MEDDLY_DCASSERT(nalloc>0);
    MEDDLY_DCASSERT(nalloc>alloc);
    down = (node_handle*) realloc(down, nalloc*sizeof(node_handle));
    if (0==down) throw error(error::INSUFFICIENT_MEMORY);
    index = (int*) realloc(index, nalloc*sizeof(int));
    if (0==index) throw error(error::INSUFFICIENT_MEMORY);
    alloc = nalloc;
  }
  if (edge_bytes * size > ealloc) {
    int nalloc = ((edge_bytes * size)/8+1)*8;
    MEDDLY_DCASSERT(nalloc>0);
    MEDDLY_DCASSERT(nalloc>ealloc);
    edge = realloc(edge, nalloc);
    if (0==edge) throw error(error::INSUFFICIENT_MEMORY);
    ealloc = nalloc;
  }
}

void MEDDLY::unpacked_node::bind_to_forest(const expert_forest* f, int k, int ns, bool full)
{
  parent = f;
  level = k;
  is_full = full;
  edge_bytes = f->edgeBytes();
  resize(ns);

  // Allocate headers
  ext_h_size = parent->hashedHeaderBytes();
  if (ext_h_size > ext_h_alloc) {
    ext_h_alloc = ((ext_h_size/8)+1)*8;
    MEDDLY_DCASSERT(ext_h_alloc > ext_h_size);
    MEDDLY_DCASSERT(ext_h_alloc>0);
    extra_hashed =  realloc(extra_hashed, ext_h_alloc);
    if (0==extra_hashed) throw error(error::INSUFFICIENT_MEMORY);
  }

  ext_uh_size = parent->unhashedHeaderBytes();
  if (ext_uh_size > ext_uh_alloc) {
    ext_uh_alloc = ((ext_uh_size/8)+1)*8;
    MEDDLY_DCASSERT(ext_uh_alloc > ext_uh_size);
    MEDDLY_DCASSERT(ext_uh_alloc>0);
    extra_unhashed =  realloc(extra_unhashed, ext_uh_alloc);
    if (0==extra_unhashed) throw error(error::INSUFFICIENT_MEMORY);
  }
}


/*
void MEDDLY::unpacked_node
::resize_header(int extra_bytes)
{
  ext_h_size = extra_bytes;
  if (ext_h_size > ext_h_alloc) {
    ext_h_alloc = ((ext_h_size/8)+1)*8;
    MEDDLY_DCASSERT(ext_h_alloc > ext_h_size);
    MEDDLY_DCASSERT(ext_h_alloc>0);
    extra_hashed =  realloc(extra_hashed, ext_h_alloc);
    if (0==extra_hashed) throw error(error::INSUFFICIENT_MEMORY);
  }
}
*/

void MEDDLY::unpacked_node::computeHash()
{
#ifdef DEVELOPMENT_CODE
  MEDDLY_DCASSERT(!has_hash);
#endif
  
  hash_stream s;
  s.start(0);

  if (ext_h_size) {
    s.push(extra_hashed, ext_h_size);
  }
  
  if (isSparse()) {
    if (parent->areEdgeValuesHashed()) {
      for (int z=0; z<nnzs; z++) {
        MEDDLY_DCASSERT(d(z)!=parent->getTransparentNode());
        s.push(i(z), d(z), ei(z));
      }
    } else {
      for (int z=0; z<nnzs; z++) {
        MEDDLY_DCASSERT(d(z)!=parent->getTransparentNode());
        s.push(i(z), d(z));
      }
    }
  } else {
    if (parent->areEdgeValuesHashed()) {
      for (int n=0; n<size; n++) {
        if (d(n)!=parent->getTransparentNode()) {
          s.push(n, d(n), ei(n));
        }
      }
    } else {
      for (int n=0; n<size; n++) {
        if (d(n)!=parent->getTransparentNode()) {
          s.push(n, d(n));
        }
      }
    }
  }
  h = s.finish();
#ifdef DEVELOPMENT_CODE
  has_hash = true;
#endif
}


// ******************************************************************
// *                                                                *
// *                                                                *
// *                   node_storage_style methods                   *
// *                                                                *
// *                                                                *
// ******************************************************************

MEDDLY::node_storage_style::node_storage_style()
{
}

MEDDLY::node_storage_style::~node_storage_style()
{
}

// ******************************************************************
// *                                                                *
// *                                                                *
// *                      node_storage methods                      *
// *                                                                *
// *                                                                *
// ******************************************************************

MEDDLY::node_storage::node_storage(expert_forest* f)
{
  counts = 0;
  nexts = 0;

  parent = f;
  stats = &parent->changeStats();
}

MEDDLY::node_storage::~node_storage()
{
  // nothing, derived classes must handle everything
}


#ifndef INLINED_COUNT
MEDDLY::node_handle
MEDDLY::node_storage::getCountOf(node_address addr) const
{
  MEDDLY_DCASSERT(counts);
  MEDDLY_DCASSERT(addr > 0);
  return counts[addr];
}

void
MEDDLY::node_storage::setCountOf(node_address addr, MEDDLY::node_handle c)
{
  MEDDLY_DCASSERT(counts);
  MEDDLY_DCASSERT(addr > 0);
  counts[addr] = c;
}

MEDDLY::node_handle
MEDDLY::node_storage::incCountOf(node_address addr)
{
  MEDDLY_DCASSERT(counts);
  MEDDLY_DCASSERT(addr > 0);
  return ++counts[addr];
}
;

MEDDLY::node_handle
MEDDLY::node_storage::decCountOf(node_address addr)
{
  MEDDLY_DCASSERT(counts);
  MEDDLY_DCASSERT(addr > 0);
  return --counts[addr];
}
#endif

#ifndef INLINED_NEXT
MEDDLY::node_handle
MEDDLY::node_storage::getNextOf(node_address addr) const
{
  MEDDLY_DCASSERT(nexts);
  MEDDLY_DCASSERT(addr > 0);
  return nexts[addr];
}

void
MEDDLY::node_storage::setNextOf(node_address addr, MEDDLY::node_handle n)
{
  MEDDLY_DCASSERT(nexts);
  MEDDLY_DCASSERT(addr > 0);
  nexts[addr] = n;
}
#endif


void MEDDLY::node_storage::dumpInternal(output &s, unsigned flags) const
{
  dumpInternalInfo(s);
  s << "Data array by record:\n";
  for (node_address a=1; a > 0; ) {
    s.flush();
    a = dumpInternalNode(s, a, flags);
  } // for a
  dumpInternalTail(s);
  s.flush();
}

/*
void MEDDLY::node_storage::localInitForForest(const expert_forest* f)
{
  // default - do nothing
}
*/
