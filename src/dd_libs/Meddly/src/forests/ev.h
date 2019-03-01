
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

#ifndef EV_FOREST
#define EV_FOREST

#include "../defines.h"

namespace MEDDLY {
  class ev_forest;
};

/**
    Base class for all edge-valued DDs.

    Derived classes probably want to build an OPERATION class
    that contains inlined static methods:
        void setEdge(void*, type v);
        bool isIdentityEdge(const void*);
*/
class MEDDLY::ev_forest : public expert_forest {
  protected:
    ev_forest(int dsl, domain *d, bool rel, range_type t, edge_labeling ev,
      const policies &p,int* level_reduction_rule=NULL);

  protected:
    virtual void showTerminal(output &s, node_handle tnode) const;
    virtual void writeTerminal(output &s, node_handle tnode) const;
    virtual node_handle readTerminal(input &s);

  public:

    template <class OPERATION>
    inline bool isRedundantTempl(const unpacked_node &nb) const {
      if (isQuasiReduced()) return false;
      if (nb.getLevel() < 0 && isIdentityReduced()) return false;
      int rawsize = nb.isSparse() ? nb.getNNZs() : nb.getSize();
      if (rawsize < getLevelSize(nb.getLevel())) return false;
      int common = nb.d(0);
      for (int i=1; i<rawsize; i++) {
        if (nb.d(i) != common)  return false;
      }
      // This might be expensive, so split the loops to cheapest first
      for (int i=0; i<rawsize; i++) {
        if (!OPERATION::isIdentityEdge(nb.eptr(i))) return false;
      }
      return true;
    }

    template <class OPERATION>
    inline bool isIdentityEdgeTempl(const unpacked_node &nb, int i) const {
      if (nb.getLevel() > 0) return false;
      if (!isIdentityReduced()) return false;
      if (i<0) return false;
      return (nb.d(i) != 0)  &&  OPERATION::isIdentityEdge(nb.eptr(i));
    }

  // ------------------------------------------------------------
  // Helpers for this and derived classes

    /** Add redundant nodes from level k to the given node.
          @param  k   Top level we want
          @param  ev  In/Out: Edge value
          @param  ed  In/Out: Edge node pointer
    */
    template <class OPERATION, typename TYPE>
    void makeNodeAtLevel(int k, TYPE &ev, node_handle &ed);

  protected:
    /// make a node at the top level
    template <class OPERATION, typename TYPE>
    inline void makeNodeAtTop(TYPE &ev, node_handle &ed) {
      makeNodeAtLevel<OPERATION>(getDomain()->getNumVariables(), ev, ed);
    }

    /**
        Enlarge variables to include all given minterms.
    */
    inline void enlargeVariables(const int* const* vlist, int N, bool primed) {
      for (int k=1; k<=getDomain()->getNumVariables(); k++) {
        int maxv = vlist[0][k];
        for (int i=1; i<N; i++) {
          maxv = MAX(maxv, vlist[i][k]);
        }
        if (maxv < 1) continue;
        if (maxv >= getDomain()->getVariableBound(k, primed)) {
          useExpertDomain()->enlargeVariableBound(k, primed, maxv+1);
        }
      }
    }

    template <class OPERATION, typename T>
    inline
    void createEdgeForVarTempl(int vh, bool pr, const T* vals, dd_edge& result)
    {
      /*
          Sanity checks
      */
      if (vh < 0 || vh > getNumVariables())
        throw error(error::INVALID_VARIABLE);
      if (result.getForest() != this) 
        throw error(error::INVALID_OPERATION);
      if (!isForRelations() && pr) 
        throw error(error::INVALID_ASSIGNMENT);

      /*
          Get info for node we're building
      */
      int k = pr ? -vh: vh;
      int km1;
      if (isForRelations()) {
        km1 = (k<0) ? (-k)-1 : -k;
      } else {
        km1 = k-1;
      }
      int sz = getLevelSize(vh);

      /*
          Make this node
      */
      unpacked_node *nb = unpacked_node::newFull(this, k, sz);
      for (int i=0; i<sz; i++) {
        T ev = vals ? vals[i] : i;
        node_handle ed = bool_Tencoder::value2handle(true);
        makeNodeAtLevel<OPERATION, T>(km1, ev, ed);
        nb->d_ref(i) = ed;
        nb->setEdge(i, ev);
      }

      /*
          Reduce, add redundant as necessary, and set answer
      */
      T ev;
      node_handle node;
      createReducedNode(-1, nb, ev, node);
      makeNodeAtTop<OPERATION, T>(ev, node); 
      result.set(node, ev);
    }


    template <class OPERATION, class T>
    inline void createEdgeTempl(T term, dd_edge& e) {
      if (e.getForest() != this) throw error(error::INVALID_OPERATION);
      node_handle ed = bool_Tencoder::value2handle(true);
      makeNodeAtTop<OPERATION, T>(term, ed);
      e.set(ed, term);
    }



  // statics

  public:
    static void initStatics();
    static void enlargeStatics(int n);
    static void clearStatics();

  protected:
    static int* order;
    static int order_size;
};







namespace MEDDLY {

  template <class OPERATION, typename TYPE>
  void ev_forest::makeNodeAtLevel(int k, TYPE &ev, node_handle &ed)
  {
    MEDDLY_DCASSERT(abs(k) >= abs(getNodeLevel(ed)));
    if (0==ed) return;
    if (isFullyReduced()) return;

    // prevSize == 1 enables getSingletonIndex for the first run.
    int prevSize = 1;
    int dk = getNodeLevel(ed);
    while (dk != k) {
      // make node at one level up
      int up = (dk < 0) ? -dk : isForRelations() ? -(dk + 1) : (dk + 1);
      int sz = getLevelSize(up);
      unpacked_node* nb = unpacked_node::newFull(this, up, sz);
  
      if (isIdentityReduced() && (dk < 0) && (1 == prevSize)) {
        // Build unprimed node with check for identity reduction.
        // Note 0: ed cannot be a terminal node (dk < 0).
        // Note 1: if prevSize > 1, and ed is not the original argument,
        //         and if it is a primed node, it cannot be identity reduced.
        MEDDLY_DCASSERT(!isTerminalNode(ed));
        node_handle sd;
        int si = getSingletonIndex(ed, sd);
        for (int i = 0; i < sz; i++) {
          nb->d_ref(i) = linkNode( (si == i) ? sd : ed );
          nb->setEdge(i, ev);
        }
      } else {
        // No identity reduction possible.
        for (int i = 0; i < sz; i++) {
          nb->d_ref(i) = linkNode(ed);
          nb->setEdge(i, ev);
        }
      }

      unlinkNode(ed);
      createReducedNode(-1, nb, ev, ed);
      dk = up;
      prevSize = sz;
    } // while
  }

};  // namespace MEDDLY

#endif

