
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

#ifndef EVMXD_H
#define EVMXD_H

#include "ev.h"

namespace MEDDLY {
  class evmxd_forest;
};

class MEDDLY::evmxd_forest : public ev_forest {
  public:
    evmxd_forest(int dsl, domain* d, range_type t, edge_labeling ev, 
      const policies &p,int* level_reduction_rule=NULL);

    virtual void reorderVariables(const int* level2var);
    virtual void swapAdjacentVariables(int level);
    virtual void moveDownVariable(int high, int low);
    virtual void moveUpVariable(int low, int high);

  protected:
    template <class OPERATION, typename TYPE>
    inline void evaluateT(const dd_edge &f, const int* vlist,
      const int* vplist, TYPE &val) const 
    {
      if (f.getForest() != this) throw error(error::INVALID_OPERATION);
      if (vlist == 0) throw error(error::INVALID_VARIABLE);
      if (vplist == 0) throw error(error::INVALID_VARIABLE);

      // Assumptions:
      // (1) vlist and vplist do not contain special values (-1, -2, etc).
      // (2) vlist and vplist contains a single element.
      node_handle node = f.getNode();
      f.getEdgeValue(val);

      while (!isTerminalNode(node)) {
        TYPE ev;
        int k = getNodeLevel(node);
        getDownPtr(node, ((k > 0) ? vlist[k] : vplist[-k]), ev, node);
        val = (node) ? OPERATION::apply(val, ev) : ev;
      }
    }
    
  public:
    /// Special case for createEdge(), with only one minterm.
    template <class OPERATION, typename TYPE>
    inline void 
    createEdgePath(int k, const int* vlist, const int* vplist,
      TYPE &ev, node_handle &ed) 
    {
      if (0==ed) return;

      for (int i=1; i<=k; i++) {
        if (DONT_CHANGE == vplist[i]) {
          //
          // Identity node
          //
          MEDDLY_DCASSERT(DONT_CARE == vlist[i]);
          if (isIdentityReduced()) continue;
          // Build an identity node by hand
          int sz = getLevelSize(i);
          unpacked_node* nb = unpacked_node::newFull(this, i, sz);
          for (int v=0; v<sz; v++) {
            unpacked_node* nbp = unpacked_node::newSparse(this, -i, 1);
            nbp->i_ref(0) = v;
            nbp->d_ref(0) = linkNode(ed);
            nbp->setEdge(0, ev);
            TYPE pev;
            node_handle pd;
            createReducedNode(v, nbp, pev, pd);
            nb->d_ref(v) = pd;
            nb->setEdge(v, pev);
          }
          unlinkNode(ed);
          createReducedNode(-1, nb, ev, ed);
          continue;
        }
        //
        // process primed level
        //
        node_handle edpr;
        TYPE evpr;
        if (DONT_CARE == vplist[i]) {
          if (isFullyReduced()) {
            // DO NOTHING
            edpr = ed;
            evpr = ev;
          } else {
            // build redundant node
            int sz = getLevelSize(-i);
            unpacked_node* nb = unpacked_node::newFull(this, -i, sz);
            for (int v=0; v<sz; v++) {
              nb->d_ref(v) = linkNode(ed);
              nb->setEdge(v, ev);
            }
            unlinkNode(ed);
            createReducedNode(-1, nb, evpr, edpr);
          }
        } else {
          // sane value
          unpacked_node* nb = unpacked_node::newSparse(this, -i, 1);
          nb->i_ref(0) = vplist[i];
          nb->d_ref(0) = ed;
          nb->setEdge(0, ev);
          createReducedNode(vlist[i], nb, evpr, edpr);
        }
        //
        // process unprimed level
        //
        if (DONT_CARE == vlist[i]) {
          if (isFullyReduced()) {
            ed = edpr;
            ev = evpr;
            continue;
          }
          // build redundant node
          int sz = getLevelSize(i);
          unpacked_node *nb = unpacked_node::newFull(this, i, sz);
          if (isIdentityReduced()) {
            // Below is likely a singleton, so check for identity reduction
            // on the appropriate v value
            for (int v=0; v<sz; v++) {
              node_handle dpr = (v == vplist[i]) ? ed : edpr;
              nb->d_ref(v) = linkNode(dpr);
              nb->setEdge(v, evpr);
            }
          } else {
            // Doesn't matter what happened below
            for (int v=0; v<sz; v++) {
              nb->d_ref(v) = linkNode(edpr);
              nb->setEdge(v, evpr);
            }
          }
          unlinkNode(edpr);
          createReducedNode(-1, nb, ev, ed);
        } else {
          // sane value
          unpacked_node* nb = unpacked_node::newSparse(this, i, 1);
          nb->i_ref(0) = vlist[i];
          nb->d_ref(0) = edpr;
          nb->setEdge(0, evpr);
          createReducedNode(-1, nb, ev, ed);
        }
      } // for i
    }

};


//
// Helper class for createEdge
//

namespace MEDDLY {

  template <class OPERATION, typename T>
  class evmxd_edgemaker {
      evmxd_forest* F;
      const int* const* vulist;
      const int* const* vplist;
      const T* values;
      int* order;
      int N;
      int K;
      binary_operation* unionOp;
    public:
      evmxd_edgemaker(evmxd_forest* f,
        const int* const* mt, const int* const* mp, const T* v, 
        int* o, int n, int k, binary_operation* unOp) 
      {
        F = f;
        vulist = mt;
        vplist = mp;
        values = v;
        order = o;
        N = n;
        K = k;
        unionOp = unOp;
      }

      inline const int* unprimed(int i) const {
        MEDDLY_CHECK_RANGE(0, i, N);
        return vulist[order[i]];
      }
      inline int unprimed(int i, int k) const {
        MEDDLY_CHECK_RANGE(0, i, N);
        MEDDLY_CHECK_RANGE(1, k, K+1);
        return vulist[order[i]][k];
      }
      inline const int* primed(int i) const {
        MEDDLY_CHECK_RANGE(0, i, N);
        return vplist[order[i]];
      }
      inline int primed(int i, int k) const {
        MEDDLY_CHECK_RANGE(0, i, N);
        MEDDLY_CHECK_RANGE(1, k, K+1);
        return vplist[order[i]][k];
      }
      inline T term(int i) const {
        MEDDLY_CHECK_RANGE(0, i, N);
        return values ? values[order[i]]: 1;
      }
      inline void swap(int i, int j) {
        MEDDLY_CHECK_RANGE(0, i, N);
        MEDDLY_CHECK_RANGE(0, j, N);
        MEDDLY::SWAP(order[i], order[j]);
      }

      inline void createEdge(T &ev, node_handle &ed) {
        createEdgeUn(K, 0, N, ev, ed);
      }

    protected:

      /**
          Recursive implementation of createEdge(),
          unprimed levels, for use by evmxd_forest descendants.
      */
      void createEdgeUn(int k, int start, int stop, T &ev, node_handle &ed) {
        MEDDLY_DCASSERT(k>=0);
        MEDDLY_DCASSERT(stop > start);
        // 
        // Fast special case
        //
        if (1==stop-start) {
          ev = term(start);
          ed = expert_forest::bool_Tencoder::value2handle(true);
          F->createEdgePath<OPERATION, T>(k, unprimed(start), primed(start),
            ev, ed);
          return;
        }
        //
        // Check terminal case
        //
        if (0==k) {
          ev = term(start);
          for (int i=start+1; i<stop; i++) {
            OPERATION::unionEq(ev, term(i));
          }
          ed = expert_forest::bool_Tencoder::value2handle(true);
          return;
        }

        // size of variables at level k
        int lastV = F->getLevelSize(k);
        // index of end of current batch
        int batchP = start;

        //
        // Move any "don't cares" to the front, and process them
        //
        int nextV = lastV;
        for (int i=start; i<stop; i++) {
          if (DONT_CARE == unprimed(i, k)) {
            if (batchP != i) {
              swap(batchP, i);
            }
            batchP++;
          } else {
            MEDDLY_DCASSERT(unprimed(i, k) >= 0);
            nextV = MIN(nextV, unprimed(i, k));
          }
        }

        //
        // Move any "don't changes" below the "don't cares", to the front,
        // and process them to construct a new level-k node.
        int dch = start;
        for (int i=start; i<batchP; i++) {
          if (DONT_CHANGE == primed(i, k)) {
            if (dch != i) {
              swap(dch, i);
            }
            dch++;
          }
        } 

        //
        // Process "don't care, don't change" pairs, if any
        //
        T dc_ev;
        node_handle dc;
        OPERATION::makeEmptyEdge(dc_ev, dc);

        if (dch > start) {
          createEdgeUn(k-1, start, dch, dc_ev, dc);
          makeIdentityEdge(k, dc_ev, dc);
          // done with those
          start = dch;
        }

        //
        // Process "don't care, ordinary" pairs, if any
        // (producing a level-k node)
        //
        if (batchP > start) {
          T dcnormal_ev;
          node_handle dcnormal;
          createEdgePr(-1, -k, start, batchP, dcnormal_ev, dcnormal);
          F->makeNodeAtLevel<OPERATION, T>(k, dcnormal_ev, dcnormal);

          MEDDLY_DCASSERT(unionOp);
          T total_ev;
          node_handle total;
          unionOp->compute(dc_ev, dc, dcnormal_ev, dcnormal, total_ev, total);
          F->unlinkNode(dcnormal);
          F->unlinkNode(dc);
          dc_ev = total_ev;
          dc = total;
        }

        //
        // Start new node at level k
        //
        unpacked_node* nb = unpacked_node::newSparse(F, k, lastV);
        int z = 0; // number of nonzero edges in our sparse node

        //
        // For each value v, 
        //  (1) move those values to the front
        //  (2) process them, if any
        // Then when we are done, union with any don't cares
        //
        for (int v=nextV; v<lastV; v=nextV) {
          nextV = lastV;
          //
          // neat trick!
          // shift the array over, because we're done with the previous batch
          //
          start = batchP;

          //
          // (1) move anything with value v, to the "new" front
          //
          for (int i=start; i<stop; i++) {
            if (v == unprimed(i, k)) {
              if (batchP != i) {
                swap(batchP, i);
              }
              batchP++;
            } else {
              nextV = MIN(nextV, unprimed(i, k));
            }
          }

          //
          // (2) recurse if necessary
          //
          if (0==batchP) continue;
          T these_ev;
          node_handle these_ptr;
          nb->i_ref(z) = v;
          createEdgePr(v, -k, start, batchP, these_ev, these_ptr);
          nb->d_ref(z) = these_ptr;
          nb->setEdge(z, these_ev);
          z++;
        } // for v

        //
        // Union with don't cares
        //
        MEDDLY_DCASSERT(unionOp);
        node_handle built;
        T built_ev;
        nb->shrinkSparse(z);
        F->createReducedNode(-1, nb, built_ev, built);
        unionOp->compute(dc_ev, dc, built_ev, built, ev, ed);
        F->unlinkNode(dc);
        F->unlinkNode(built);
      };

    protected:

      /**
          Recursive implementation of createEdge(),
          primed levels
      */
      void createEdgePr(int in, int k, int start, int stop,
        T &ev, node_handle &ed) {
        MEDDLY_DCASSERT(k<0);
        MEDDLY_DCASSERT(stop > start);

        //
        // Don't need to check for terminals
        //

        // size of variables at level k
        int lastV = F->getLevelSize(k);
        // current batch size
        int batchP = start;

        //
        // Move any "don't cares" to the front, and process them
        //
        int nextV = lastV;
        for (int i=start; i<stop; i++) {
          if (DONT_CARE == primed(i, -k)) {
            if (batchP != i) {
              swap(batchP, i);
            }
            batchP++;
          } else {
            nextV = MIN(nextV, primed(i, -k));
          }
        }

        node_handle dc_ptr;
        T dc_val;
        if (batchP > start) {
          createEdgeUn(-k-1, start, batchP, dc_val, dc_ptr);
        } else {
          OPERATION::makeEmptyEdge(dc_val, dc_ptr);
        }

        //
        // Start new node at level k
        //
        unpacked_node* nb = unpacked_node::newSparse(F, k, lastV);
        int z = 0; // number of nonzero edges in our sparse node

        //
        // For each value v, 
        //  (1) move those values to the front
        //  (2) process them, if any
        //  (3) union with don't cares
        //
        for (int v = (dc_ptr) ? 0 : nextV; 
             v<lastV; 
             v = (dc_ptr) ? v+1 : nextV) 
        {
          nextV = lastV;
          //
          // neat trick!
          // shift the array over, because we're done with the previous batch
          //
          start = batchP;

          //
          // (1) move anything with value v, or don't change if v=in,
          //     to the "new" front
          //
          bool veqin = (v==in);
          for (int i=start; i<stop; i++) {
            if (v == primed(i, -k) || (veqin && DONT_CHANGE==primed(i, -k))) {
              if (batchP != i) {
                swap(batchP, i);
              }
              batchP++;
            } else {
              nextV = MIN(nextV, primed(i, -k));
            }
          }

          //
          // (2) recurse if necessary
          //
          node_handle these_ptr;
          T these_val;
          if (batchP > start) {
            createEdgeUn(-k-1, start, batchP, these_val, these_ptr);
          } else {
            OPERATION::makeEmptyEdge(these_val, these_ptr);
          }

          //
          // (3) union with don't cares
          //
          node_handle total_ptr;
          T total_val;
          if (0 == dc_ptr) {
            total_val = these_val;
            total_ptr = F->linkNode(these_ptr);
          } else if (0 == these_val) {
            total_val = dc_val;
            total_ptr = F->linkNode(dc_ptr);
          } else {
            MEDDLY_DCASSERT(unionOp);
            unionOp->compute(dc_val, dc_ptr, these_val, these_ptr, 
              total_val, total_ptr);
          }
          F->unlinkNode(these_ptr);

          //
          // add to sparse node, unless empty
          //
          if (0==total_ptr) continue;
          nb->i_ref(z) = v;
          nb->d_ref(z) = total_ptr;
          nb->setEdge(z, total_val);
          z++;
        } // for v

        //
        // Cleanup
        //
        F->unlinkNode(dc_ptr);
        nb->shrinkSparse(z);
        F->createReducedNode(in, nb, ev, ed);
      };


      //
      //
      // Helper for createEdge
      //
      inline void makeIdentityEdge(int k, T& pev, node_handle& p) {
        if (F->isIdentityReduced()) return;

        // build an identity node by hand
        int lastV = F->getLevelSize(k);
        unpacked_node* nb = unpacked_node::newFull(F, k, lastV);
        for (int v=0; v<lastV; v++) {
          unpacked_node* nbp = unpacked_node::newSparse(F, -k, 1);
          nbp->i_ref(0) = v;
          nbp->d_ref(0) = F->linkNode(p);
          nbp->setEdge(0, pev);
          node_handle pr;
          T pr_ev;
          F->createReducedNode(v, nbp, pr_ev, pr);
          nb->d_ref(v) = pr;
          nb->setEdge(v, pr_ev);
        } // for v
        F->unlinkNode(p);
        F->createReducedNode(-1, nb, pev, p);
      }



  }; // class evmxd_edgemaker

}; // namespace MEDDLY

#endif
