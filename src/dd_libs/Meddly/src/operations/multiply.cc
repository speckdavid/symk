
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "../defines.h"
#include "multiply.h"
#include "apply_base.h"

namespace MEDDLY {
  class multiply_mdd;
  class multiply_mxd;
  class multiply_evplus;
  class multiply_evtimes;

  class multiply_opname;
};

// ******************************************************************
// *                                                                *
// *                       multiply_mdd class                       *
// *                                                                *
// ******************************************************************

class MEDDLY::multiply_mdd : public generic_binary_mdd {
  public:
    multiply_mdd(const binary_opname* opcode, expert_forest* arg1,
      expert_forest* arg2, expert_forest* res);

  protected:
    virtual bool checkTerminals(node_handle a, node_handle b, node_handle& c);
};

MEDDLY::multiply_mdd::multiply_mdd(const binary_opname* opcode, 
  expert_forest* arg1, expert_forest* arg2, expert_forest* res)
  : generic_binary_mdd(opcode, arg1, arg2, res)
{
  operationCommutes();
}

bool MEDDLY::multiply_mdd::checkTerminals(node_handle a, node_handle b, node_handle& c)
{
  if (a == 0 || b == 0) {
    c = 0;
    return true;
  }
  if (arg1F->isTerminalNode(a)) {
    if (arg2F->isTerminalNode(b)) {
      if (resF->getRangeType() == forest::INTEGER) {
        int av, bv;
        arg1F->getValueFromHandle(a, av);
        arg2F->getValueFromHandle(b, bv);
        c = resF->handleForValue(av * bv);
      } else {
        MEDDLY_DCASSERT(resF->getRangeType() == forest::REAL);
        float av, bv;
        arg1F->getValueFromHandle(a, av);
        arg2F->getValueFromHandle(b, bv);
        c = resF->handleForValue(av * bv);
      }
      return true;
    }
    if (arg2F != resF) return false;
    if (resF->getRangeType() == forest::INTEGER) {
      if (1==arg1F->getIntegerFromHandle(a)) {
        c = arg2F->linkNode(b);
        return true;
      }
    } else {
      MEDDLY_DCASSERT(resF->getRangeType() == forest::REAL);
      if (1.0==arg1F->getRealFromHandle(a)) {
        c = arg2F->linkNode(b);
        return true;
      }
    }
  } // a is terminal
  if (arg2F->isTerminalNode(b)) {
    if (arg1F != resF) return false;
    if (resF->getRangeType() == forest::INTEGER) {
      if (1==arg2F->getIntegerFromHandle(b)) {
        c = arg1F->linkNode(a);
        return true;
      }
    } else {
      MEDDLY_DCASSERT(resF->getRangeType() == forest::REAL);
      if (1.0==arg2F->getRealFromHandle(b)) {
        c = arg1F->linkNode(a);
        return true;
      }
    }
  }
  return false;
}



// ******************************************************************
// *                                                                *
// *                       multiply_mxd class                       *
// *                                                                *
// ******************************************************************

class MEDDLY::multiply_mxd : public generic_binary_mxd {
  public:
    multiply_mxd(const binary_opname* opcode, expert_forest* arg1,
      expert_forest* arg2, expert_forest* res);

  protected:
    virtual bool checkTerminals(node_handle a, node_handle b, node_handle& c);
};

MEDDLY::multiply_mxd::multiply_mxd(const binary_opname* opcode, 
  expert_forest* arg1, expert_forest* arg2, expert_forest* res)
  : generic_binary_mxd(opcode, arg1, arg2, res)
{
  operationCommutes();
}

bool MEDDLY::multiply_mxd::checkTerminals(node_handle a, node_handle b, node_handle& c)
{
  if (a == 0 || b == 0) {
    c = 0;
    return true;
  }

  if (arg1F->isTerminalNode(a)) {
    if (arg2F->isTerminalNode(b)) {
      if (resF->getRangeType() == forest::INTEGER) {
        int av, bv;
        arg1F->getValueFromHandle(a, av);
        arg2F->getValueFromHandle(b, bv);
        c = resF->handleForValue(av * bv);
      } else {
        MEDDLY_DCASSERT(resF->getRangeType() == forest::REAL);
        float av, bv;
        arg1F->getValueFromHandle(a, av);
        arg2F->getValueFromHandle(b, bv);
        c = resF->handleForValue(av * bv);
      }
      return true;
    }
    if (arg2F != resF) return false;
    if (resF->getRangeType() == forest::INTEGER) {
      if (1==arg1F->getIntegerFromHandle(a)) {
        c = arg2F->linkNode(b);
        return true;
      }
    } else {
      MEDDLY_DCASSERT(resF->getRangeType() == forest::REAL);
      if (1.0==arg1F->getRealFromHandle(a)) {
        c = arg2F->linkNode(b);
        return true;
      }
    }
  } // a is terminal
  if (arg2F->isTerminalNode(b)) {
    if (arg1F != resF) return false;
    if (resF->getRangeType() == forest::INTEGER) {
      if (1==arg2F->getIntegerFromHandle(b)) {
        c = arg1F->linkNode(a);
        return true;
      }
    } else {
      MEDDLY_DCASSERT(resF->getRangeType() == forest::REAL);
      if (1.0==arg2F->getRealFromHandle(b)) {
        c = arg1F->linkNode(a);
        return true;
      }
    }
  }
  return false;
}


// ******************************************************************
// *                                                                *
// *                     multiply_evplus  class                     *
// *                                                                *
// ******************************************************************

class MEDDLY::multiply_evplus : public generic_binary_evplus {
  public:
    multiply_evplus(const binary_opname* opcode, expert_forest* arg1,
      expert_forest* arg2, expert_forest* res);

  protected:
    virtual bool checkTerminals(int aev, node_handle a, int bev, node_handle b, 
      int& cev, node_handle& c);
};

MEDDLY::multiply_evplus::multiply_evplus(const binary_opname* opcode, 
  expert_forest* arg1, expert_forest* arg2, expert_forest* res)
  : generic_binary_evplus(opcode, arg1, arg2, res)
{
  operationCommutes();
}

bool MEDDLY::multiply_evplus::checkTerminals(int aev, node_handle a, int bev, node_handle b,
  int& cev, node_handle& c)
{
  if (a == 0 || b == 0) {
    c = 0; cev = Inf();
    return true;
  }
  if (a == -1 && b == -1) {
    c = -1; cev = aev * bev;
    return true;
  }
  return false;
}



// ******************************************************************
// *                                                                *
// *                     multiply_evtimes class                     *
// *                                                                *
// ******************************************************************

class MEDDLY::multiply_evtimes : public generic_binary_evtimes {
  public:
    multiply_evtimes(const binary_opname* opcode, expert_forest* arg1,
      expert_forest* arg2, expert_forest* res);

  protected:
    virtual bool checkTerminals(float aev, node_handle a, float bev, node_handle b, 
      float& cev, node_handle& c);
};

MEDDLY::multiply_evtimes::multiply_evtimes(const binary_opname* opcode, 
  expert_forest* arg1, expert_forest* arg2, expert_forest* res)
  : generic_binary_evtimes(opcode, arg1, arg2, res)
{
  operationCommutes();
}

bool MEDDLY::multiply_evtimes::checkTerminals(float aev, node_handle a, 
  float bev, node_handle b, float& cev, node_handle& c)
{
  if (a == 0 || b == 0) {
    c = 0; cev = Nan();
    return true;
  }
  if (a == -1 && b == -1) {
    c = -1; cev = aev * bev;
    return true;
  }
  return false;
}



// ******************************************************************
// *                                                                *
// *                     multiply_opname  class                     *
// *                                                                *
// ******************************************************************

class MEDDLY::multiply_opname : public binary_opname {
  public:
    multiply_opname();
    virtual binary_operation* buildOperation(expert_forest* a1, 
      expert_forest* a2, expert_forest* r) const;
};

MEDDLY::multiply_opname::multiply_opname()
 : binary_opname("Multiply")
{
}

MEDDLY::binary_operation* 
MEDDLY::multiply_opname::buildOperation(expert_forest* a1, expert_forest* a2, 
  expert_forest* r) const
{
  if (0==a1 || 0==a2 || 0==r) return 0;

  if (  
    (a1->getDomain() != r->getDomain()) || 
    (a2->getDomain() != r->getDomain()) 
  )
    throw error(error::DOMAIN_MISMATCH);

  if (
    (a1->isForRelations() != r->isForRelations()) ||
    (a2->isForRelations() != r->isForRelations()) ||
    (a1->getEdgeLabeling() != r->getEdgeLabeling()) ||
    (a2->getEdgeLabeling() != r->getEdgeLabeling()) ||
    (r->getRangeType() == forest::BOOLEAN)
  )
    throw error(error::TYPE_MISMATCH);

  if (r->getEdgeLabeling() == forest::MULTI_TERMINAL) {
    if (r->isForRelations())
      return new multiply_mxd(this, a1, a2, r);
    else
      return new multiply_mdd(this, a1, a2, r);
  }

  if (
    (a1->getRangeType() != r->getRangeType()) ||
    (a2->getRangeType() != r->getRangeType()) 
  )
    throw error(error::TYPE_MISMATCH);

  if (r->getEdgeLabeling() == forest::EVPLUS && r->getRangeType() == forest::INTEGER)
    return new multiply_evplus(this, a1, a2, r);

  if (r->getEdgeLabeling() == forest::EVTIMES)
    return new multiply_evtimes(this, a1, a2, r);

  throw error(error::NOT_IMPLEMENTED);
}

// ******************************************************************
// *                                                                *
// *                           Front  end                           *
// *                                                                *
// ******************************************************************

MEDDLY::binary_opname* MEDDLY::initializeMultiply()
{
  return new multiply_opname;
}

