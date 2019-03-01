
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

#ifndef EVMDD_PLUSINT_H
#define EVMDD_PLUSINT_H

#include "evmdd.h"

namespace MEDDLY {
  class evmdd_plusint;
  class evmdd_index_set;
};


class MEDDLY::evmdd_plusint : public evmdd_forest {
  public:
    class OP : public int_EVencoder {
      public:
        static inline void setEdge(void* ptr, int v) {
          writeValue(ptr, v);
        }
        static inline bool isIdentityEdge(const void* p) {
          int ev;
          readValue(p, ev);
          return (0==ev);
        }
        static inline int getRedundantEdge() {
          return 0;
        }
        static inline int apply(int a, int b) {
          return a+b;
        }
        static inline void makeEmptyEdge(int &ev, node_handle &ep) {
          ev = 0;
          ep = 0;
        }
        static inline void unionEq(int &a, int b) {
          if (b<a) a = b;
        }
    };

  public:
    evmdd_plusint(int dsl, domain *d, const policies &p,int* level_reduction_rule=NULL,bool index_set=false);
    ~evmdd_plusint();

    virtual void createEdge(int val, dd_edge &e);
    virtual void createEdge(const int* const* vlist, const int* terms, int N, dd_edge &e);
    virtual void createEdgeForVar(int vh, bool vp, const int* terms, dd_edge& a);
    virtual void evaluate(const dd_edge &f, const int* vlist, int &term) const;

    virtual bool areEdgeValuesEqual(const void* eva, const void* evb) const;
    virtual bool isRedundant(const unpacked_node &nb) const;
    virtual bool isIdentityEdge(const unpacked_node &nb, int i) const;

    virtual enumerator::iterator* makeFullIter() const {
      return new evpimdd_iterator(this);
    }

    virtual void swapAdjacentVariables(int level);

  protected:
    virtual void normalize(unpacked_node &nb, int& ev) const;
    virtual void showEdgeValue(output &s, const void* edge) const;
    virtual void writeEdgeValue(output &s, const void* edge) const;
    virtual void readEdgeValue(input &s, void* edge);
    virtual void showUnhashedHeader(output &s, const void* uh) const;
    virtual void writeUnhashedHeader(output &s, const void* uh) const;
    virtual void readUnhashedHeader(input &s, unpacked_node &nb) const;
    virtual const char* codeChars() const;

  protected:
    class evpimdd_iterator : public enumerator::iterator {
      public:
        evpimdd_iterator(const expert_forest* F);
        virtual ~evpimdd_iterator();

        virtual void getValue(int &termVal) const;
        virtual bool start(const dd_edge &e);
        virtual bool next();
      private:
        bool first(int k, node_handle p);

      protected:
        long* acc_evs;  // for accumulating edge values
    };
};


class MEDDLY::evmdd_index_set : public evmdd_plusint {
  public:
    evmdd_index_set(int dsl, domain *d, const policies &p);
    ~evmdd_index_set();

    virtual void getElement(const dd_edge& a, int index, int* e);
};

#endif

