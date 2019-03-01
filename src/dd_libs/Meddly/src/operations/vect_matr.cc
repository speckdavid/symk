
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

#include "../defines.h"
#include "vect_matr.h"
#include <typeinfo> // for "bad_cast" exception

namespace MEDDLY {
  class base_evplus_mt;

  class VM_evplus_mt;

  class MV_evplus_mt;

  class VM_opname;
  class MV_opname;

  inline bool isEvPlusStyle(const forest* f) {
    return f->isEVPlus() || f->isIndexSet();
  }
};

// ******************************************************************
// *                                                                *
// *                      base_evplus_mt class                      *
// *                                                                *
// ******************************************************************

class MEDDLY::base_evplus_mt : public specialized_operation {
  public:
    base_evplus_mt(const numerical_opname* code, const dd_edge &x_ind,
      const dd_edge& A, const dd_edge &y_ind);

    virtual ~base_evplus_mt();

    virtual void compute(double* y, const double* x);

    virtual void compute_r(int ht, double* y, node_handle y_ind, const double* x, 
      node_handle x_ind, node_handle A) = 0;

    virtual bool isStaleEntry(const node_handle*) {
      throw error(error::MISCELLANEOUS);
    }
    virtual void discardEntry(const node_handle*) {
      throw error(error::MISCELLANEOUS);
    }
    virtual void showEntry(output &, const node_handle*) const {
      throw error(error::MISCELLANEOUS);
    }

  protected:
    const expert_forest* fx;
    const expert_forest* fA;
    const expert_forest* fy;
    node_handle x_root;
    node_handle A_root;
    node_handle y_root;
    int L;

    inline virtual bool checkForestCompatibility() const
    {
      auto o1 = fx->variableOrder();
      auto o2 = fA->variableOrder();
      auto o3 = fy->variableOrder();
      return o1->is_compatible_with(*o2) && o1->is_compatible_with(*o3);
    }
};

MEDDLY::base_evplus_mt::base_evplus_mt(const numerical_opname* code, 
  const dd_edge &x_ind, const dd_edge& A, const dd_edge &y_ind)
 : specialized_operation(code, 0, 0)
{
  fx = (const expert_forest*) x_ind.getForest();
  fA = (const expert_forest*) A.getForest();
  fy = (const expert_forest*) y_ind.getForest();
  MEDDLY_DCASSERT(fx);
  MEDDLY_DCASSERT(fA);
  MEDDLY_DCASSERT(fy);
  x_root = x_ind.getNode();
  A_root = A.getNode();
  y_root = y_ind.getNode();
  L = fx->getDomain()->getNumVariables();
}

MEDDLY::base_evplus_mt::~base_evplus_mt()
{
}

void MEDDLY::base_evplus_mt::compute(double* y, const double* x)
{
  if (!checkForestCompatibility()) {
    throw error(error::INVALID_OPERATION);
  }
  compute_r(L, y, y_root, x, x_root, A_root);
}

// ******************************************************************
// *                                                                *
// *                       VM_evplus_mt class                       *
// *                                                                *
// ******************************************************************

class MEDDLY::VM_evplus_mt : public base_evplus_mt {
  public:
    VM_evplus_mt(const numerical_opname* code, const dd_edge &x_ind,
      const dd_edge& A, const dd_edge &y_ind);

    virtual void compute_r(int k, double* y, node_handle y_ind, const double* x, 
      node_handle x_ind, node_handle A);

    void comp_pr(int k, double* y, node_handle y_ind, const double* x, 
      node_handle x_ind, int ain, node_handle A);

};

MEDDLY::VM_evplus_mt::VM_evplus_mt(const numerical_opname* code, 
  const dd_edge &x_ind, const dd_edge& A, const dd_edge &y_ind)
  : base_evplus_mt(code, x_ind, A, y_ind)
{
}

void MEDDLY::VM_evplus_mt::compute_r(int k, double* y, node_handle y_ind, 
  const double* x, node_handle x_ind, node_handle a)
{
  // Handles the unprimed levels of a
  if (0==k) {
    y[0] += x[0] * expert_forest::float_Tencoder::handle2value(a);
    return;
  }

  // It should be impossible for an indexing function to skip levels, right?   
  MEDDLY_DCASSERT(fx->getNodeLevel(x_ind) == k);
  MEDDLY_DCASSERT(fy->getNodeLevel(y_ind) == k);
  int aLevel = fA->getNodeLevel(a);

  //
  // A is identity matrix times a constant; exploit that if we can
  //
  if (0==aLevel && (x_ind == y_ind)) {
    if (fx == fy && fx->isIndexSet()) {
      // yes we can
      float v = expert_forest::float_Tencoder::handle2value(a);
      for (long i = fx->getIndexSetCardinality(x_ind)-1; i>=0; i--) {
        y[i] += x[i] * v;
      }
      return;
    }
  }

  //
  // Check if A is an identity node
  //
  if (ABS(aLevel) < k) {
    // Init sparse readers
    unpacked_node* xR = unpacked_node::newFromNode(fx, x_ind, false);
    unpacked_node* yR = unpacked_node::newFromNode(fy, y_ind, false);

    int xp = 0;
    int yp = 0;
    for (;;) {
      if (xR->i(xp) < yR->i(yp)) {
        xp++;
        if (xp >= xR->getNNZs()) break;
        continue;
      }
      if (xR->i(xp) > yR->i(yp)) {
        yp++;
        if (yp >= yR->getNNZs()) break;
        continue;
      }
      // match, need to recurse
      compute_r(k-1, y + yR->ei(yp), yR->d(yp), x + xR->ei(xp), xR->d(xp), a);
      xp++;
      if (xp >= xR->getNNZs()) break;
      yp++;
      if (yp >= yR->getNNZs()) break;
    } // for (;;)
    
    // Cleanup
    unpacked_node::recycle(yR);
    unpacked_node::recycle(xR);

    // Done
    return;
  }

  //
  // A is not an identity node.
  //

  // Init sparse readers
  unpacked_node* aR = unpacked_node::useUnpackedNode();
  if (aLevel == k) {
    aR->initFromNode(fA, a, false);
  } else {
    aR->initRedundant(fA, k, a, false);
  }

  unpacked_node* xR = unpacked_node::useUnpackedNode();
  xR->initFromNode(fx, x_ind, false);

  int xp = 0;
  int ap = 0;
  for (;;) {
    if (aR->i(ap) < xR->i(xp)) {
      ap++;
      if (ap >= aR->getNNZs()) break;
      continue;
    }
    if (aR->i(ap) > xR->i(xp)) {
      xp++;
      if (xp >= xR->getNNZs()) break;
      continue;
    }
    // match, need to recurse
    comp_pr(k, y, y_ind, x + xR->ei(xp), xR->d(xp), aR->i(ap), aR->d(ap));
    ap++;
    if (ap >= aR->getNNZs()) break;
    xp++;
    if (xp >= xR->getNNZs()) break;
  } // for (;;)

  // Cleanup
  unpacked_node::recycle(xR);
  unpacked_node::recycle(aR);
}

void MEDDLY::VM_evplus_mt::comp_pr(int k, double* y, node_handle y_ind, 
  const double* x, node_handle x_ind, int ain, node_handle a)
{
  // Handles the primed levels of A
  if (0==k) {
    y[0] += x[0] * expert_forest::float_Tencoder::handle2value(a);
    return;
  }

  // Init sparse readers
  unpacked_node* aR = unpacked_node::useUnpackedNode();
  if (fA->getNodeLevel(a) == -k) {
    aR->initFromNode(fA, a, false);
  } else {
    aR->initIdentity(fA, k, ain, a, false);
  }

  unpacked_node* yR = unpacked_node::newFromNode(fy, y_ind, false);


  int yp = 0;
  int ap = 0;
  for (;;) {
    if (aR->i(ap) < yR->i(yp)) {
      ap++;
      if (ap >= aR->getNNZs()) break;
      continue;
    }
    if (aR->i(ap) > yR->i(yp)) {
      yp++;
      if (yp >= yR->getNNZs()) break;
      continue;
    }
    // match, need to recurse
    compute_r(k-1, y + yR->ei(yp), yR->d(yp), x, x_ind, aR->d(ap));
    ap++;
    if (ap >= aR->getNNZs()) break;
    yp++;
    if (yp >= yR->getNNZs()) break;
  } // for (;;)

  // Cleanup
  unpacked_node::recycle(yR);
  unpacked_node::recycle(aR);
}


// ******************************************************************
// *                                                                *
// *                       MV_evplus_mt class                       *
// *                                                                *
// ******************************************************************

class MEDDLY::MV_evplus_mt : public base_evplus_mt {
  public:
    MV_evplus_mt(const numerical_opname* code, const dd_edge &x_ind,
      const dd_edge& A, const dd_edge &y_ind);

    virtual void compute_r(int k, double* y, node_handle y_ind, const double* x, 
      node_handle x_ind, node_handle A);

    void comp_pr(int k, double* y, node_handle y_ind, const double* x, 
      node_handle x_ind, int ain, node_handle A);

};

MEDDLY::MV_evplus_mt::MV_evplus_mt(const numerical_opname* code, 
  const dd_edge &x_ind, const dd_edge& A, const dd_edge &y_ind)
  : base_evplus_mt(code, x_ind, A, y_ind)
{
}

void MEDDLY::MV_evplus_mt::compute_r(int k, double* y, node_handle y_ind, 
  const double* x, node_handle x_ind, node_handle a)
{
  // Handles the unprimed levels of a
  if (0==k) {
    y[0] += x[0] * expert_forest::float_Tencoder::handle2value(a);
    return;
  }

  // It should be impossible for an indexing function to skip levels, right?   
  MEDDLY_DCASSERT(fx->getNodeLevel(x_ind) == k);
  MEDDLY_DCASSERT(fy->getNodeLevel(y_ind) == k);
  int aLevel = fA->getNodeLevel(a);

  //
  // A is identity matrix times a constant; exploit that if we can
  //
  if (0==aLevel && (x_ind == y_ind)) {
    if (fx == fy && fx->isIndexSet()) {
      // yes we can
      float v = expert_forest::float_Tencoder::handle2value(a);
      for (long i = fy->getIndexSetCardinality(y_ind)-1; i>=0; i--) {
        y[i] += x[i] * v;
      }
      return;
    }
  }

  //
  // Check if a is an identity node
  //
  if (ABS(aLevel) < k) {
    // Init sparse readers
    unpacked_node* xR = unpacked_node::newFromNode(fx, x_ind, false);
    unpacked_node* yR = unpacked_node::newFromNode(fy, y_ind, false);

    int xp = 0;
    int yp = 0;
    for (;;) {
      if (xR->i(xp) < yR->i(yp)) {
        xp++;
        if (xp >= xR->getNNZs()) break;
        continue;
      }
      if (xR->i(xp) > yR->i(yp)) {
        yp++;
        if (yp >= yR->getNNZs()) break;
        continue;
      }
      // match, need to recurse
      compute_r(k-1, y + yR->ei(yp), yR->d(yp), x + xR->ei(xp), xR->d(xp), a);
      xp++;
      if (xp >= xR->getNNZs()) break;
      yp++;
      if (yp >= yR->getNNZs()) break;
    } // for (;;)
    
    // Cleanup
    unpacked_node::recycle(yR);
    unpacked_node::recycle(xR);

    // Done
    return;
  }

  //
  // A is not an identity node.
  //

  // Init sparse readers
  unpacked_node* aR = unpacked_node::useUnpackedNode();
  if (aLevel == k) {
    aR->initFromNode(fA, a, false);
  } else {
    aR->initRedundant(fA, k, a, false);
  }

  unpacked_node* yR = unpacked_node::newFromNode(fy, y_ind, false);


  int yp = 0;
  int ap = 0;
  for (;;) {
    if (aR->i(ap) < yR->i(yp)) {
      ap++;
      if (ap >= aR->getNNZs()) break;
      continue;
    }
    if (aR->i(ap) > yR->i(yp)) {
      yp++;
      if (yp >= yR->getNNZs()) break;
      continue;
    }
    // match, need to recurse
    comp_pr(k, y + yR->ei(yp), yR->d(yp), x, x_ind, aR->i(ap), aR->d(ap));
    ap++;
    if (ap >= aR->getNNZs()) break;
    yp++;
    if (yp >= yR->getNNZs()) break;
  } // for (;;)

  // Cleanup
  unpacked_node::recycle(yR);
  unpacked_node::recycle(aR);
}

void MEDDLY::MV_evplus_mt::comp_pr(int k, double* y, node_handle y_ind, 
  const double* x, node_handle x_ind, int ain, node_handle a)
{
  // Handles the primed levels of A
  if (0==k) {
    y[0] += x[0] * expert_forest::float_Tencoder::handle2value(a);
    return;
  }

  // Init sparse readers
  unpacked_node* aR = unpacked_node::useUnpackedNode();
  if (fA->getNodeLevel(a) == -k) {
    aR->initFromNode(fA, a, false);
  } else {
    aR->initIdentity(fA, k, ain, a, false);
  }

  unpacked_node* xR = unpacked_node::newFromNode(fx, x_ind, false);


  int xp = 0;
  int ap = 0;
  for (;;) {
    if (aR->i(ap) < xR->i(xp)) {
      ap++;
      if (ap >= aR->getNNZs()) break;
      continue;
    }
    if (aR->i(ap) > xR->i(xp)) {
      xp++;
      if (xp >= xR->getNNZs()) break;
      continue;
    }
    // match, need to recurse
    compute_r(k-1, y, y_ind, x + xR->ei(xp), xR->d(xp), aR->d(ap));
    ap++;
    if (ap >= aR->getNNZs()) break;
    xp++;
    if (xp >= xR->getNNZs()) break;
  } // for (;;)

  // Cleanup
  unpacked_node::recycle(xR);
  unpacked_node::recycle(aR);
}


// ******************************************************************
// *                                                                *
// *                        VM_opname  class                        *
// *                                                                *
// ******************************************************************

class MEDDLY::VM_opname : public numerical_opname {
  public:
    VM_opname();
    virtual specialized_operation* buildOperation(arguments* a) const;
};

MEDDLY::VM_opname::VM_opname() : numerical_opname("VectMatrMult")
{
}

MEDDLY::specialized_operation* 
MEDDLY::VM_opname::buildOperation(arguments* a) const
{
  numerical_args* na = dynamic_cast<numerical_args*>(a);
  if (0==na) throw error(error::INVALID_ARGUMENT);

  const expert_forest* fx = (const expert_forest*) na->x_ind.getForest();
  const expert_forest* fA = (const expert_forest*) na->A.getForest();
  const expert_forest* fy = (const expert_forest*) na->y_ind.getForest();

  // everyone must use the same domain
  if (      (fx->getDomain() != fy->getDomain()) 
        ||  (fx->getDomain() != fA->getDomain())  )
  {
    throw error(error::DOMAIN_MISMATCH);
  }

  // Check edge types
  if (
           (fy->getRangeType() != forest::INTEGER) 
        || (fy->isForRelations())
        || (fx->getRangeType() != forest::INTEGER)
        || (fx->isForRelations())
        || (fA->getRangeType() != forest::REAL)
        || (!fA->isForRelations())
      ) 
  {
    throw error(error::TYPE_MISMATCH);
  }

  // A can't be fully reduced.
  if (fA->isFullyReduced()) {
    throw error(error::TYPE_MISMATCH);
  }

  // For now, fy and fx must be Indexed sets or EVPLUS forests.
  if ( !isEvPlusStyle(fy) || !isEvPlusStyle(fx) ) {
    throw error(error::NOT_IMPLEMENTED);
  }

  switch (fA->getEdgeLabeling()) {
    case forest::MULTI_TERMINAL:
      return new VM_evplus_mt(this, na->x_ind, na->A, na->y_ind);

    case forest::EVTIMES:
      throw error(error::NOT_IMPLEMENTED);

    default:
      throw error(error::TYPE_MISMATCH);
  };

  if (na->autoDestroy()) delete na;
}

// ******************************************************************
// *                                                                *
// *                        MV_opname  class                        *
// *                                                                *
// ******************************************************************

class MEDDLY::MV_opname : public numerical_opname {
  public:
    MV_opname();
    virtual specialized_operation* buildOperation(arguments* a) const;
};

MEDDLY::MV_opname::MV_opname() : numerical_opname("MatrVectMult")
{
}

MEDDLY::specialized_operation* 
MEDDLY::MV_opname::buildOperation(arguments* a) const
{
  numerical_args* na = dynamic_cast<numerical_args*>(a);
  if (0==na) throw error(error::INVALID_ARGUMENT);

  const expert_forest* fx = (const expert_forest*) na->x_ind.getForest();
  const expert_forest* fA = (const expert_forest*) na->A.getForest();
  const expert_forest* fy = (const expert_forest*) na->y_ind.getForest();


    // everyone must use the same domain
  if (      (fx->getDomain() != fy->getDomain()) 
        ||  (fx->getDomain() != fA->getDomain())  )
  {
    throw error(error::DOMAIN_MISMATCH);
  }

  // Check edge types
  if (
           (fy->getRangeType() != forest::INTEGER) 
        || (fy->isForRelations())
        || (fx->getRangeType() != forest::INTEGER)
        || (fx->isForRelations())
        || (fA->getRangeType() != forest::REAL)
        || (!fA->isForRelations())
      ) 
  {
    throw error(error::TYPE_MISMATCH);
  }

  // A can't be fully reduced.
  if (fA->isFullyReduced()) {
    throw error(error::TYPE_MISMATCH);
  }

  // For now, fy and fx must be Indexed sets or EVPLUS forests.
  if ( !isEvPlusStyle(fy) || !isEvPlusStyle(fx) ) {
    throw error(error::NOT_IMPLEMENTED);
  }

  switch (fA->getEdgeLabeling()) {
    case forest::MULTI_TERMINAL:
      return new MV_evplus_mt(this, na->x_ind, na->A, na->y_ind);

    case forest::EVTIMES:
      throw error(error::NOT_IMPLEMENTED);

    default:
      throw error(error::TYPE_MISMATCH);
  };

  if (na->autoDestroy()) delete na;
}

// ******************************************************************
// *                                                                *
// *                           Front  end                           *
// *                                                                *
// ******************************************************************

MEDDLY::numerical_opname* MEDDLY::initExplVectorMatrixMult()
{
  return new VM_opname;
}

MEDDLY::numerical_opname* MEDDLY::initMatrixExplVectorMult()
{
  return new MV_opname;
}

