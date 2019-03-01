
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

// This hasn't been tested very much yet...

#ifndef EVMDD_TIMESREAL_H
#define EVMDD_TIMESREAL_H

#include "evmdd.h"

namespace MEDDLY {
  class evmdd_timesreal;
};


class MEDDLY::evmdd_timesreal : public evmdd_forest {
  public:
    class OP : public float_EVencoder {
      public:
        static inline void setEdge(void* ptr, float v) {
          writeValue(ptr, v);
        }
        static inline bool isIdentityEdge(const void* p) {
          float ev;
          readValue(p, ev);
          return !notClose(ev, 1.0);
        }
        static inline double apply(double a, double b) {
          return a*b;
        }
        static inline void makeEmptyEdge(float &ev, node_handle &ep) {
          ev = 0;
          ep = 0;
        }
        static inline void unionEq(float &a, float b) {
          a += b;
        }
        // bonus
        static inline bool notClose(float a, float b) {
          if (a) {
            double diff = a-b;
            return ABS(diff/a) > 1e-6;
          } else {
            return ABS(b) > 1e-10;
          }
        }
    };

  public:
    evmdd_timesreal(int dsl, domain *d, const policies &p,int* level_reduction_rule=NULL);
    ~evmdd_timesreal();

    virtual void createEdge(float val, dd_edge &e);
    virtual void createEdge(const int* const* vlist, const float* terms, int N, dd_edge &e);
    virtual void createEdgeForVar(int vh, bool vp, const float* terms, dd_edge& a);
    virtual void evaluate(const dd_edge &f, const int* vlist, float &term) const;

    virtual bool areEdgeValuesEqual(const void* eva, const void* evb) const;
    virtual bool isRedundant(const unpacked_node &nb) const;
    virtual bool isIdentityEdge(const unpacked_node &nb, int i) const;

    virtual enumerator::iterator* makeFullIter() const {
      return new evtrmdd_iterator(this);
    }

  protected:
    virtual void normalize(unpacked_node &nb, float& ev) const;
    virtual void showEdgeValue(output &s, const void* edge) const;
    virtual void writeEdgeValue(output &s, const void* edge) const;
    virtual void readEdgeValue(input &s, void* edge);
    virtual const char* codeChars() const;

  protected:
    class evtrmdd_iterator : public enumerator::iterator {
      public:
        evtrmdd_iterator(const expert_forest* F);
        virtual ~evtrmdd_iterator();

        virtual void getValue(float &termVal) const;
        virtual bool start(const dd_edge &e);
        virtual bool next();
      private:
        bool first(int k, node_handle p);

      protected:
        double* acc_evs;  // for accumulating edge values
    };
};

#endif

