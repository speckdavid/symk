
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

#ifndef MTMDD_H
#define MTMDD_H

#include "mt.h"

namespace MEDDLY {
  class mtmdd_forest;
};

class MEDDLY::mtmdd_forest : public mt_forest {
  public:
    mtmdd_forest(int dsl, domain* d, range_type t, const policies &p,int* level_reduction_rule=NULL);

    virtual void swapAdjacentVariables(int level);
    virtual void moveDownVariable(int high, int low);
    virtual void moveUpVariable(int low, int high);

    virtual void dynamicReorderVariables(int top, int bottom);

    virtual enumerator::iterator* makeFullIter() const 
    {
      return new mtmdd_iterator(this);
    }

  protected:
    inline node_handle evaluateRaw(const dd_edge &f, const int* vlist) const {
      node_handle p = f.getNode();
      while (!isTerminalNode(p)) {
    	int level = getNodeLevel(p);
        p = getDownPtr(p, vlist[getVarByLevel(level)]);
      }
      return p;
    }

    // Move the variable to the optimal level between top and bottom
    void sifting(int var, int top, int bottom);

  protected:
    class mtmdd_iterator : public mt_iterator {
      public:
        mtmdd_iterator(const expert_forest* F);
        virtual ~mtmdd_iterator();
        virtual bool start(const dd_edge &e);
        virtual bool next();
      private:
        bool first(int k, node_handle p);
    };
};


//
// Helper class for createEdge
//

namespace MEDDLY {

  template <class ENCODER, typename T>
  class mtmdd_edgemaker {
      mtmdd_forest* F;
      const int* const* vlist;
      const T* values;
      int* order;
      int N;
      int K;
      binary_operation* unionOp;
    public:
      mtmdd_edgemaker(mtmdd_forest* f, const int* const* mt, const T* v, 
        int* o, int n, int k, binary_operation* unOp) 
      {
        F = f;
        vlist = mt;
        values = v;
        order = o;
        N = n;
        K = k;
        unionOp = unOp;
      }

      inline const int* unprimed(int i) const {
        MEDDLY_CHECK_RANGE(0, i, N);
        return vlist[order[i]];
      }
      inline int unprimed(int i, int k) const {
        MEDDLY_CHECK_RANGE(0, i, N);
        MEDDLY_CHECK_RANGE(1, k, K+1);
        return vlist[order[i]][k];
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
        return createEdge(K, 0, N);
      }

      /**
          Recursive implementation of createEdge(),
          for use by mtmdd_forest descendants.
      */
      node_handle createEdge(int k, int start, int stop) {
        MEDDLY_DCASSERT(k>=0);
        MEDDLY_DCASSERT(stop > start);
        // 
        // Fast special case
        //
        if (1==stop-start) {
          return createEdgePath(k, unprimed(start),
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

        node_handle dontcares = (batchP > start) ? createEdge(k-1, start, batchP) : 0;
        if(dontcares && F->isQuasiReduced()){
          // Add the redundant node at level k
          unpacked_node* nb = unpacked_node::newFull(F, k, lastV);
          for (int v = 0; v<lastV; v++) {
        	  nb->d_ref(v)=F->linkNode(dontcares);
          }
          node_handle built=F->createReducedNode(-1, nb);
          F->unlinkNode(dontcares);
          dontcares=built;
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
		    if(F->isQuasiReduced() && F->getTransparentNode()!=ENCODER::value2handle(0)){
		      // TODO: Can be simpler
		      node_handle zero=makeOpaqueZeroNodeAtLevel(k-1);

		      while(z<nextV) {
			      nb->i_ref(z)=z;
			      nb->d_ref(z)=F->linkNode(zero);
			      z++;
		      }
		      int v = nextV;
		      while(v<lastV) {
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
			      MEDDLY_DCASSERT(batchP > start);
			      node_handle total = createEdge(k-1, start, batchP);

			      //
			      // add to sparse node, unless transparent
			      //
			      if (total!=F->getTransparentNode()) {
			        nb->i_ref(z) = v;
			        nb->d_ref(z) = total;
			        z++;
			      }

			      v++;
			      while(v<nextV) {
			        nb->i_ref(z)=v;
			        nb->d_ref(z)=F->linkNode(zero);
			        z++;
			        v++;
			      }
		      }

		      F->unlinkNode(zero);
		    }
		    else{
		      for (int v = nextV; v<lastV; v = nextV) {
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
			    MEDDLY_DCASSERT(batchP > start);
			    node_handle total = createEdge(k-1, start, batchP);

			    //
			    // add to sparse node, unless transparent
			    //
			    if (total==F->getTransparentNode()) continue;
			    nb->i_ref(z) = v;
			    nb->d_ref(z) = total;
			    z++;
		      }
		    }

        //
        // Cleanup
        //
        nb->shrinkSparse(z);
        node_handle built=F->createReducedNode(-1, nb);

        MEDDLY_DCASSERT(unionOp);
        node_handle total=unionOp->compute(dontcares, built);

        F->unlinkNode(dontcares);
        F->unlinkNode(built);

        return total;
      }

    protected:
      /// Special case for createEdge(), with only one minterm.
      inline node_handle
      createEdgePath(int k, const int* vlist, node_handle bottom)
      {
          if (bottom==0 && (!F->isQuasiReduced() || F->getTransparentNode()==ENCODER::value2handle(0))) {
            return bottom;
          }

          for (int i=1; i<=k; i++) {
            if (DONT_CARE == vlist[i]) {
              // make a redundant node
              if (F->isFullyReduced()) continue;
              int sz = F->getLevelSize(i);
              unpacked_node* nb = unpacked_node::newFull(F, i, sz);
              nb->d_ref(0) = bottom;
              for (int v=1; v<sz; v++) {
                nb->d_ref(v) = F->linkNode(bottom);
              }
              bottom = F->createReducedNode(-1, nb);
            } else {
              if(F->isQuasiReduced() && F->getTransparentNode()!=ENCODER::value2handle(0)){
                int sz = F->getLevelSize(i);
                unpacked_node* nb = unpacked_node::newFull(F, i, sz);
                node_handle zero=makeOpaqueZeroNodeAtLevel(i-1);
                // add opaque zero nodes
                for(int v=0; v<sz; v++) {
                  nb->d_ref(v)=(v==vlist[i] ? bottom : F->linkNode(zero));
                }
                F->unlinkNode(zero);
                bottom=F->createReducedNode(-1, nb);
              }
              else{
                // make a singleton node
                unpacked_node* nb = unpacked_node::newSparse(F, i, 1);
                nb->i_ref(0) = vlist[i];
                nb->d_ref(0) = bottom;
                bottom = F->createReducedNode(-1, nb);
              }
            }
          } // for i
          return bottom;
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
  }; // class mtmdd_edgemaker

}; // namespace MEDDLY

#endif
