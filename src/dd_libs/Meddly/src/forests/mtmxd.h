
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

#ifndef MTMXD_H
#define MTMXD_H

#include "mt.h"

namespace MEDDLY {
  class mtmxd_forest;
};

class MEDDLY::mtmxd_forest : public mt_forest {
  public:
    mtmxd_forest(int dsl, domain* d, range_type t, const policies &p,int* level_reduction_rule=NULL);

    virtual void swapAdjacentVariables(int level);
    virtual void moveDownVariable(int high, int low);
    virtual void moveUpVariable(int low, int high);

    virtual void dynamicReorderVariables(int top, int bottom);
    void sifting(int var, int top, int bottom);

    virtual enumerator::iterator* makeFullIter() const 
    {
      return new mtmxd_iterator(this);
    }

    virtual enumerator::iterator* makeFixedRowIter() const 
    {
      return new mtmxd_fixedrow_iter(this);
    }

    virtual enumerator::iterator* makeFixedColumnIter() const 
    {
      return new mtmxd_fixedcol_iter(this);
    }

  protected:
      inline node_handle evaluateRaw(const dd_edge &f, const int* vlist, 
        const int* vplist) const
      {
        node_handle p = f.getNode();
        while (!isTerminalNode(p)) {
          int k = getNodeLevel(p);
          int i = (k<0) ? vplist[-k] : vlist[k];
          p = getDownPtr(p, i);
        } 
        return p;
      }

  protected:
    class mtmxd_iterator : public mt_iterator {
      public:
        mtmxd_iterator(const expert_forest* F);
        virtual ~mtmxd_iterator();
        virtual bool start(const dd_edge &e);
        virtual bool next();
      private:
        bool first(int k, node_handle p);
    };

    class mtmxd_fixedrow_iter : public mt_iterator {
      public:
        mtmxd_fixedrow_iter(const expert_forest* F);
        virtual ~mtmxd_fixedrow_iter();
        virtual bool start(const dd_edge &e, const int*);
        virtual bool next();
      private:
        bool first(int k, node_handle p);
    };

    class mtmxd_fixedcol_iter : public mt_iterator {
      public:
        mtmxd_fixedcol_iter(const expert_forest* F);
        virtual ~mtmxd_fixedcol_iter();
        virtual bool start(const dd_edge &e, const int*);
        virtual bool next();
      private:
        bool first(int k, node_handle p);
    };

    void swapAdjacentVariablesByVarSwap(int level);
    /** Return the root node after swapping the adjacent variables
        in the MxD with the given root node.
    */
    node_handle swapAdjacentVariablesOf(node_handle node);

    void swapAdjacentVariablesByLevelSwap(int level);
    void swapAdjacentLevels(int level);
};

//
// Helper class for createEdge
//

namespace MEDDLY {

  template <class ENCODER, typename T>
  class mtmxd_edgemaker {
      mtmxd_forest* F;
      const int* const* vulist;
      const int* const* vplist;
      const T* values;
      int* order;
      int N;
      int K;
      binary_operation* unionOp;
    public:
      mtmxd_edgemaker(mtmxd_forest* f, 
        const int* const* mt, const int* const* mp, const T* v, int* o, int n, 
        int k, binary_operation* unOp) 
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

      inline node_handle createEdge() {
        return createEdgeUn(K, 0, N);
      }

      /**
          Recursive implementation of createEdge(),
          unprimed levels, for use by mtmxd_forest descendants.
      */
      node_handle createEdgeUn(int k, int start, int stop) {
        MEDDLY_DCASSERT(k>=0);
        MEDDLY_DCASSERT(stop > start);
        // 
        // Fast special case
        //
        if (1==stop-start) {
          return createEdgePath(k, unprimed(start), primed(start),
            ENCODER::value2handle(term(start))
          );
        }
        //
        // Check terminal case
        //
        if (0==k) {
          T accumulate = term(start);
          for (int i=start+1; i<stop; i++) {
            accumulate += term(i);
          }
          return ENCODER::value2handle(accumulate);
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
        node_handle dontcares = 0;

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
        if (dch > start) {
          node_handle below = createEdgeUn(k-1, start, dch);
          dontcares = makeIdentityEdgeForDontCareDontChange(k, below);
          // done with those
          start = dch;
        }

        //
        // Process "don't care, ordinary" pairs, if any
        // (producing a level-k node)
        //
        if (batchP > start) {
          node_handle dcnormal = F->makeNodeAtLevel(
              k, createEdgePr(-1, -k, start, batchP)
          );
          MEDDLY_DCASSERT(unionOp);
          node_handle total = unionOp->compute(dontcares, dcnormal);
          F->unlinkNode(dcnormal);
          F->unlinkNode(dontcares);
          dontcares = total;
        }

        //
        // Start new node at level k
        //
        unpacked_node* nb = unpacked_node::newSparse(F, k, lastV);
        int z = 0; // number of opaque edges in our sparse node

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

          node_handle total=createEdgePr(v, F->downLevel(k), start, batchP);
          if(total!=F->getTransparentNode()){
        	  nb->i_ref(z) = v;
            nb->d_ref(z) =total;
            z++;
          }
        }

        //
        // Union with don't cares
        //

        nb->shrinkSparse(z);
        node_handle built = F->createReducedNode(-1, nb);

        MEDDLY_DCASSERT(unionOp);
        node_handle total = unionOp->compute(dontcares, built);

        F->unlinkNode(dontcares);
        F->unlinkNode(built);

        return total; 
      };

    protected:

      /**
          Recursive implementation of createEdge(),
          primed levels
      */
      node_handle createEdgePr(int in, int k, int start, int stop) {
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

        node_handle dontcares = (batchP > start) ? createEdgeUn(F->downLevel(k), start, batchP) : 0;

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
        for (int v = (dontcares) ? 0 : nextV; v<lastV; v = (dontcares) ? v+1 : nextV) {
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
          node_handle these;
          if (batchP > start) {
            these = createEdgeUn(F->downLevel(k), start, batchP);
          } else {
            these = 0;
          }

          //
          // (3) union with don't cares
          //
          MEDDLY_DCASSERT(unionOp);
          node_handle total = unionOp->compute(dontcares, these);
          F->unlinkNode(these);

          //
          // add to sparse node, unless transparent
          //
          if (total!=F->getTransparentNode()){
            nb->i_ref(z) = v;
            nb->d_ref(z) = total;
            z++;
          }
        } // for v

        //
        // Cleanup
        //
        F->unlinkNode(dontcares);
        nb->shrinkSparse(z);
        return F->createReducedNode(in, nb);
      };

      //
      //
      // Helper for createEdge for (DONT_CARE -> DONT_CHANGE)
      //
      node_handle makeIdentityEdgeForDontCareDontChange(int k, node_handle p) {
        if (F->isIdentityReduced()) {
          return p;
        }
//        if(p==F->getTransparentNode()){
//        	return p;
//        }
        // build an identity node by hand
        int lastV = F->getLevelSize(k);
        unpacked_node* nb = unpacked_node::newFull(F, k, lastV);
        for (int v=0; v<lastV; v++) {
          unpacked_node* nbp = unpacked_node::newSparse(F, -k, 1);
          nbp->i_ref(0) = v;
          nbp->d_ref(0) = F->linkNode(p);
          nb->d_ref(v) = F->createReducedNode(v, nbp);
        }
        F->unlinkNode(p);
        return F->createReducedNode(-1, nb);
      }

      /// Special case for createEdge(), with only one minterm.
      node_handle createEdgePath(int k, const int* vlist, const int* vplist, node_handle next)
      {
          MEDDLY_DCASSERT(F->isForRelations());

          if (0==next && (!F->isQuasiReduced() || F->getTransparentNode()==ENCODER::value2handle(0))) {
        	return next;
          }

          for (int i=1; i<=k; i++) {
            //
            // process primed level
            //
            node_handle nextpr;
            if (DONT_CARE == vplist[i]) {
              if (F->isFullyReduced()) {
                // DO NOTHING
                nextpr = next;
              } else {
                // build redundant node
                int sz = F->getLevelSize(-i);
                unpacked_node* nb = unpacked_node::newFull(F, -i, sz);
                for (int v=0; v<sz; v++) {
                  nb->d_ref(v) = F->linkNode(next);
                }
                F->unlinkNode(next);
                nextpr = F->createReducedNode(-1, nb);
              }
            }
            else if (DONT_CHANGE == vplist[i]) {
                //
                // Identity node
                //
                if(DONT_CARE == vlist[i]){
                  if (F->isIdentityReduced()) continue;
                	next = makeIdentityEdgeForDontCareDontChange(i, next);
                	continue;
                }

                MEDDLY_DCASSERT(vlist[i]>=0);

                if (F->isIdentityReduced()) {
                	// DO NOTHING
                	nextpr = next;
                }
                else if(F->isQuasiReduced() && F->getTransparentNode()!=ENCODER::value2handle(0)){
                	int sz = F->getLevelSize(-i);
                  unpacked_node* nbp = unpacked_node::newFull(F, -i, sz);
					        node_handle zero=makeOpaqueZeroNodeAtLevel(i-1);
					        for(int v=0; v<sz; v++){
					          nbp->d_ref(v)=(v==vlist[i] ? F->linkNode(next) : F->linkNode(zero));
					        }
					        F->unlinkNode(zero);

					        nextpr = F->createReducedNode(vlist[i], nbp);
                }
                else {
                  unpacked_node* nbp = unpacked_node::newSparse(F, -i, 1);
                	nbp->i_ref(0) = vlist[i];
                	nbp->d_ref(0) = next;
                  // link count should be unchanged

                	nextpr = F->createReducedNode(vlist[i], nbp);
                }
            }
            else {
              // sane value
              if(F->isQuasiReduced() && F->getTransparentNode()!=ENCODER::value2handle(0)){
            	int sz = F->getLevelSize(-i);
                unpacked_node* nbp = unpacked_node::newFull(F, -i, sz);
                node_handle zero=makeOpaqueZeroNodeAtLevel(i-1);
                for(int v=0; v<sz; v++){
                  nbp->d_ref(v)=(v==vplist[i] ? F->linkNode(next) : F->linkNode(zero));
                }
                F->unlinkNode(zero);

                nextpr = F->createReducedNode(vlist[i], nbp);
              }
              else {
                unpacked_node* nbp = unpacked_node::newSparse(F, -i, 1);
                nbp->i_ref(0) = vplist[i];
                nbp->d_ref(0) = next;
                // link count should be unchanged

                nextpr = F->createReducedNode(vlist[i], nbp);
              }
            }

            //
            // process unprimed level
            //
            if (DONT_CARE == vlist[i]) {
              if (F->isFullyReduced()) { 
            	  next=nextpr;
            	  continue;
              }
              // build redundant node
              int sz = F->getLevelSize(i);
              unpacked_node* nb = unpacked_node::newFull(F, i, sz);
              if (F->isIdentityReduced()) {
                // Below is likely a singleton, so check for identity reduction
                // on the appropriate v value
                for (int v=0; v<sz; v++) {
//                  node_handle dpr = (v == vplist[i]) ? next : nextpr;
                  nb->d_ref(v) = F->linkNode(v == vplist[i] ? next : nextpr);
                }
              } else {
                // Doesn't matter what happened below
                for (int v=0; v<sz; v++) {
                  nb->d_ref(v) = F->linkNode(nextpr);
                }
              }
              F->unlinkNode(nextpr);
              next = F->createReducedNode(-1, nb);
            } else {
              // sane value
              if(F->isQuasiReduced() && F->getTransparentNode()!=ENCODER::value2handle(0)){
            	int sz=F->getLevelSize(i);
              unpacked_node* nb = unpacked_node::newFull(F, i, sz);
            	node_handle zero = makeOpaqueZeroNodeAtLevel(-i);
            	for(int v=0; v<sz; v++){
            	  nb->d_ref(v) = F->linkNode(v==vlist[i] ? nextpr : zero);
            	}
            	F->unlinkNode(zero);

            	next = F->createReducedNode(-1, nb);
              }
              else {
                unpacked_node* nb = unpacked_node::newSparse(F, i, 1);
                nb->i_ref(0) = vlist[i];
                nb->d_ref(0) = nextpr;
                // link count should be unchanged

                next = F->createReducedNode(-1, nb);
              }
            }
          }

          return next;
      }

      // Make zero nodes recursively when:
      // 1. quasi reduction
      // 2. the transparent value is not zero
      node_handle makeOpaqueZeroNodeAtLevel(int k)
      {
  	    MEDDLY_DCASSERT(F->isQuasiReduced());
  	    MEDDLY_DCASSERT(F->getTransparentNode()!=ENCODER::value2handle(0));

  	    return F->makeNodeAtLevel(k, ENCODER::value2handle(0));
      }

  }; // class mtmxd_edgemaker

};  // namespace MEDDLY

#endif

