
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
#include "unique_table.h"
#include "hash_stream.h"
#include "storage/bytepack.h"
#include "reordering/reordering_factory.h"

// for timestamps.
// to do - check during configuration that these are present,
// and act accordingly here

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

//#include <set>
//#include <queue>
//#include <vector>

// #define DEBUG_CLEANUP
// #define DEBUG_ADDRESS_RESIZE
// #define DEBUG_GC
// #define DEBUG_CREATE_REDUCED
// #define DEBUG_GC
// #define DEBUG_WRITE
// #define DEBUG_READ 
// #define DEBUG_HANDLE_FREELIST

// #define TRACK_DELETIONS

// Thoroughly check reference counts.
// Very slow.  Use only for debugging.
// #define VALIDATE_INCOUNTS
// #define VALIDATE_INCOUNTS_ON_DELETE

// #define GC_OFF

// #define REPORT_ON_DESTROY
// #define DUMP_ON_FOREST_DESTROY

const int a_min_size = 1024;


// ******************************************************************
// *                                                                *
// *                    forest::policies methods                    *
// *                                                                *
// ******************************************************************

const unsigned char MEDDLY::forest::policies::ALLOW_FULL_STORAGE    = 0x01;
const unsigned char MEDDLY::forest::policies::ALLOW_SPARSE_STORAGE  = 0x02;

MEDDLY::forest::policies::policies()
{
  nodestor = 0; // should cause an exception later
}

MEDDLY::forest::policies::policies(bool rel) 
{
  useDefaults(rel);
}

void MEDDLY::forest::policies::useDefaults(bool rel)
{
  reduction = rel ? IDENTITY_REDUCED : FULLY_REDUCED;
  storage_flags = ALLOW_FULL_STORAGE | ALLOW_SPARSE_STORAGE;
  deletion = OPTIMISTIC_DELETION;
  compact_min = 100;
  compact_max = 1000000;
  compact_frac = 40;
  zombieTrigger = 1000000;
  orphanTrigger = 500000;
  compactAfterGC = false;
  compactBeforeExpand = true;

  // nodestor = CLASSIC_STORAGE;
  nodestor = SIMPLE_GRID;
  // nodestor = SIMPLE_ARRAY;
  // nodestor = SIMPLE_HEAP;
  // nodestor = SIMPLE_NONE;
  // nodestor = COMPACT_GRID;
  reorder = reordering_type::SINK_DOWN;
  swap = variable_swap_type::VAR;
}

// ******************************************************************
// *                                                                *
// *                    forest::statset  methods                    *
// *                                                                *
// ******************************************************************

MEDDLY::forest::statset::statset()
{
  reclaimed_nodes = 0;
  num_compactions = 0;
  garbage_collections = 0;
  zombie_nodes = 0;
  orphan_nodes = 0;
  active_nodes = 0;
  peak_active = 0;
  memory_used = 0;
  memory_alloc = 0;
  peak_memory_used = 0;
  peak_memory_alloc = 0;
  memory_UT = 0;
  peak_memory_UT = 0;
  max_UT_chain = 0;
}

// ******************************************************************
// *                                                                *
// *                     forest::logger methods                     *
// *                                                                *
// ******************************************************************

MEDDLY::forest::logger::logger()
{
  nfix = true;
  /* 
    All other settings to false;
    the default logger does nothing!
  */
  node_counts = false;
  time_stamps = false;

  /* Do this now, regardless */
#ifdef HAVE_SYS_TIME_H
  static struct timeval curr_time;
  static struct timezone tz;
  gettimeofday(&curr_time, &tz);
  startsec = curr_time.tv_sec;
  startusec = curr_time.tv_usec;
#else 
  startsec = 0;
  startusec = 0;
#endif
}

MEDDLY::forest::logger::~logger()
{
}

void MEDDLY::forest::logger::currentTime(long &sec, long &usec)
{
#ifdef HAVE_SYS_TIME_H
  static struct timeval curr_time;
  static struct timezone tz;
  gettimeofday(&curr_time, &tz);
  sec = curr_time.tv_sec - startsec;
  usec = curr_time.tv_usec - startusec;
  if (usec<0) {
    sec--;
    usec += 1000000;
  }
#else
  sec = 0;
  usec = 0;
#endif
}

// ******************************************************************
// *                                                                *
// *                                                                *
// *                         forest methods                         *
// *                                                                *
// *                                                                *
// ******************************************************************

unsigned MEDDLY::forest::gfid = 0;

MEDDLY::forest::policies MEDDLY::forest::mddDefaults;
MEDDLY::forest::policies MEDDLY::forest::mxdDefaults;

MEDDLY::forest
::forest(int ds, domain* _d, bool rel, range_type t, edge_labeling ev, 
  const policies &p,int* lrr) : deflt(p)
{
  // FID
  //
  fid = ++gfid;

#ifdef DEBUG_CLEANUP
  fprintf(stderr, "Creating forest #%d in domain #%d\n", ds, _d->ID());
#endif
  d_slot = ds;
  is_marked_for_deletion = false;
  d = _d;
  isRelation = rel;
  rangeType = t;
  edgeLabel = ev;

  
    if(lrr==NULL){
       
        if(isUserDefinedReduced()){
		throw error(error::INVALID_POLICY);
        }

	else{

        if(isRelation)
        {
            int span = 2*(d->getNumVariables()) + 1;
            lrr=(int*)malloc(sizeof(int)*span);
            
            if(isQuasiReduced())
                for(int i=0;i<span;i++)
                {lrr[i]=i==0?-4:(i%2==0?-2:-1);}
            
            else if (isFullyReduced())
                for(int i=0;i<span;i++)
                {lrr[i]=i==0?-4:(i%2==0?-1:-1);}
            
            else if (isIdentityReduced())
                for(int i=0;i<span;i++)
                {lrr[i]=i==0?-4:(i%2==0?-3:-1);}
        }
        else
        {
            lrr=(int*)malloc(sizeof(int)*(d->getNumVariables()+1));
            lrr[0]=-4;
            if(isQuasiReduced())
                for(int i=1;i<=d->getNumVariables();i++)
                    lrr[i]=-2;
            
            else
                if (isFullyReduced())
                for(int i=1;i<=d->getNumVariables();i++)
                    lrr[i]=-1;
            
        }
      
	}

	level_reduction_rule=lrr;        
    }
	else if(isUserDefinedReduced()){

    	level_reduction_rule=lrr;
    }
	else
		throw error(error::INVALID_POLICY);
    
    
    
    
  // check policies
  if (!isRelation) {
    if (policies::IDENTITY_REDUCED == deflt.reduction)
      throw error(error::INVALID_POLICY);
      
    for(int i=1;i<=d->getNumVariables();i++)
        if(level_reduction_rule[i]==-3)              //isIdentityReduced()
         throw error(error::INVALID_POLICY);
  }
  //
  // Initialize array of operations
  //
  opCount = 0;
  szOpCount = 0;
  //
  // Initialize list of registered dd_edges
  //
  firstHole = -1; // firstHole < 0 indicates no holes.
  firstFree = 0;
  sz = 256;

  // Create an array to store pointers to dd_edges.
  edge = (edge_data *) malloc(sz * sizeof(edge_data));
  for (unsigned i = 0; i < sz; ++i) {
    edge[i].nextHole = -1;
    edge[i].edge = 0;
  }

  //
  // Empty logger
  //

  theLogger = 0;
}

MEDDLY::forest::~forest()
{
#ifdef DEBUG_CLEANUP
  fprintf(stderr, "Deleting forest #%d in domain #%d\n", d_slot, d->ID());
#endif
  // operations are deleted elsewhere...
  free(opCount);
  // Make SURE our edges are orphaned
  for (unsigned i = 0; i < firstFree; ++i) {
    if (edge[i].edge) edge[i].edge->orphan();
  }
  free(edge);
  // NOTE: since the user is provided with the dd_edges instances (as opposed
  // to a pointer), the user program will automatically call the
  // destructor for each dd_edge when the corresponding variable goes out of
  // scope. Therefore there is no need to destruct dd_edges from here.
  d->unlinkForest(this, d_slot);
}

void MEDDLY::forest::markForDeletion()
{
#ifdef DEBUG_CLEANUP
  fprintf(stderr, "Marking forest #%d for deletion in domain #%d\n", d_slot, d->ID());
#endif
  if (is_marked_for_deletion) return;
  is_marked_for_deletion = true;
  // deal with operations associated with this forest
  for (int i=0; i<szOpCount; i++) 
    if (opCount[i]) {
      operation* op = operation::getOpWithIndex(i);
      op->markForDeletion();
    }
  unregisterDDEdges();
}

void MEDDLY::forest::createEdgeForVar(int vh, bool vp, const bool* terms, dd_edge& a)
{
  throw error(error::TYPE_MISMATCH);
}

void MEDDLY::forest::createEdgeForVar(int vh, bool vp, const int* terms, dd_edge& a)
{
  throw error(error::TYPE_MISMATCH);
}

void MEDDLY::forest::createEdgeForVar(int vh, bool vp, const float* terms, dd_edge& a)
{
  throw error(error::TYPE_MISMATCH);
}

void MEDDLY::forest::createEdge(const int* const* vlist, int N, dd_edge &e)
{
  throw error(error::TYPE_MISMATCH);
}

void MEDDLY::forest
::createEdge(const int* const* vlist, const int* terms, int N, dd_edge &e)
{
  throw error(error::TYPE_MISMATCH);
}

void MEDDLY::forest
::createEdge(const int* const* vlist, const float* terms, int N, dd_edge &e)
{
  throw error(error::TYPE_MISMATCH);
}

void MEDDLY::forest
::createEdge(const int* const* vl, const int* const* vpl, int N, dd_edge &e)
{
  throw error(error::TYPE_MISMATCH);
}

void MEDDLY::forest
::createEdge(const int* const* vlist, const int* const* vplist, const int* terms, int N, dd_edge &e)
{
  throw error(error::TYPE_MISMATCH);
}

void MEDDLY::forest
::createEdge(const int* const* vlist, const int* const* vplist, const float* terms, int N, dd_edge &e)
{
  throw error(error::TYPE_MISMATCH);
}

void MEDDLY::forest::createEdge(bool val, dd_edge &e)
{
  throw error(error::TYPE_MISMATCH);
}

void MEDDLY::forest::createEdge(int val, dd_edge &e)
{
  throw error(error::TYPE_MISMATCH);
}

void MEDDLY::forest::createEdge(float val, dd_edge &e)
{
  throw error(error::TYPE_MISMATCH);
}

void MEDDLY::forest::evaluate(const dd_edge &f, const int* vl, bool &t) const
{
  throw error(error::TYPE_MISMATCH);
}

void MEDDLY::forest::evaluate(const dd_edge &f, const int* vl, int &t) const
{
  throw error(error::TYPE_MISMATCH);
}

void MEDDLY::forest::evaluate(const dd_edge &f, const int* vl, float &t) const
{
  throw error(error::TYPE_MISMATCH);
}

void MEDDLY::forest
::evaluate(const dd_edge& f, const int* vl, const int* vpl, bool &t) const
{
  throw error(error::TYPE_MISMATCH);
}

void MEDDLY::forest
::evaluate(const dd_edge& f, const int* vl, const int* vpl, int &t) const
{
  throw error(error::TYPE_MISMATCH);
}

void MEDDLY::forest
::evaluate(const dd_edge& f, const int* vl, const int* vpl, float &t) const
{
  throw error(error::TYPE_MISMATCH);
}

void MEDDLY::forest::getElement(const dd_edge& a, int index, int* e)
{
  throw error(error::INVALID_OPERATION);
}

void MEDDLY::forest::removeStaleComputeTableEntries()
{
  if (operation::usesMonolithicComputeTable()) {
    operation::removeStalesFromMonolithic();
  } else {
    for (int i=0; i<szOpCount; i++) 
      if (opCount[i]) {
        operation* op = operation::getOpWithIndex(i);
        op->removeStaleComputeTableEntries();
      }
  }
}

void MEDDLY::forest::removeAllComputeTableEntries()
{
  if (is_marked_for_deletion) return;
  if (operation::usesMonolithicComputeTable()) {
    is_marked_for_deletion = true;
    operation::removeStalesFromMonolithic();
    is_marked_for_deletion = false;
  } else {
    for (int i=0; i<szOpCount; i++) 
      if (opCount[i]) {
        operation* op = operation::getOpWithIndex(i);
        op->removeAllComputeTableEntries();
      }
  }
}

void MEDDLY::forest::showComputeTable(output &s, int verbLevel) const
{
  if (operation::usesMonolithicComputeTable()) {
    operation::showMonolithicComputeTable(s, verbLevel);
  } else {
    for (int i=0; i<szOpCount; i++) 
      if (opCount[i]) {
        operation* op = operation::getOpWithIndex(i);
        op->showComputeTable(s, verbLevel);
      }
  }
}

void MEDDLY::forest::registerOperation(const operation* op)
{
  MEDDLY_DCASSERT(op->getIndex() >= 0);
  if (op->getIndex() >= szOpCount) {
    // need to expand
    int newSize = ((op->getIndex() / 16) +1 )*16; // expand in chunks of 16
    int* tmp = (int*) realloc(opCount, newSize * sizeof(int));
    if (0==tmp) throw error(error::INSUFFICIENT_MEMORY);
    for ( ; szOpCount < newSize; szOpCount++) {
      tmp[szOpCount] = 0;
    }
    opCount = tmp;
  }
  opCount[op->getIndex()] ++;
}

void MEDDLY::forest::unregisterOperation(const operation* op)
{
  MEDDLY_DCASSERT(op->getIndex() >= 0);
  MEDDLY_DCASSERT(szOpCount > op->getIndex());
  MEDDLY_DCASSERT(opCount[op->getIndex()]>0);
  opCount[op->getIndex()] --;
}

void MEDDLY::forest::registerEdge(dd_edge& e) 
{
  // add to collection of edges for this forest.
  // change e.index to help find this edge at a later time.
  if (firstHole >= 0) {
    // hole available; fill it up
    int index = firstHole;
    firstHole = edge[firstHole].nextHole;
    edge[index].edge = &e;
    edge[index].nextHole = -1;
    e.setIndex(index);
  } else {
    // no holes available, add to end of array
    if (firstFree >= sz) {
      // expand edge[]
      int new_sz = sz * 2;
      edge_data* new_edge =
          (edge_data*) realloc(edge, new_sz * sizeof(edge_data));
      if (0 == new_edge) throw error(error::INSUFFICIENT_MEMORY);
      edge = new_edge;
      for (int i = sz; i < new_sz; ++i)
      {
        edge[i].nextHole = -1;
        edge[i].edge = 0;
      }
      sz = new_sz;
    }
    MEDDLY_DCASSERT(firstFree < sz);
    edge[firstFree].nextHole = -1;
    edge[firstFree].edge = &e;
    e.setIndex(firstFree);
    ++firstFree;
  }
}


void MEDDLY::forest::unregisterEdge(dd_edge& e) 
{
  // remove this edge from the collection of edges for this forest.
  // change e.index to -1.
  MEDDLY_DCASSERT(e.getIndex() >= 0);
  int index = e.getIndex();
  MEDDLY_DCASSERT(edge[index].edge == &e);
  edge[index].edge = 0;
  edge[index].nextHole = firstHole;
  firstHole = index;
  e.setIndex(-1);
}


void MEDDLY::forest::unregisterDDEdges() 
{
  // Go through the list of valid edges (value > 0), and set
  // the e.index to -1 (indicating unregistered edge).

  // ignore the NULLs; release the rest
  for (unsigned i = 0; i < firstFree; ++i) {
    if (edge[i].edge != 0) {
      MEDDLY_DCASSERT(edge[i].nextHole == -1);
      edge[i].edge->clear();
      edge[i].edge->setIndex(-1);
    }
  }

  // firstHole < 0 indicates no holes.
  for (unsigned i = 0; i < firstFree; ++i) {
    edge[i].nextHole = -1;
    edge[i].edge = 0;
  }
  firstHole = -1;
  firstFree = 0;
}

// ******************************************************************

MEDDLY::forest::edge_visitor::edge_visitor()
{
}

MEDDLY::forest::edge_visitor::~edge_visitor()
{
}


// ******************************************************************
// *                                                                *
// *                 expert_forest encoder  methods                 *
// *                                                                *
// ******************************************************************

void MEDDLY::expert_forest::bool_Tencoder::show(output &s, node_handle h)
{
  s.put(handle2value(h) ? 'T' : 'F');
}

void MEDDLY::expert_forest::bool_Tencoder::write(output &s, node_handle h)
{
  s.put(handle2value(h) ? 'T' : 'F');
}

MEDDLY::node_handle MEDDLY::expert_forest::bool_Tencoder::read(input &s)
{
  s.stripWS();
  int c = s.get_char();
  if ('T' == c) return value2handle(true);
  if ('F' == c) return value2handle(false);
  throw error(error::INVALID_FILE);
}

// ******************************************************************

void MEDDLY::expert_forest::int_Tencoder::show(output &s, node_handle h)
{
  s << "t" << handle2value(h);
}

void MEDDLY::expert_forest::int_Tencoder::write(output &s, node_handle h)
{
  s << "t" << handle2value(h);
}

MEDDLY::node_handle MEDDLY::expert_forest::int_Tencoder::read(input &s)
{
  s.stripWS();
  int c = s.get_char();
  if ('t' != c) throw error(error::INVALID_FILE);
  return value2handle(s.get_integer());
}

// ******************************************************************

void MEDDLY::expert_forest::float_Tencoder::show(output &s, node_handle h)
{
  s << "t" << handle2value(h);
}

void MEDDLY::expert_forest::float_Tencoder::write(output &s, node_handle h)
{
  s.put('t');
  s.put(handle2value(h), 8, 8, 'e');
}

MEDDLY::node_handle MEDDLY::expert_forest::float_Tencoder::read(input &s)
{
  s.stripWS();
  int c = s.get_char();
  if ('t' != c) throw error(error::INVALID_FILE);
  return value2handle(s.get_real());
}

// ******************************************************************
// ******************************************************************

void MEDDLY::expert_forest::int_EVencoder::show(output &s, const void* ptr)
{
  int val;
  readValue(ptr, val);
  s << val;
}

void MEDDLY::expert_forest::int_EVencoder::write(output &s, const void* ptr)
{
  int val;
  readValue(ptr, val);
  s << val;
}

void MEDDLY::expert_forest::int_EVencoder::read(input &s, void* ptr)
{
  writeValue(ptr, int(s.get_integer()));
}

// ******************************************************************

void MEDDLY::expert_forest::float_EVencoder::show(output &s, const void* ptr)
{
  float val;
  readValue(ptr, val);
  s << val;
}

void MEDDLY::expert_forest::float_EVencoder::write(output &s, const void* ptr)
{
  float val;
  readValue(ptr, val);
  s.put(val, 8, 8, 'e');
}

void MEDDLY::expert_forest::float_EVencoder::read(input &s, void* ptr)
{
  writeValue(ptr, float(s.get_real()));
}

// ******************************************************************
// *                                                                *
// *                                                                *
// *                     expert_forest  methods                     *
// *                                                                *
// *                                                                *
// ******************************************************************

const unsigned MEDDLY::expert_forest::HUMAN_READABLE_MEMORY   = 0x0001;
const unsigned MEDDLY::expert_forest::BASIC_STATS             = 0x0002;
const unsigned MEDDLY::expert_forest::EXTRA_STATS             = 0x0004;
const unsigned MEDDLY::expert_forest::FOREST_STATS            = 0x0008;
const unsigned MEDDLY::expert_forest::STORAGE_STATS           = 0x0010;
const unsigned MEDDLY::expert_forest::STORAGE_DETAILED        = 0x0020;
const unsigned MEDDLY::expert_forest::UNIQUE_TABLE_STATS      = 0x0040;
const unsigned MEDDLY::expert_forest::UNIQUE_TABLE_DETAILED   = 0x0080;
const unsigned MEDDLY::expert_forest::HOLE_MANAGER_STATS      = 0x0100;
const unsigned MEDDLY::expert_forest::HOLE_MANAGER_DETAILED   = 0x0200;

//
// Display flags
//

const unsigned int MEDDLY::expert_forest::SHOW_DELETED    = 0x10;
const unsigned int MEDDLY::expert_forest::SHOW_ZOMBIE     = 0x08;
const unsigned int MEDDLY::expert_forest::SHOW_DETAILS    = 0x04;
const unsigned int MEDDLY::expert_forest::SHOW_INDEX      = 0x02;
const unsigned int MEDDLY::expert_forest::SHOW_TERMINALS  = 0x01;


MEDDLY::expert_forest::expert_forest(int ds, domain *d, bool rel, range_type t,
  edge_labeling ev, const policies &p,int* level_reduction_rule)
: forest(ds, d, rel, t, ev, p,level_reduction_rule)
{
  //
  // Inltialize address array
  //
  a_size = a_min_size;
  address = (node_header *) malloc(a_size * sizeof(node_header));
  if (0 == address) throw error(error::INSUFFICIENT_MEMORY);
  stats.incMemAlloc(a_size * sizeof(node_header));
  memset(address, 0, a_size * sizeof(node_header));
  a_last = a_next_shrink = 0;
  for (int i=0; i<8; i++) a_unused[i] = 0;
  a_lowest_index = 8;

  // Initialize variable order
  var_order = d->makeDefaultVariableOrder();

  //
  // Initialize misc. protected data
  //
  terminalNodesAreStale = false;

  //
  // Initialize misc. private data
  //
  unique = new unique_table(this);
  performing_gc = false;
  in_validate = 0;
  in_val_size = 0;
  delete_depth = 0;

  //
  // Initialize node characteristics to defaults
  //
  edge_bytes = 0;
  hash_edge_values = false;
  unhashed_bytes = 0;
  hashed_bytes = 0;
}


MEDDLY::expert_forest::~expert_forest() 
{
#ifdef REPORT_ON_DESTROY
  printf("Destroying forest.  Stats:\n");
  reportMemoryUsage(stdout, "\t", 9);
#endif
  // Address array
  free(address);

  delete nodeMan;

  // unique table
  delete unique;

  // Misc. private data
  free(in_validate);
}

void MEDDLY::expert_forest::initializeForest()
{
  if (!deflt.nodestor) {
    throw error(error::MISCELLANEOUS);
  }

  //
  // Initialize node storage
  //
  nodeMan = deflt.nodestor->createForForest(this);

}

// ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
// '                                                                '
// '                   public debugging   methods                   '
// '                                                                '
// ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

void MEDDLY::expert_forest::dump(output &s, unsigned int flags) const
{
  for (long p=0; p<=a_last; p++) {
    if (showNode(s, p, flags | SHOW_INDEX)) {
      s.put('\n');
      s.flush();
    }
  }
}

void MEDDLY::expert_forest::dumpInternal(output &s) const
{
  s << "Internal forest storage\n";
  for (int i=0; i<8; i++) {
    s << "First " << i << "-byte unused node index: " << a_unused[i] << "\n";
  }
  int awidth = digits(a_last);
  s << " Node# :  ";
  for (node_handle p=1; p<=a_last; p++) {
    if (p) s.put(' ');
    s.put(long(p), awidth);
  }
  s << "\nLevel  : [";
  for (node_handle p=1; p<=a_last; p++) {
    if (p) s.put('|');
    s.put(long(address[p].level), awidth);
  }
  s << "]\n";
  s << "\nOffset : [";
  for (node_handle p=1; p<=a_last; p++) {
    if (p) s.put('|');
    s.put(long(address[p].offset), awidth);
  }
  s << "]\n";
  s << "\nCache  : [";
  for (node_handle p=1; p<=a_last; p++) {
    if (p) s.put('|');
    s.put(long(address[p].cache_count), awidth);
  }
  s << "]\n\n";

  nodeMan->dumpInternal(s, 0x03);
 
  unique->show(s);
  s.flush();
}

void MEDDLY::expert_forest::dumpUniqueTable(output &s) const
{
  unique->show(s);
}

void MEDDLY::expert_forest::validateIncounts(bool exact)
{
#ifdef VALIDATE_INCOUNTS
  static int idnum = 0;
  idnum++;

  // Inspect every active node's down pointers to determine
  // the incoming count for every active node.
  
  node_handle sz = getLastNode() + 1;
  if (sz > in_val_size) {
    in_validate = (node_handle*) 
                  realloc(in_validate, a_size * sizeof(node_handle));
    in_val_size = a_size;
  }
  MEDDLY_DCASSERT(sz <= in_val_size);
  memset(in_validate, 0, sizeof(node_handle) * sz);
  node_reader P;
  for (node_handle i = 1; i < sz; ++i) {
    MEDDLY_DCASSERT(!isTerminalNode(i));
    if (!isActiveNode(i)) continue;
    initNodeReader(P, i, false);

    // add to reference counts
    for (int z=0; z<P.getNNZs(); z++) {
      if (isTerminalNode(P.d(z))) continue;
      MEDDLY_CHECK_RANGE(0, P.d(z), sz);
      in_validate[P.d(z)]++;
    }
  } // for i

  // Add counts for registered dd_edges
  nodecounter foo(this, in_validate);
  visitRegisteredEdges(foo);
  
  // Validate the incoming count stored with each active node using the
  // in_count array computed above
  for (node_handle i = 1; i < sz; ++i) {
    MEDDLY_DCASSERT(!isTerminalNode(i));
    if (!isActiveNode(i)) continue;
    bool fail = exact
      ?  in_validate[i] != getNodeInCount(i)
      :  in_validate[i] >  getNodeInCount(i);
    if (fail) {
      printf("Validation #%d failed\n", idnum);
      long l_i = i;
      long l_v = in_validate[i];
      long l_c = getNodeInCount(i);
      printf("For node %ld\n\tcount: %ld\n\tnode:  %ld\n", l_i, l_v, l_c);
      dump(stdout, SHOW_DETAILS);
      MEDDLY_DCASSERT(0);
      throw error(error::MISCELLANEOUS);
    }
    // Note - might not be exactly equal
    // because there could be dd_edges that refer to nodes
    // and we didn't count them.
  }

#ifdef TRACK_DELETIONS
  printf("Incounts validated #%d\n", idnum);
#endif
#endif
}


void MEDDLY::expert_forest::countNodesByLevel(long* active) const
{
  int L = getNumVariables();
  int l;
  if (isForRelations()) {
    l = -L;
  } else {
    l = 0;
  }

  for (; l<=L; l++) active[l] = 0;

  for (long p=0; p<=a_last; p++) {
    if (address[p].isDeleted()) continue; 
    active[address[p].level]++;
  }
}

// ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
// '                                                                '
// '                      public handy methods                      '
// '                                                                '
// ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

MEDDLY::node_handle* 
MEDDLY::expert_forest
::markNodesInSubgraph(const node_handle* root, int N, bool sort) const
{
  MEDDLY_DCASSERT(root);

  // initialize lists
  bool* inList = new bool[a_last];
  for (int i=0; i<a_last; i++) inList[i] = false;
  inList--;
  int mlen = 0;
  int msize = 1024;
  node_handle* marked = (node_handle*) malloc(msize * sizeof(node_handle));

  // Initialize search
  for (int i=0; i<N; i++) {
    if (isTerminalNode(root[i])) continue;
    marked[mlen] = root[i];
    mlen++;
    inList[root[i]] = true;
  }

  unpacked_node *M = unpacked_node::useUnpackedNode();

  // Breadth-first search
  for (int mexpl=0; mexpl<mlen; mexpl++) {
    // explore node marked[mexpl]
    M->initFromNode(this, marked[mexpl], false);
    for (int i=0; i<M->getNNZs(); i++) {
      if (isTerminalNode(M->d(i))) continue;
      MEDDLY_CHECK_RANGE(0, M->d(i)-1, a_last);
      if (inList[M->d(i)]) continue;
      // add dn to list
      if (mlen+1 >= msize) { 
          // expand.  Note we're leaving an extra slot
          // at the end, for the terminal 0.
          msize += 1024;
          node_handle* new_marked = (node_handle*) 
            realloc(marked, msize*sizeof(node_handle));
          if (0==new_marked) throw error(error::INSUFFICIENT_MEMORY);
          marked = new_marked;
      }
      inList[M->d(i)] = true;
      marked[mlen] = M->d(i);
      mlen++;
    } // for i
  } // for mexpl

  unpacked_node::recycle(M);

  // sort
  if (sort && mlen>0) {
    mlen = 0;
    for (int i=1; i<=a_last; i++) if (inList[i]) {
      marked[mlen] = i;
      mlen++;
    }
  }

  // cleanup
  inList++;
  delete[] inList;
  if (0==mlen) {
    free(marked);
    return 0;
  }
  // add 0 to the list
  marked[mlen] = 0;
  return marked;
}

long MEDDLY::expert_forest::getNodeCount(node_handle p) const
{
  node_handle* list = markNodesInSubgraph(&p, 1, true);
  if (0==list) return 0;
  long i;
  for (i=0; list[i]; i++) { }
  free(list);
  return i;
}

long MEDDLY::expert_forest::getEdgeCount(node_handle p, bool countZeroes) const
{
  node_handle* list = markNodesInSubgraph(&p, 1, true);
  if (0==list) return 0;
  long ec=0;
  unpacked_node *M = unpacked_node::useUnpackedNode();
  for (long i=0; list[i]; i++) {
    M->initFromNode(this, list[i], countZeroes);
    ec += countZeroes ? M->getSize() : M->getNNZs();
  }
  unpacked_node::recycle(M);
  free(list);
  return ec;
}

bool MEDDLY::expert_forest
::showNode(output &s, node_handle p, unsigned int flags) const
{
  /*
    Deal with cases where nothing will be displayed.
  */
  if (isTerminalNode(p)) {
    if (!(flags & SHOW_TERMINALS))  return false;
  } else 
  if (isDeletedNode(p)) {
    if (!(flags & SHOW_DELETED))    return false;
  } else 
  if (isZombieNode(p)) {
    if (!(flags & SHOW_ZOMBIE))     return false;
  }

  /*
    Show the node index, if selected.
  */
  if (flags & SHOW_INDEX) {
    int nwidth = digits(a_last);
    s.put(long(p), nwidth);
    s.put('\t');
  }

  /*
    Deal with special cases
  */
  if (isTerminalNode(p)) {
    s << "(terminal)";
    return true;
  }
  if (isDeletedNode(p)) {
    s << "DELETED";
    return true;
  }
  if (isZombieNode(p)) {
    s << "Zombie cc: " <<  -address[p].cache_count;
    return true;
  }

  /*
    Ordinary node
  */
  if (flags & SHOW_DETAILS) {
    // node: was already written.
    const variable* v = getDomain()->getVar(getVarByLevel(ABS(getNodeLevel(p))));
    if (v->getName()) {
      s << " level: " << v->getName();
    } else {
      s << " level: " <<  ABS(getNodeLevel(p));
    }
    s.put( (getNodeLevel(p) < 0) ? '\'' : ' ' );
    s << " in: " << getNodeInCount(p);
    s << " cc: " << getNodeCacheCount(p);
  } else {
    s << "node: " << long(p);
  }

  s.put(' ');
  unpacked_node* un = unpacked_node::newFromNode(this, p, unpacked_node::AS_STORED);
  un->show(s, flags & SHOW_DETAILS);
  unpacked_node::recycle(un);

  return true;
}

void MEDDLY::expert_forest
::showNodeGraph(output &s, const node_handle* p, int n) const
{
  node_handle* list = markNodesInSubgraph(p, n, true);
  if (0==list) return;

  // Print by levels
  for (int k = getNumVariables(); k; )
  {
    bool printed = false;
    for (long i=0; list[i]; i++) {
      if (getNodeLevel(list[i]) != k) continue;

      if (!printed) {
        const variable* v = getDomain()->getVar(getVarByLevel(ABS(k)));
        char primed = (k>0) ? ' ' : '\'';
        if (v->getName()) {
          s << "Level: " << k << " Var: " << v->getName() << primed << '\n';
        } else {
          s << "Level: " << k << " Var: " << getVarByLevel(ABS(k)) << primed << '\n';
        }
        printed = true;
      }

      s << "  ";
      showNode(s, list[i]);
      s.put('\n');
    }
    
    // next level
    k *= -1;
    if (k>0) k--;
  } // for k

  free(list);
}

void MEDDLY::expert_forest
::reportStats(output &s, const char* pad, unsigned flags) const
{
  if (flags & BASIC_STATS) {
    bool human = flags & HUMAN_READABLE_MEMORY;
    s << pad << getCurrentNumNodes() << " current nodes\n";
    s << pad << getPeakNumNodes() << " peak nodes\n" << pad;
    s.put_mem(getCurrentMemoryUsed(), human);
    s << " current memory used\n" << pad;
    s.put_mem(getPeakMemoryUsed(), human);
    s << " peak memory used\n" << pad;
    s.put_mem(getCurrentMemoryAllocated(), human);
    s << " current memory allocated\n" << pad;
    s.put_mem(getPeakMemoryAllocated(), human);
    s << " peak memory allocated\n";
  }
  if (flags & EXTRA_STATS) {
    s << pad << stats.reclaimed_nodes << " reclaimed nodes\n";
    s << pad << stats.num_compactions << " compactions\n";
    s << pad << stats.garbage_collections << " garbage collections\n";
  }
  // forest specific
  reportForestStats(s, pad);
  // node storage
  nodeMan->reportStats(s, pad, flags);
  // unique table
  unique->reportStats(s, pad, flags);
}


// ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
// '                                                                '
// '   public virtual methods provided here,  no need to override   '
// '                                                                '
// ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

void MEDDLY::expert_forest
::writeEdges(output &s, const dd_edge* E, int n) const
{
  node_handle* eRaw = new node_handle[n];
  for (int i=0; i<n; i++) {
    eRaw[i] = E[i].getNode();
  }
  node_handle* output2index = markNodesInSubgraph(eRaw, n, false);
  delete[] eRaw;

  // move a pointer to the end of the list, and
  // find the largest node index we're writing
  int maxnode = 0;
  int last;
  for (last = 0; output2index[last]; last++) { 
    maxnode = MAX(maxnode, output2index[last]);
  };
  int num_nodes = last;
  last--;

  // arrange nodes to output, by levels
  for (int k=getNumVariables(); k; ) {
    int i = 0;
    while (i < last) {
      // move last to the left, until we have something not at level k
      for (; i < last; last--) {
        if (getNodeLevel(output2index[last]) != k) break;
      }
      // move i to the right, until we have something at level k
      for (; i < last; i++) {
        if (getNodeLevel(output2index[i]) == k) break;
      }
      if (i < last) {
        SWAP(output2index[i], output2index[last]);
      }
    }

    // next level
    k *= -1;
    if (k>0) k--;
  } // loop over levels

  // build the inverse mapping
  node_handle* index2output = new node_handle[maxnode+1];
  for (int i=0; i<maxnode; i++) index2output[i] = 0;
  for (int i=0; output2index[i]; i++) {
    MEDDLY_CHECK_RANGE(1, output2index[i], maxnode+1);
    index2output[output2index[i]] = i+1;
  }

#ifdef DEBUG_WRITE
  printf("Writing edges:\n");
  showNodeGraph(stdout, eRaw, n);
  printf("Got list of nodes:\n");
  for (int i=0; output2index[i]; i++) {
    if (i) printf(", ");
    printf("%ld", long(output2index[i]));
  }
  printf("\n");
  printf("Got inverse list:\n");
  for (int i=0; i<=maxnode; i++) {
    if (i) printf(", ");
    printf("%ld", long(index2output[i]));
  }
  printf("\n");
#endif

  // Write the nodes
  const char* block = codeChars();
  s << block << " " << num_nodes << "\n";

  unpacked_node* un = unpacked_node::useUnpackedNode();
  for (int i=0; output2index[i]; i++) {
    s << getNodeLevel(output2index[i]) << " ";
    un->initFromNode(this, output2index[i], unpacked_node::AS_STORED);
    un->write(s, index2output);
  }
  unpacked_node::recycle(un);

  // reverse the block
  for (int i=strlen(block); i; ) {
    i--;
    s.put(block[i]);
  }
  s.put('\n');

  // Write the actual edge pointers
  s << "ptrs " << n << "\n";
  for (int i=0; i<n; i++) {
    E[i].write(s, index2output);
  }
  s << "srtp\n";


  // Cleanup
  delete[] index2output;
  free(output2index);
}

void MEDDLY::expert_forest::readEdges(input &s, dd_edge* E, int n)
{
  try {
    const char* block = codeChars();

    s.stripWS();
    s.consumeKeyword(block);
    s.stripWS();
    int num_nodes = s.get_integer();
#ifdef DEBUG_READ
    printf("Reading %d nodes in forest %s\n", num_nodes, codeChars());
#endif

    // start a mapping
    node_handle* map = new node_handle[num_nodes+1];
    for (int i=0; i<=num_nodes; i++) map[i] = 0;

    for (int node_index=1; node_index<=num_nodes; node_index++) {
      // Read this node

      // read the level number
      s.stripWS();
      int k = s.get_integer();
      if (!isValidLevel(k)) {
        throw error(error::INVALID_LEVEL);
      }

#ifdef DEBUG_READ
      printf("Reading %d ", k);
#endif

      // read the node size (sparse/full)
      s.stripWS();
      int rawsize = s.get_integer();
      int n;
      unpacked_node* nb = (rawsize < 0)
        ? unpacked_node::newSparse(this, k, n=-rawsize)
        : unpacked_node::newFull(this, k, n=rawsize);

#ifdef DEBUG_READ
      printf("%d ", rawsize);
#endif

      // read indexes (sparse only)
      if (rawsize<0) {
        for (int i=0; i<n; i++) {
          s.stripWS();
          nb->i_ref(i) = s.get_integer();
        }
#ifdef DEBUG_READ
        printf("indexes ");
#endif
      } else {
#ifdef DEBUG_READ
        printf("no indexes ");
#endif
      }

      // read down
      for (int i=0; i<n; i++) {
        s.stripWS();
        int c = s.get_char();
        s.unget(c);
        if (c>='0' && c<='9') {
          // a regular non-terminal node
#ifdef DEBUG_READ
          printf("n");
#endif
          long down = s.get_integer();
          MEDDLY_DCASSERT(down>=0);
          if (down >= node_index) {
            throw error(error::INVALID_ASSIGNMENT);
          }
          nb->d_ref(i) = linkNode(map[down]);
        } else {
          // must be a terminal node
#ifdef DEBUG_READ
          printf("t");
#endif
          nb->d_ref(i) = readTerminal(s);
        }
      }
#ifdef DEBUG_READ
      printf(" down ");
#endif

      // read edges
      if (nb->hasEdges()) {
        for (int i=0; i<n; i++) {
          s.stripWS();
          readEdgeValue(s, nb->eptr_write(i));
        }
#ifdef DEBUG_READ
        printf("edges ");
#endif
      }

      // any extra header info?  read it
      if (unhashedHeaderBytes()) {
        s.stripWS();
        readUnhashedHeader(s, *nb);
      }
      if (hashedHeaderBytes()) {
        s.stripWS();
        readHashedHeader(s, *nb);
      }

      // ok, done reading; time to reduce it
      map[node_index] = createReducedNode(-1, nb); 

#ifdef DEBUG_READ
      printf("\nNode index %d reduced to ", node_index);
      FILE_output myout(stdout);
      showNode(myout, map[node_index], SHOW_DETAILS | SHOW_INDEX);
      printf("\n");
#endif

  } // for node_index

    // reverse the block
    static char buffer[40];
    int blocklen = strlen(block);
    MEDDLY_DCASSERT(blocklen < 40);
    for (int i=0; i<blocklen; i++) {
      buffer[i] = block[blocklen-i-1];
    }
    buffer[blocklen] = 0;
#ifdef DEBUG_READ
    printf("Done reading, expecting %s keyword\n", buffer);
#endif

    // match the reversed block
    s.stripWS();
    s.consumeKeyword(buffer);
#ifdef DEBUG_READ
    printf("Got %s\n", buffer);
#endif

    // Read the pointers
    s.stripWS();
    s.consumeKeyword("ptrs");
#ifdef DEBUG_READ
    printf("Got ptrs\n");
#endif

    s.stripWS();
    int num_ptrs = s.get_integer();
#ifdef DEBUG_READ
    printf("Reading %d pointers\n", num_ptrs);
#endif
    MEDDLY_DCASSERT(num_ptrs <= n);
    if (num_ptrs > n) {
#ifdef DEBUG_READ
      printf("Error at %s:%d, E[] is of size %d, needs to be at least %d\n",
        __FILE__, __LINE__, n, num_ptrs);
      fflush(stdout);
#endif
      throw error(error::INVALID_ASSIGNMENT);
    }
    for (int i=0; i<num_ptrs; i++) {
      E[i].read(this, s, map);
    }

    s.stripWS();
    s.consumeKeyword("srtp");

    // unlink map pointers
    for (int i=0; i<=num_nodes; i++) unlinkNode(map[i]);
    delete[] map;

#ifdef DEVELOPMENT_CODE
    validateIncounts(true);
#endif
  } // try
  catch (error& e) {
#ifdef DEBUG_READ
    printf("Read failed (error: %s)\n", e.getName());
    printf("Failed, next few characters of file:\n");
    for (int i=0; i<10; i++) {
      int c = s.get_char();
      if (EOF == c) {
        printf("EOF");
        break;
      }
      printf("%c (%d) ", c, c);
    }
    fputc('\n', stdout);
#endif
    throw e;
  }
}

void MEDDLY::expert_forest::garbageCollect()
{
  if (performing_gc) return;
  performing_gc = true;
  stats.garbage_collections++;

#ifdef DEBUG_GC
  printf("Garbage collection in progress... \n");
  fflush(stdout);
#endif

  if (isPessimistic()) {
#ifdef DEBUG_GC
    printf("Zombie nodes: %ld\n", stats.zombie_nodes);
#endif
    // remove the stale nodes entries from caches
    removeStaleComputeTableEntries();
#ifdef DEBUG_GC
    printf("Zombie nodes: %ld\n", stats.zombie_nodes);
#endif
#ifdef DEVELOPMENT_CODE
    if (stats.zombie_nodes != 0) {
      FILE_output mystderr(stderr);
      showInfo(mystderr, 1);
      showComputeTable(mystderr, 6);
    }
    MEDDLY_DCASSERT(stats.zombie_nodes == 0);
#endif
  } else {
    // remove the stale nodes entries from caches
    removeStaleComputeTableEntries();
  }

  if (deflt.compactAfterGC) {
#ifdef DEBUG_GC
    printf("Compacting levels...\n");
    fflush(stdout);
#endif

    compactMemory(); 

#ifdef DEBUG_GC
    printf("  done compacting.\n");
    fflush(stdout);
#endif
  }

  performing_gc = false;
}

void MEDDLY::expert_forest::compactMemory()
{
  nodeMan->collectGarbage(true);
}

void MEDDLY::expert_forest::showInfo(output &s, int verb)
{
  // Show forest with appropriate level of detail
  if (1==verb)  dump(s, SHOW_DETAILS);
  else          dumpInternal(s); 
  s << "DD stats:\n";
  reportStats(s, "    ", ~0);
  // reportMemoryUsage(s, "    ", verb);
  s << "Unique table stats:\n\t";
  s.put("Current size:", -24);
  s << long(unique->getSize()) << "\n\t";
  s.put("Current entries:", -24);
  s << long(unique->getNumEntries()) << "\n";
}


// ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
// '                                                                '
// '    virtual methods to be overridden by some derived classes    '
// '                                                                '
// ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

bool MEDDLY::expert_forest
::areEdgeValuesEqual(const void* eva, const void* evb) const
{
  throw error(error::TYPE_MISMATCH);
}

MEDDLY::enumerator::iterator* MEDDLY::expert_forest::makeFixedRowIter() const
{
  throw error(error::TYPE_MISMATCH);
}

MEDDLY::enumerator::iterator*
MEDDLY::expert_forest::makeFixedColumnIter() const
{
  throw error(error::TYPE_MISMATCH);
}

const char* MEDDLY::expert_forest::codeChars() const
{
  return "unknown dd";
}

void MEDDLY::expert_forest::normalize(unpacked_node &nb, int& ev) const
{
  throw error(error::TYPE_MISMATCH);
}

void MEDDLY::expert_forest::normalize(unpacked_node &nb, float& ev) const
{
  throw error(error::TYPE_MISMATCH);
}

void MEDDLY::expert_forest::reportForestStats(output &s, const char* pad) const
{
  // default - do nothing
}

void MEDDLY::expert_forest::showTerminal(output &s, node_handle tnode) const
{
  throw error(error::NOT_IMPLEMENTED);
}

void MEDDLY::expert_forest::writeTerminal(output &s, node_handle tnode) const
{
  throw error(error::NOT_IMPLEMENTED);
}

MEDDLY::node_handle MEDDLY::expert_forest::readTerminal(input &s)
{
  throw error(error::NOT_IMPLEMENTED);
}

void MEDDLY::expert_forest::showEdgeValue(output &s, const void* edge) const
{
  throw error(error::TYPE_MISMATCH);
}

void MEDDLY::expert_forest::writeEdgeValue(output &s, const void* edge) const
{
  throw error(error::TYPE_MISMATCH);
}

void MEDDLY::expert_forest::readEdgeValue(input &s, void* edge)
{
  throw error(error::TYPE_MISMATCH);
}

void MEDDLY::expert_forest::showHashedHeader(output &s, const void* hh) const
{
  throw error(error::TYPE_MISMATCH);
}

void MEDDLY::expert_forest::writeHashedHeader(output &s, const void* hh) const
{
  throw error(error::TYPE_MISMATCH);
}

void MEDDLY::expert_forest::readHashedHeader(input &s, unpacked_node &nb) const
{
  throw error(error::TYPE_MISMATCH);
}

void MEDDLY::expert_forest::showUnhashedHeader(output &s, const void* uh) const
{
  throw error(error::TYPE_MISMATCH);
}

void MEDDLY::expert_forest::writeUnhashedHeader(output &s, const void* uh) const
{
  throw error(error::TYPE_MISMATCH);
}

void MEDDLY::expert_forest::readUnhashedHeader(input &s, unpacked_node &nb) const
{
  throw error(error::TYPE_MISMATCH);
}

void MEDDLY::expert_forest::reorderVariables(const int* level2var)
{
  removeAllComputeTableEntries();

  // Create a temporary variable order
  // Support in-place update and avoid interfering other forests
  var_order = std::make_shared<variable_order>(*var_order);

  auto reordering = reordering_factory::create(getPolicies().reorder);
  reordering->reorderVariables(this, level2var);

  var_order = useDomain()->makeVariableOrder(*var_order);
}

// ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
// '                                                                '
// '                                                                '
// '                        private  methods                        '
// '                                                                '
// '                                                                '
// ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

void MEDDLY::expert_forest::handleNewOrphanNode(node_handle p)
{
  MEDDLY_DCASSERT(!isPessimistic() || !isZombieNode(p));
  MEDDLY_DCASSERT(isActiveNode(p));
  MEDDLY_DCASSERT(!isTerminalNode(p));
  MEDDLY_DCASSERT(getNodeInCount(p) == 0);

  // insted of orphan_nodes++ here; do it only when the orphan is not going
  // to get deleted or converted into a zombie

  // Two possible scenarios:
  // (1) a reduced node, or
  // (2) a temporary node ready to be deleted.
  // MEDDLY_DCASSERT(isReducedNode(p) || getNodeCacheCount(p) == 0);

  if (getNodeCacheCount(p) == 0) {
    // delete node
    // this should take care of the temporary nodes also
    deleteNode(p);
  }
  else if (isPessimistic()) {
    // zombify node
    zombifyNode(p);
  }
  else {
    stats.orphan_nodes++;
  }

}

void MEDDLY::expert_forest::deleteNode(node_handle p)
{
#ifdef TRACK_DELETIONS
  for (int i=0; i<delete_depth; i++) printf(" ");
  printf("Deleting node ");
  showNode(stdout, p, SHOW_INDEX | SHOW_DETAILS);
  printf("\n");
  fflush(stdout);
#endif
#ifdef VALIDATE_INCOUNTS_ON_DELETE
  delete_depth++;
#endif

  MEDDLY_DCASSERT(isValidNonterminalIndex(p));
  MEDDLY_DCASSERT(getNodeInCount(p) == 0);
  MEDDLY_DCASSERT(isActiveNode(p));

  unsigned h = hashNode(p);
#ifdef DEVELOPMENT_CODE
  unpacked_node* key = unpacked_node::newFromNode(this, p, false);
  key->computeHash();
  if (unique->find(*key, getVarByLevel(key->getLevel())) != p) {
    fprintf(stderr, "Error in deleteNode\nFind: %ld\np: %ld\n",
      static_cast<long>(unique->find(*key, getVarByLevel(key->getLevel()))), static_cast<long>(p));
    FILE_output myout(stdout);
    dumpInternal(myout);
    MEDDLY_DCASSERT(false);
  }
  node_handle x = unique->remove(h, p);
  MEDDLY_DCASSERT(p == x);
  unpacked_node::recycle(key);
#else
  unique->remove(h, p);
#endif

#ifdef TRACK_DELETIONS
  // start at one, because we have incremented the depth
  for (int i=1; i<delete_depth; i++) printf(" "); 
  printf("%s: p = %d, unique->remove(p) = %d\n", __func__, p, x);
  fflush(stdout);
#endif

  MEDDLY_DCASSERT(address[p].cache_count == 0);

  // unlink children and recycle node memory
  nodeMan->unlinkDownAndRecycle(getNodeAddress(p));

  // recycle the index
  //
  stats.decActive(1);
  if (theLogger && theLogger->recordingNodeCounts()) {
    theLogger->addToActiveNodeCount(this, getNodeLevel(p), -1);
  }
  recycleNodeHandle(p);

  // if (nodeMan.compactLevel) nodeMan.compact(false);

#ifdef VALIDATE_INCOUNTS_ON_DELETE
  delete_depth--;
  if (0==delete_depth) {
    validateIncounts(false);
  }
#endif

}

void MEDDLY::expert_forest::zombifyNode(node_handle p)
{
#ifdef TRACK_DELETIONS
  for (int i=0; i<delete_depth; i++) printf(" ");
  printf("Zombifying node ");
  showNode(stdout, p, SHOW_INDEX | SHOW_DETAILS);
  printf("\n");
  fflush(stdout);
#endif
#ifdef VALIDATE_INCOUNTS_ON_DELETE
  delete_depth++;
#endif

  MEDDLY_DCASSERT(isActiveNode(p));
  MEDDLY_DCASSERT(!isTerminalNode(p));
  MEDDLY_DCASSERT(getNodeCacheCount(p) > 0);  // otherwise this node should be deleted
  MEDDLY_DCASSERT(getNodeInCount(p) == 0);
  MEDDLY_DCASSERT(address[p].cache_count > 0);

  stats.zombie_nodes++;
  stats.decActive(1);
  if (theLogger && theLogger->recordingNodeCounts()) {
    theLogger->addToActiveNodeCount(this, getNodeLevel(p), -1);
  }

  unsigned h = hashNode(p);
#ifdef DEVELOPMENT_CODE 
  unpacked_node* key = unpacked_node::newFromNode(this, p, false);
  key->computeHash();
  MEDDLY_DCASSERT(key->hash() == h);
  if (unique->find(*key, getVarByLevel(key->getLevel())) != p) {
    fprintf(stderr, "Fail: can't find reduced node %ld; got %ld\n",
      static_cast<long>(p), static_cast<long>(unique->find(*key, getVarByLevel(key->getLevel()))));
    FILE_output myerr(stderr);
    dumpInternal(myerr);
    MEDDLY_DCASSERT(false);
  }
  node_handle x = unique->remove(h, p);
  MEDDLY_DCASSERT(x==p);
  unpacked_node::recycle(key);
#else
  unique->remove(h, p);
#endif

  // node_address addr = getNode(p).offset;

  // unlink children and recycle node memory
  nodeMan->unlinkDownAndRecycle(getNodeAddress(p));

  // mark node as zombie
  address[p].makeZombie();

#ifdef VALIDATE_INCOUNTS_ON_DELETE
  delete_depth--;
  if (0==delete_depth) {
    validateIncounts(false);
  }
#endif
}


#ifdef DEBUG_HANDLE_FREELIST
void print_sequence(long a)
{
  static bool printed;
  static long first = 0;
  static long last = -1;
  if (a<=0) {
    if (first>last) return;
    if (printed) printf(", "); 
    printed = false;
    if (first < last) {
      if (first+1<last) {
        printf("%ld ... %ld", first, last);
      } else {
        printf("%ld, %ld", first, last);
      }
    } else {
      printf("%ld", first);
    }
    first = 0;
    last = -1;
    return;
  }
  // a > 0
  if (0==first) {
    first = last = a;
    return;
  }
  if (last+1 == a) {
    last++;
    return;
  }
  // break in the sequence, we need to print
  if (printed) printf(", "); else printed = true;
  if (first < last) {
    if (first+1<last) {
      printf("%ld ... %ld", first, last);
    } else {
      printf("%ld, %ld", first, last);
    }
  } else {
    printf("%ld", first);
  }
  first = last = a;
}

inline void dump_handle_info(const MEDDLY::node_header* A, long size)
{
  printf("Used handles:  ");
  print_sequence(0);
  for (long i=1; i<size; i++) if (!A[i].isDeleted()) {
    print_sequence(i);
  }
  print_sequence(0);
  printf("\nFree handles:  ");
  for (long i=1; i<size; i++) if (A[i].isDeleted()) {
    print_sequence(i);
  }
  print_sequence(0);
  printf("\n");
}
#endif

MEDDLY::node_handle MEDDLY::expert_forest::getFreeNodeHandle() 
{
  MEDDLY_DCASSERT(address);
  stats.incMemUsed(sizeof(node_header));
  node_handle found = 0;
  for (int i=a_lowest_index; i<8; i++) {
    // try the various lists
    while (a_unused[i] > a_last) {
      a_unused[i] = address[a_unused[i]].getNextDeleted();
    }
    if (a_unused[i]) {  // get a recycled one from list i
      found = a_unused[i];
      a_unused[i] = address[a_unused[i]].getNextDeleted();
      break;
    } else {
      if (i == a_lowest_index) a_lowest_index++;
    }
  }
  if (found) {  
    MEDDLY_DCASSERT(address[found].isDeleted());
#ifdef DEBUG_HANDLE_FREELIST
    address[found].setNotDeleted();
    dump_handle_info(address, a_last+1);
#endif
    return found;
  }
  a_last++;
  if (a_last >= a_size) {
    expandHandleList();
  }
  MEDDLY_DCASSERT(a_last < a_size);
#ifdef DEBUG_HANDLE_FREELIST
  address[a_last].setNotDeleted();
  dump_handle_info(address, a_last+1);
#endif
  return a_last;
}


void MEDDLY::expert_forest::recycleNodeHandle(node_handle p) 
{
  MEDDLY_DCASSERT(address);
  MEDDLY_DCASSERT(p>0);
  MEDDLY_DCASSERT(0==address[p].cache_count);
  stats.decMemUsed(sizeof(node_header));
  address[p].setDeleted();

  // Determine which list to add this into
  int i = bytesRequiredForDown(p) -1;
  address[p].setNextDeleted(a_unused[i]);
  a_unused[i] = p;
  a_lowest_index = MIN(a_lowest_index, (char)i);

  // if this was the last node, collapse nodes into the
  // "not yet allocated" pile.  But, we don't remove them
  // from the free list(s); we simply discard any too-large
  // ones when we pull from the free list(s).
  if (p == a_last) {
    while (a_last && address[a_last].isDeleted()) {
      a_last--;
    }

    if (a_last < a_next_shrink) shrinkHandleList();
  }
#ifdef DEBUG_HANDLE_FREELIST
  dump_handle_info(address, a_last+1);
#endif
}


MEDDLY::node_handle MEDDLY::expert_forest
::createReducedHelper(int in, const unpacked_node &nb)
{
#ifdef DEVELOPMENT_CODE
  validateDownPointers(nb);
#endif

  // get sparse, truncated full sizes and check
  // for redundant / identity reductions.
  int nnz;
  if (nb.isSparse()) {
    // Reductions for sparse nodes
    nnz = nb.getNNZs();
#ifdef DEVELOPMENT_CODE
    for (int z=0; z<nnz; z++) {
      MEDDLY_DCASSERT(nb.d(z)!=getTransparentNode());
    } // for z
#endif

    // Check for identity nodes
    if (1==nnz && in==nb.i(0)) {
      if (isIdentityEdge(nb, 0)) {
#ifdef DEBUG_CREATE_REDUCED
        printf("Identity node ");
        // showNode(stdout, nb.d(0), SHOW_INDEX);
        printf("\n");
#endif
        return nb.d(0);
      }
    }

    // Check for redundant nodes
    if (nnz == getLevelSize(nb.getLevel())) {
      if (isRedundant(nb)) {
        // unlink downward pointers, except the one we're returning.
        for (int i = 1; i<nnz; i++)  unlinkNode(nb.d(i));  
#ifdef DEBUG_CREATE_REDUCED
        printf("Redundant node 1");
        // showNode(stdout, nb.d(0),SHOW_INDEX);
        printf("\n");
#endif
        return nb.d(0);
      }
    }

  } else {
    // Reductions for full nodes
    MEDDLY_DCASSERT(nb.getSize() <= getLevelSize(nb.getLevel()));
    nnz = 0;
    for (int i=0; i<nb.getSize(); i++) {
      if (nb.d(i)!=getTransparentNode()) nnz++;
    } // for i

    // Check for identity nodes
    if (1==nnz) {
      if (isIdentityEdge(nb, in)) {
#ifdef DEBUG_CREATE_REDUCED
        printf("Identity node ");
        // showNode(stdout, nb.d(0), SHOW_INDEX);
        printf("\n");
#endif
        return nb.d(in);
      }
    }

    // Check for redundant nodes
    if (nnz == getLevelSize(nb.getLevel())) {
      if (isRedundant(nb)) {
        // unlink downward pointers, except the one we're returning.
        for (int i = 1; i<nb.getSize(); i++)  unlinkNode(nb.d(i));
#ifdef DEBUG_CREATE_REDUCED
        printf("Redundant node 2");
        // showNode(stdout, nb.d(0),SHOW_INDEX);
        printf("\n");
#endif
        return nb.d(0);
      }
    }
  }

  // Is this a transparent node?
  if (0==nnz) {
    // no need to unlink
    return getTransparentNode();
  }

  // check for duplicates in unique table
  node_handle q = unique->find(nb, getVarByLevel(nb.getLevel()));
  if (q) {
    // unlink all downward pointers
    int rawsize = nb.isSparse() ? nb.getNNZs() : nb.getSize();
    for (int i = 0; i<rawsize; i++)  unlinkNode(nb.d(i));
    return linkNode(q);
  }

  // 
  // Not eliminated by reduction rule.
  // Not a duplicate.
  //
  // We need to create a new node for this.

  // NOW is the best time to run the garbage collector, if necessary.
#ifndef GC_OFF
  if (isTimeToGc()) garbageCollect();
#endif

  // Grab a new node
  node_handle p = getFreeNodeHandle();
  address[p].level = nb.getLevel();
  MEDDLY_DCASSERT(0 == address[p].cache_count);

  stats.incActive(1);
  if (theLogger && theLogger->recordingNodeCounts()) {
    theLogger->addToActiveNodeCount(this, address[p].level, 1);
  }

  // All of the work is in nodeMan now :^)
  address[p].offset = nodeMan->makeNode(p, nb, getNodeStorage());

  // add to UT
  unique->add(nb.hash(), p);

#ifdef DEVELOPMENT_CODE
  unpacked_node* key = unpacked_node::newFromNode(this, p, false);
  key->computeHash();
  MEDDLY_DCASSERT(key->hash() == nb.hash());
  node_handle f = unique->find(*key, getVarByLevel(key->getLevel()));
  MEDDLY_DCASSERT(f == p);
  unpacked_node::recycle(key);
#endif
#ifdef DEBUG_CREATE_REDUCED
  printf("Created node ");
  showNode(stdout, p, SHOW_DETAILS | SHOW_INDEX);
  printf("\n");
#endif

  return p;
}

void MEDDLY::expert_forest::swapNodes(node_handle p, node_handle q)
{
  unique->remove(hashNode(p), p);
  unique->remove(hashNode(q), q);

  int pCount = getNodeInCount(p);
  int qCount = getNodeInCount(q);

  // Swap
  node_header temp = address[p];
  address[p] = address[q];
  address[q] = temp;

  // Do not change inCount
  nodeMan->setCountOf(address[p].offset, pCount);
  nodeMan->setCountOf(address[q].offset, qCount);

  unique->add(hashNode(p), p);
  unique->add(hashNode(q), q);
}

MEDDLY::node_handle MEDDLY::expert_forest::modifyReducedNodeInPlace(unpacked_node* un, node_handle p)
{
  int count = getNodeInCount(p);

  unique->remove(hashNode(p), p);
  nodeMan->unlinkDownAndRecycle(address[p].offset);

  un->computeHash();

  address[p].level = un->getLevel();
  address[p].offset = nodeMan->makeNode(p, *un, getNodeStorage());

  nodeMan->setCountOf(address[p].offset, count);

  unique->add(un->hash(), p);

#ifdef DEVELOPMENT_CODE
  unpacked_node* key = unpacked_node::newFromNode(this, p, false);
  key->computeHash();
  MEDDLY_DCASSERT(key->hash() == un->hash());
  node_handle f = unique->find(*key, getVarByLevel(key->getLevel()));
  MEDDLY_DCASSERT(f == p);
  unpacked_node::recycle(key);
#endif
#ifdef DEBUG_CREATE_REDUCED
  printf("Created node ");
  showNode(stdout, p, SHOW_DETAILS | SHOW_INDEX);
  printf("\n");
#endif

  unpacked_node::recycle(un);
  return p;
}

void MEDDLY::expert_forest::validateDownPointers(const unpacked_node &nb) const
{
  switch (getReductionRule()) {
    case policies::IDENTITY_REDUCED:
    case policies::FULLY_REDUCED:
      if (nb.isSparse()) {
        for (int z=0; z<nb.getNNZs(); z++) {
          MEDDLY_DCASSERT(isLevelAbove(nb.getLevel(), getNodeLevel(nb.d(z))));
        } 
      } else {
        for (int i=0; i<nb.getSize(); i++) {
          MEDDLY_DCASSERT(isLevelAbove(nb.getLevel(), getNodeLevel(nb.d(i))));
        }
      }
      break;

    case policies::QUASI_REDUCED:
#ifdef DEVELOPMENT_CODE
      int nextLevel;
      if (isForRelations()) 
        nextLevel = (nb.getLevel()<0) ? -(nb.getLevel()+1) : -nb.getLevel();
      else
        nextLevel = nb.getLevel()-1;
#endif
      if (nb.isSparse()) {
        for (int z=0; z<nb.getNNZs(); z++) {
          MEDDLY_DCASSERT(getNodeLevel(nb.d(z)) == nextLevel);
        } 
      } else {
        node_handle tv=getTransparentNode();
        for (int i=0; i<nb.getSize(); i++) {
          if (nb.d(i)==tv) continue;
          MEDDLY_DCASSERT(getNodeLevel(nb.d(i)) == nextLevel);
        }
      }
      break;

    default:
      throw error(error::NOT_IMPLEMENTED);
  }

}


void MEDDLY::expert_forest::expandHandleList()
{
  // increase size by 50%
  int delta = a_size / 2;
  MEDDLY_DCASSERT(delta>=0);
  node_header* new_address = (node_header*) 
    realloc(address, (a_size+delta) * sizeof(node_header));
  if (0==new_address) {
    /*
    fprintf(stderr, "Error in allocating array of size %lu at %s, line %d\n",
        (a_size+delta) * sizeof(node_header), __FILE__, __LINE__);
    */
    throw error(error::INSUFFICIENT_MEMORY);
  }
  address = new_address;
  stats.incMemAlloc(delta * sizeof(node_header));
  memset(address + a_size, 0, delta * sizeof(node_header));
  a_size += delta;
  a_next_shrink = a_size / 2;
#ifdef DEBUG_ADDRESS_RESIZE
  printf("Enlarged address array, new size %d\n", a_size);
#endif
}

void MEDDLY::expert_forest::shrinkHandleList()
{
  // Determine new size
  int new_size = a_min_size;
  while (a_last >= new_size) new_size += new_size/2;
  int delta = a_size - new_size;
  if (0==delta) {
    a_next_shrink = 0;
    return;
  }

  // clean out free lists, because we're about
  // to trash memory beyond new_size.
  for (int i=0; i<8; i++) {
    //
    // clean list i
    //
    node_handle prev = 0;
    node_handle curr;
    for (curr = a_unused[i]; curr; curr=address[curr].getNextDeleted()) 
    {
      if (curr > a_last) continue;  // don't add to the list
      if (prev) {
        address[prev].setNextDeleted(curr);
      } else {
        a_unused[i] = curr;
      }
    } 
    if (prev) {
      address[prev].setNextDeleted(0);
    } else {
      a_unused[i] = 0;
    }
  } // for i

  // shrink the array
  MEDDLY_DCASSERT(delta>=0);
  MEDDLY_DCASSERT(a_size-delta>=a_min_size);
  node_header* new_address = (node_header*) 
    realloc(address, new_size * sizeof(node_header));
  if (0==new_address) {
    /*
    fprintf(stderr, "Error in allocating array of size %lu at %s, line %d\n",
        new_size*sizeof(node_header), __FILE__, __LINE__);
    */
    throw error(error::INSUFFICIENT_MEMORY);
  }
  address = new_address;
  stats.decMemAlloc(delta * sizeof(node_header));
  a_size -= delta;
  a_next_shrink = a_size / 2;
  MEDDLY_DCASSERT(a_last < a_size);
#ifdef DEBUG_ADDRESS_RESIZE
  printf("Shrank address array, new size %d\n", a_size);
#endif
}

// ******************************************************************
// *                                                                *
// *               expert_forest::nodecounter methods               *
// *                                                                *
// ******************************************************************

MEDDLY::expert_forest::nodecounter::nodecounter(expert_forest *p, int* c)
 : edge_visitor()
{
  parent = p;
  counts = c;
}

MEDDLY::expert_forest::nodecounter::~nodecounter()
{
  // DO NOT delete counts.
}

void MEDDLY::expert_forest::nodecounter::visit(dd_edge &e)
{
  int n = e.getNode();
  if (parent->isTerminalNode(n)) return;
  MEDDLY_DCASSERT(n>0);
  MEDDLY_DCASSERT(n<=parent->getLastNode());
  counts[n]++;
}

