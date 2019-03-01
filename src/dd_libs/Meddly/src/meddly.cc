
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

/*
    Implementation of the "global" functions given in
    meddly.h and meddly_expert.h.

*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "defines.h"
#include "revision.h"
// #include "compute_table.h"
#include "operations/init_builtin.h"
#include "forests/init_forests.h"
#include "storage/init_storage.h"

// #define STATS_ON_DESTROY

namespace MEDDLY {
  // "global" variables

  initializer_list* meddlyInitializers;


  bool libraryRunning = 0;

  // cache of operations
  operation** op_cache = 0;
  // size of cache
  int op_cache_size = 0;

  // Monolithic compute table, if used
  compute_table* operation::Monolithic_CT = 0;

  //
  // List of operations
  //
  operation** operation::op_list = 0;
  int* operation::op_holes = 0;
  int operation::list_size = 0;
  int operation::list_alloc = 0;
  int operation::free_list = -1;

  //
  // List of all domains
  //
  domain** domain::dom_list = 0;
  int* domain::dom_free = 0;
  int domain::dom_list_size = 0;
  int domain::free_list = -1;

  //
  // List of free unpacked nodes
  unpacked_node* unpacked_node::freeList = 0;

  // helper functions
  void purgeMarkedOperations();
  void destroyOpInternal(operation* op);

};

//----------------------------------------------------------------------
// front end - unary operations
//----------------------------------------------------------------------

MEDDLY::unary_operation* MEDDLY::getOperation(const unary_opname* code, 
  expert_forest* arg, expert_forest* res)
{
  if (!libraryRunning) 
    throw error(error::UNINITIALIZED);
  if (code->getIndex()<0 || code->getIndex()>=op_cache_size)
    throw error(error::INVALID_OPERATION);

  operation* curr;
  operation* prev = 0;
  for (curr=op_cache[code->getIndex()]; curr; curr=curr->getNext()) {
    if (((unary_operation*)curr)->matches(arg, res)) {
      // move to front of list...
      if (prev) {
        prev->setNext(curr->getNext());
        curr->setNext(op_cache[code->getIndex()]);
        op_cache[code->getIndex()] = curr;
      }
      // ... and return
      return (unary_operation*) curr;
    }
    prev = curr;
  } // for

  // none present, build a new one...
  curr = code->buildOperation(arg, res);
  // ...move it to the front...
  curr->setNext(op_cache[code->getIndex()]);
  op_cache[code->getIndex()] = curr;
  // ...and return
  return (unary_operation*) curr;
}

MEDDLY::unary_operation* MEDDLY::getOperation(const unary_opname* code, 
  expert_forest* arg, opnd_type res)
{
  if (!libraryRunning) 
    throw error(error::UNINITIALIZED);
  if (code->getIndex()<0 || code->getIndex()>=op_cache_size)
    throw error(error::INVALID_OPERATION);

  operation* curr;
  operation* prev = 0;
  for (curr=op_cache[code->getIndex()]; curr; curr=curr->getNext()) {
    if (((unary_operation*)curr)->matches(arg, res)) {
      // move to front of list...
      if (prev) {
        prev->setNext(curr->getNext());
        curr->setNext(op_cache[code->getIndex()]);
        op_cache[code->getIndex()] = curr;
      }
      // ... and return
      return (unary_operation*) curr;
    }
    prev = curr;
  } // for

  // none present, build a new one...
  curr = code->buildOperation(arg, res);
  // ...move it to the front...
  curr->setNext(op_cache[code->getIndex()]);
  op_cache[code->getIndex()] = curr;
  // ...and return
  return (unary_operation*) curr;
}

MEDDLY::binary_operation* MEDDLY::getOperation(const binary_opname* code, 
  expert_forest* arg1, expert_forest* arg2, expert_forest* res)
{
  if (!libraryRunning) 
    throw error(error::UNINITIALIZED);
  if (code->getIndex()<0 || code->getIndex()>=op_cache_size)
    throw error(error::INVALID_OPERATION);

  operation* curr;
  operation* prev = 0;
  for (curr=op_cache[code->getIndex()]; curr; curr=curr->getNext()) {
    if (((binary_operation*)curr)->matches(arg1, arg2, res)) {
      // move to front of list...
      if (prev) {
        prev->setNext(curr->getNext());
        curr->setNext(op_cache[code->getIndex()]);
        op_cache[code->getIndex()] = curr;
      }
      // ... and return
      return (binary_operation*) curr;
    }
    prev = curr;
  } // for

  // none present, build a new one...
  curr = code->buildOperation(arg1, arg2, res);
  // ...move it to the front...
  curr->setNext(op_cache[code->getIndex()]);
  op_cache[code->getIndex()] = curr;
  // ...and return
  return (binary_operation*) curr;
}

void MEDDLY::removeOperationFromCache(operation* op)
{
  if (0==op || 0==op_cache) return;
  if (!libraryRunning) 
    throw error(error::UNINITIALIZED);
  const opname* code = op->getOpName();

  operation* curr;
  operation* prev = 0;
  for (curr=op_cache[code->getIndex()]; curr; curr=curr->getNext()) {
    if (curr == op) break;
    prev = curr;
  } // for
  if (0==curr) return;  // not found
  // remove curr
  if (prev) {
    prev->setNext(curr->getNext());
  } else {
    op_cache[code->getIndex()] = curr->getNext();
  }
  curr->setNext(0);
}

void MEDDLY::apply(const unary_opname* code, const dd_edge &a, dd_edge &c)
{
  if (!libraryRunning) 
    throw error(error::UNINITIALIZED);
  if (0==code)  
    throw error(error::UNKNOWN_OPERATION);
  unary_operation* op = getOperation(code, a, c);
  op->compute(a, c);
}

void MEDDLY::apply(const unary_opname* code, const dd_edge &a, long &c)
{
  if (!libraryRunning) 
    throw error(error::UNINITIALIZED);
  if (0==code)
    throw error(error::UNKNOWN_OPERATION);
  unary_operation* op = getOperation(code, a, INTEGER);
  op->compute(a, c);
}

void MEDDLY::apply(const unary_opname* code, const dd_edge &a, double &c)
{
  if (!libraryRunning) 
    throw error(error::UNINITIALIZED);
  if (0==code)
    throw error(error::UNKNOWN_OPERATION);
  unary_operation* op = getOperation(code, a, REAL);
  op->compute(a, c);
}

void MEDDLY::apply(const unary_opname* code, const dd_edge &a, opnd_type cr,
  ct_object &c)
{
  if (!libraryRunning) 
    throw error(error::UNINITIALIZED);
  if (0==code)
    throw error(error::UNKNOWN_OPERATION);
  unary_operation* op = getOperation(code, a, cr);
  op->compute(a, c);
}

void MEDDLY::apply(const binary_opname* code, const dd_edge &a, 
  const dd_edge &b, dd_edge &c)
{
  if (!libraryRunning) 
    throw error(error::UNINITIALIZED);
  if (0==code)
    throw error(error::UNKNOWN_OPERATION);
  binary_operation* op = getOperation(code, a, b, c);
  op->compute(a, b, c);
}

//----------------------------------------------------------------------
// front end - create and destroy objects
//----------------------------------------------------------------------

MEDDLY::variable* MEDDLY::createVariable(int bound, char* name)
{
  if (!libraryRunning) throw error(error::UNINITIALIZED);
  return new expert_variable(bound, name);
}

MEDDLY::domain* MEDDLY::createDomain(variable** vars, int N)
{
  if (!libraryRunning) throw error(error::UNINITIALIZED);
  return new expert_domain(vars, N);
}

MEDDLY::domain* MEDDLY::createDomainBottomUp(const int* bounds, int N)
{
  if (!libraryRunning) throw error(error::UNINITIALIZED);
  domain* d = new expert_domain(0, 0);
  d->createVariablesBottomUp(bounds, N);
  return d;
}

void MEDDLY::destroyDomain(MEDDLY::domain* &d)
{
  if (0==d) return;
  if (!libraryRunning) throw error(error::UNINITIALIZED);
  d->markForDeletion();
  purgeMarkedOperations();
  delete d;
  d = 0;
}

void MEDDLY::destroyForest(MEDDLY::forest* &f)
{
  if (0==f) return;
  if (!libraryRunning) throw error(error::UNINITIALIZED);
  f->markForDeletion();
  purgeMarkedOperations();
  delete f;
  f = 0;
}

void MEDDLY::purgeMarkedOperations()
{
  operation::removeStalesFromMonolithic();
  for (int i=0; i<operation::getOpListSize(); i++) {
    operation* op = operation::getOpWithIndex(i);
    if (0==op) continue;
    if (op->isMarkedForDeletion()) {
      destroyOpInternal(op);
    }
  }
}

inline void MEDDLY::destroyOpInternal(MEDDLY::operation* op)
{
  if (0==op) return;
  if (!libraryRunning) throw error(error::UNINITIALIZED);
  removeOperationFromCache(op);
  if (!op->isMarkedForDeletion()) {
    op->markForDeletion();
    operation::removeStalesFromMonolithic();
  }
  delete op;
}

void MEDDLY::destroyOperation(MEDDLY::unary_operation* &op)
{
  destroyOpInternal(op);
  op = 0;
}

void MEDDLY::destroyOperation(MEDDLY::binary_operation* &op)
{
  destroyOpInternal(op);
  op = 0;
}

void MEDDLY::destroyOperation(MEDDLY::specialized_operation* &op)
{
  destroyOpInternal(op);
  op = 0;
}




//----------------------------------------------------------------------
// front end - initialize and cleanup of library
//----------------------------------------------------------------------

MEDDLY::initializer_list* MEDDLY::defaultInitializerList(initializer_list* prev)
{
  prev = new ct_initializer(prev);
  prev = new storage_initializer(prev);
  prev = new builtin_initializer(prev);
  prev = new forest_initializer(prev);

  return prev;
}

// void MEDDLY::initialize(const settings &s, initializer_list* L)
void MEDDLY::initialize(initializer_list* L)
{
  if (libraryRunning) throw error(error::ALREADY_INITIALIZED);

  opname::next_index = 0;

  if (L) L->setupAll();
  meddlyInitializers = L;

  // set up operation cache
  op_cache_size = opname::next_index;
  op_cache = new operation*[op_cache_size];
  for (int i=0; i<op_cache_size; i++) {
    op_cache[i] = 0;
  }

  libraryRunning = 1;
}

void MEDDLY::initialize()
{
  initialize( defaultInitializerList(0) );
}

void MEDDLY::cleanup()
{
  if (!libraryRunning) throw error(error::UNINITIALIZED);

#ifdef STATS_ON_DESTROY
  if (operation::Monolithic_CT) {
    fprintf(stderr, "Compute table (before destroy):\n");
    operation::Monolithic_CT->show(stderr, false);
  }

#endif

  domain::markDomList();

  operation::destroyAllOps();

  domain::deleteDomList();

  // clean up operation cache 
  delete[] op_cache;
  op_cache = 0;

  // clean up recycled unpacked nodes
  unpacked_node::freeRecycled();

  if (meddlyInitializers) {
    meddlyInitializers->cleanupAll();
    delete meddlyInitializers;
    meddlyInitializers = 0;
  }

  libraryRunning = 0;
}

//----------------------------------------------------------------------
// front end - library info
//----------------------------------------------------------------------


const char* MEDDLY::getLibraryInfo(int what)
{
  static char* title = 0;
  switch (what) {
    case 0:
      if (!title) {
        title = new char[80];
        if (REVISION_NUMBER) {
          snprintf(title, 80, 
#ifdef DEVELOPMENT_CODE
            "%s version %s.%d.dev", 
#else
            "%s version %s.%d", 
#endif
            PACKAGE_NAME, VERSION, REVISION_NUMBER
          );
        } else {
          snprintf(title, 80, 
#ifdef DEVELOPMENT_CODE
            "%s version %s.dev", 
#else
            "%s version %s", 
#endif
            PACKAGE_NAME, VERSION
          );
        }
      }
      return title;

    case 1:
      return "Copyright (C) 2009, Iowa State University Research Foundation, Inc.";

    case 2:
      return "Released under the GNU Lesser General Public License, version 3";
 
    case 3:
      return PACKAGE_URL;

    case 4:
      return "Data Structures and operations available:\n\
(1) MDDs: Union, Intersection, Difference.\n\
(2) Matrix Diagrams (MXDs): Union, Intersection, Difference.\n\
(3) Multi-Terminal MDDs (MTMDDs) with integer or real terminals:\n\
    Arithmetic: Plus, Minus, Multiply, Divide, Min, Max.\n\
    Logical: <, <=, >, >=, ==, !=.\n\
    Conversion to and from MDDs.\n\
(4) Multi-Terminal MXDs (MTMXDs) with integer or real terminals:\n\
    Arithmetic: Plus, Minus, Multiply, Divide, Min, Max.\n\
    Logical: <, <=, >, >=, ==, !=.\n\
    Conversion to and from MXDs.\n\
";

    case 5:
      return REVISION_DATE;
  }
  return 0;
}

// ******************************************************************
// *                                                                *
// *                    initializer_list methods                    *
// *                                                                *
// ******************************************************************

MEDDLY::initializer_list::initializer_list(initializer_list* prev)
{
  previous = prev;
}

MEDDLY::initializer_list::~initializer_list()
{
  delete previous;
}

void MEDDLY::initializer_list::setupAll()
{
  if (previous) previous->setupAll();
  setup();
}

void MEDDLY::initializer_list::cleanupAll()
{
  cleanup();
  if (previous) previous->cleanupAll();
}

