
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
#include "divide.h"
#include "apply_base.h"

namespace MEDDLY {
  class divide_opname;
};


// ******************************************************************
// *                                                                *
// *                        divide_mdd class                        *
// *                                                                *
// ******************************************************************

namespace MEDDLY {

template <typename REAL>
class divide_mdd : public generic_binary_mdd {
  public:
    divide_mdd(const binary_opname* opcode, expert_forest* arg1,
      expert_forest* arg2, expert_forest* res)
      : generic_binary_mdd(opcode, arg1, arg2, res) { }

  protected:
    virtual bool checkTerminals(node_handle a, node_handle b, node_handle& c);
};

template <typename REAL>
bool divide_mdd<REAL>
::checkTerminals(node_handle a, node_handle b, node_handle& c)
{
  if (arg1F->isTerminalNode(a) && arg2F->isTerminalNode(b)) 
  {
    REAL av, bv;
    arg1F->getValueFromHandle(a, av);
    arg2F->getValueFromHandle(b, bv);
    if (0 == bv) throw error(error::DIVIDE_BY_ZERO);
    c = resF->handleForValue( av / bv );
    return true;
  }
  return false;
}

};  // namespace MEDDLY

// ******************************************************************
// *                                                                *
// *                        divide_mxd class                        *
// *                                                                *
// ******************************************************************

namespace MEDDLY {

template <typename REAL>
class divide_mxd : public generic_binbylevel_mxd {
  public:
    divide_mxd(const binary_opname* opcode, expert_forest* arg1,
      expert_forest* arg2, expert_forest* res)
      : generic_binbylevel_mxd(opcode, arg1, arg2, res) { }

  protected:
    virtual bool checkTerminals(node_handle a, node_handle b, node_handle& c);
};

template <typename REAL>
bool divide_mxd<REAL>
::checkTerminals(node_handle a, node_handle b, node_handle& c)
{
  if (arg1F->isTerminalNode(a) && arg2F->isTerminalNode(b)) 
  {
    REAL av, bv;
    arg1F->getValueFromHandle(a, av);
    arg2F->getValueFromHandle(b, bv);
    if (0 == bv) throw error(error::DIVIDE_BY_ZERO);
    c = resF->handleForValue( av / bv );
    return true;
  }
  return false;
}

};  // namespace MEDDLY

// ******************************************************************
// *                                                                *
// *                      divide_opname  class                      *
// *                                                                *
// ******************************************************************

class MEDDLY::divide_opname : public binary_opname {
  public:
    divide_opname();
    virtual binary_operation* buildOperation(expert_forest* a1, 
      expert_forest* a2, expert_forest* r) const;
};

MEDDLY::divide_opname::divide_opname()
 : binary_opname("Divide")
{
}

MEDDLY::binary_operation* 
MEDDLY::divide_opname::buildOperation(expert_forest* a1, expert_forest* a2, 
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
    (a2->getEdgeLabeling() != r->getEdgeLabeling()) 
  )
    throw error(error::TYPE_MISMATCH);

  if (r->getEdgeLabeling() == forest::MULTI_TERMINAL) {
    switch (r->getRangeType()) {

      case forest::INTEGER:
          if (r->isForRelations())
            return new divide_mxd<int>(this, a1, a2, r);
          else
            return new divide_mdd<int>(this, a1, a2, r);

      case forest::REAL:
          if (r->isForRelations())
            return new divide_mxd<float>(this, a1, a2, r);
          else
            return new divide_mdd<float>(this, a1, a2, r);

      default:
        throw error(error::TYPE_MISMATCH);
    }
    
  }

  throw error(error::NOT_IMPLEMENTED);
}

// ******************************************************************
// *                                                                *
// *                           Front  end                           *
// *                                                                *
// ******************************************************************

MEDDLY::binary_opname* MEDDLY::initializeDivide()
{
  return new divide_opname;
}

