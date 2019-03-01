
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
  State space generation for a 3x3 Rubik's Cube
  -- Junaid Babar

	The model has 6 square faces with 9 squares (of the same color) in each face
	for a total of 54 squares.
  
	These 54 squares can be grouped together as some of them move together.
  Each Corner is a group of 3 squares. There are 8 corners.
  Each Edge is a group of 2 squares. There are 12 edges.
  The center's of each face has a single-squared component that does not move.

  In addition to these components our model also tracks the orientation of
  each component.
  Each Corner has 3 different orientations.
  Each Edge has 2 different orientations.

  Our model therefore has (8 + 8 + 12 + 12 + 6 = ) 46 variables.

  We have two choices for the meaning of each variable:
  1: Each variable tells us the location of a component.
  2: Each variables tells the component at a location.

  There are three kinds of moves (events): rotate a face clockwise by
  90, 180 and 270 degrees.
  
  Using Scheme 1 for representing the variables, each move will span all
  the variables in the MDD. Scheme 2, on the other hand will only span the
  variables effected by a move (i.e. 17 variables vs 46). We therefore
  use Scheme 2 for this model.
  
  The locations are named as follows:

  (starting with the left top corner of the face and going clockwise)
  Front:  C1, E1, C2, E2, C3, E3, C4, E4.
  Left:   C5, E12, C1, E4, C4, E11, C8, E8.
  Right:  C2, E9, C6, E6, C7, E10, C3, E2.
  Up:     C5, E5, C6, E9, C2, E1, C1, E12.
  Down:   C4, E3, C3, E10, C7, E7, C8, E11.
  Back:   C6, E5, C5, E8, C8, E7, C7, E6.

	Initially components are placed in components locations that match their
	Ids.
*/


#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "meddly.h"
#include "meddly_expert.h"
#include "timer.h"

#define DEBUG

using namespace MEDDLY;

typedef enum {F, B, L, R, U, D} face;
typedef enum {CW, CCW, FLIP} direction;


class moves {
  public:

    // ?CW, ?CCW, ?F stand for clock-wise, counter clock-wise and flip resp.

    bool FCW;
    bool FCCW;
    bool FF;
    bool BCW;
    bool BCCW;
    bool BF;
    bool LCW;
    bool LCCW;
    bool LF;
    bool RCW;
    bool RCCW;
    bool RF;
    bool UCW;
    bool UCCW;
    bool UF;
    bool DCW;
    bool DCCW;
    bool DF;

    moves()
      : FCW(false), FCCW(false), FF(false), BCW(false), BCCW(false),
      BF(false), LCW(false), LCCW(false), LF(false), RCW(false),
      RCCW(false), RF(false), UCW(false), UCCW(false), UF(false),
      DCW(false), DCCW(false), DF(false) {}

    int countEnabledMoves() const {
      int count = 0;
      if (FCW) count++;
      if (BCW) count++;
      if (UCW) count++;
      if (DCW) count++;
      if (LCW) count++;
      if (RCW) count++;
      if (FCCW) count++;
      if (BCCW) count++;
      if (UCCW) count++;
      if (DCCW) count++;
      if (LCCW) count++;
      if (RCCW) count++;
      if (FF) count++;
      if (BF) count++;
      if (UF) count++;
      if (DF) count++;
      if (LF) count++;
      if (RF) count++;
      return count;
    }
};



class rubiks {

  protected:

    // order[variable]: position of the variable in the MDD.
    // 0 is for terminal nodes.
    int* order;

    // Number of enabled levels.
    int nLevels;

    // Variables:
    // 1-8:   Corners
    // 2-16:  Corner Orientations
    // 17-28: Edges
    // 29-40: Edge Orientations
    // 41-46: Centers

    // Get the ith corner variable
    int c(int i) const { return i; }

    int coOffset;
    // Get the ith corner orientation variable
    int co(int i) const { 
      return i + coOffset;
    }

    int eOffset;
    // Get the ith edge variable
    int e(int i) const { return i + eOffset; }

    int eoOffset;
    // Get the ith edge orientation variable
    int eo(int i) const { return i + eoOffset; }

    int centerOffset;
    // Get the ith center variable
    int center(int i) const { return i + centerOffset; }

    // Sizes of variables (i.e. locations)
    int* variableSize;
    /*
       = {
       0,
       8, 8, 8, 8, 8, 8, 8, 8, 
       3, 3, 3, 3, 3, 3, 3, 3, 
       12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 
       2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
       4, 4, 4, 4, 4, 4
       };
       */

    bool enableCorners;
    bool enableCornerOrientations;
    bool enableEdges;
    bool enableEdgeOrientations;
    bool enableCenterOrientations;

    // Domain handle
    domain *d;

    // Forest storing the next state function
    // forest_hndl mxd;
    forest* mxd;
    forest* mtmxd;

    // Forest storing the set of states
    // forest_hndl mdd;
    forest* mdd;

  public:
    // The maximum possible levels. This is the same as nLevels when
    // all the locations are enabled.
    static const int nLocations = (8 + 8 + 12 + 12 + 6);

    // Default variable ordering:
    static const int defaultVariableOrdering[nLocations+1];

    // Ben Smith's SMART file ordering:
    static const int BenSmithsVariableOrdering[nLocations+1];

    static const int plus0mod2[2];
    static const int plus1mod2[2];
    static const int plus0mod3[3];
    static const int plus1mod3[3];
    static const int plus2mod3[3];
    static const int plus0mod4[4];
    static const int plus1mod4[4];
    static const int plus2mod4[4];
    static const int plus3mod4[4];
    static const int plus0mod8[8];
    static const int plus0mod12[12];

    ~rubiks() {
      // Clean up forests and domain.
      delete[] order;
      delete[] variableSize;

      MEDDLY::destroyDomain(d);
      //MEDDLY::cleanup();
    }

    // Returns an ordering based on the full-order with restrictions.
    void buildOrdering(const int* fullOrder) {
      assert(fullOrder);
      assert(order == 0);
      assert(variableSize == 0);

      coOffset = eOffset = eoOffset = centerOffset = 0;
      nLevels = 0;
      if (enableCorners) {
        nLevels += 8;
        coOffset = nLevels;
        if (enableCornerOrientations) nLevels += 8;
      }
      eOffset = nLevels;
      if (enableEdges) {
        nLevels += 12;
        eoOffset = nLevels;
        if (enableEdgeOrientations) nLevels += 12;
      }
      centerOffset = nLevels;
      if (enableCenterOrientations) nLevels += 6;

      assert(nLevels > 0);

      // Fill in order[] and variableSize[] from fullOrder[].
      // Skip disabled locations.
      // Use nSkipped to renumber the variables in fullOrder.

      int sz = nLevels + 1;
      order = new int[sz];
      variableSize = new int[sz];

      int* orderPtr = order;
      const int* fullOrderPtr = fullOrder;
      int* vSizePtr = variableSize;

      *orderPtr++ = *fullOrderPtr++;
      *vSizePtr++ = 0;

      int nSkipped = 0;

      if (enableCorners) {
        for (int i = 0; i < 8; i++) {
          *orderPtr++ = *fullOrderPtr++;
          *vSizePtr++ = 8;
        }
        if (enableCornerOrientations) {
          for (int i = 0; i < 8; i++) {
            *orderPtr++ = *fullOrderPtr++;
            *vSizePtr++ = 3;
          }
        }
        else {
          fullOrderPtr += 8;
          nSkipped += 8;
        }
      } else {
        fullOrderPtr += 16;
        nSkipped += 16;
      }

      if (enableEdges) {
        for (int i = 0; i < 12; i++) {
          *orderPtr++ = (*fullOrderPtr++) - nSkipped;
          *vSizePtr++ = 12;
        }
        if (enableEdgeOrientations) {
          for (int i = 0; i < 12; i++) {
            *orderPtr++ = (*fullOrderPtr++) - nSkipped;
            *vSizePtr++ = 2;
          }
        }
        else {
          fullOrderPtr += 12;
          nSkipped += 12;
        }
      } else {
        fullOrderPtr += 24;
        nSkipped += 24;
      }

      if (enableCenterOrientations) {
        for (int i = 0; i < 6; i++) {
          *orderPtr++ = (*fullOrderPtr++) - nSkipped;
          *vSizePtr++ = 2;
        }
      } else {
        fullOrderPtr += 6;
        nSkipped += 6;
      }

      assert(fullOrderPtr == (fullOrder + (nLocations + 1)));
      assert(orderPtr == (order + sz));
      assert(vSizePtr == (variableSize + sz));

#ifdef DEBUG
      printf("\nfullOrder[]:\n\t");
      for (int i = 0; i < (nLocations+1); i++) printf("%d ", fullOrder[i]);
      printf("\norder[]:\n\t");
      for (int i = 0; i < sz; i++) printf("%d ", order[i]);
      printf("\n");
#endif
    }


    // Constructor
    // variableOrdering: the full variable ordering must include
    //                    all the locations in the model.
    // voSize: size of variableOrdering. Must be equal to nLocations + 1.
    // enable components and their orientations.
    rubiks(const int* variableOrdering, int voSize,
        bool enableCorners, bool enableCornerOrientations,
        bool enableEdges, bool enableEdgeOrientations,
        bool enableCenterOrientations) {

      assert(variableOrdering);
      assert(voSize == (nLocations + 1));

      this->enableCorners = enableCorners;
      this->enableCornerOrientations = enableCornerOrientations;
      this->enableEdges = enableEdges;
      this->enableEdgeOrientations = enableEdgeOrientations;
      this->enableCenterOrientations = enableCenterOrientations;

      order = 0;
      variableSize = 0;
      nLevels = 0;
      d = 0;
      mxd = 0;
      mtmxd = 0;
      mdd = 0;

      // Build the restricted variable order (order[]) and variableSize[].
      buildOrdering(variableOrdering);

      // Initialize MEDDLY
      initializer_list* L = defaultInitializerList(0);
      ct_initializer::setBuiltinStyle(ct_initializer::OperationChainedHash);
      // ct_initializer::setBuiltinStyle(ct_initializer::MonolithicChainedHash);
      ct_initializer::setMaxSize(8 * 16777216);
      // ct_initializer::setStaleRemoval(ct_initializer::Lazy);
      // ct_initializer::setStaleRemoval(ct_initializer::Moderate);
      // ct_initializer::setStaleRemoval(ct_initializer::Aggressive);
      MEDDLY::initialize(L);

      // Set up the state variables, as described earlier
      d = createDomainBottomUp(&variableSize[1], nLevels);
      if (0 == d) {
        fprintf(stderr, "Couldn't create domain\n");
        assert(false);
      }

      printf("Variable Order:\n");
      for (int i = d->getNumVariables(); i > 0; i--) {
        printf("level %d, location %d, size %d\n", i, order[i],
            static_cast<expert_domain*>(d)->getVariableBound(i));
      }

      // Create forests
      mdd = d->createForest(false, forest::BOOLEAN, forest::MULTI_TERMINAL);
      mxd = d->createForest(true, forest::BOOLEAN, forest::MULTI_TERMINAL);
      mtmxd = d->createForest(true, forest::INTEGER, forest::MULTI_TERMINAL);

      if (0 == mdd) {
        fprintf(stderr, "Couldn't create forest for states\n");
        assert(false);
      } else {
        fprintf(stdout, "Created forest for states %p\n", mdd);
      }
      if (0 == mxd) {
        fprintf(stderr, "Couldn't create forest for relations\n");
        assert(false);
      } else {
        fprintf(stdout, "Created forest for relations %p\n", mxd);
      }
      if (0 == mtmxd) {
        fprintf(stderr, "Couldn't create forest for mtmxd\n");
        assert(false);
      } else {
        fprintf(stdout, "Created forest for mtmxd %p\n", mtmxd);
      }
    }

    dd_edge buildInitialState() {
      assert(mdd);

      // sets of states
      int* initst = new int[nLevels + 1];
      assert(initst);

      // The initial state is the "solved" Rubik's Cube.
      // Therefore each component i is at location i,
      // and each component's orientation is 0 (i.e. the solved orientation).
      if (enableCorners) {
        for (int i = 0; i < 8; i++) initst[order[c(i+1)]] = i;
        if (enableCornerOrientations) {
          for (int i = 0; i < 8; i++) initst[order[co(i+1)]] = 0;
        }
      }
      if (enableEdges) {
        for (int i = 0; i < 12; i++) initst[order[e(i+1)]] = i;
        if (enableEdgeOrientations) {
          for (int i = 0; i < 12; i++) initst[order[eo(i+1)]] = 0;
        }
      }
      if (enableCenterOrientations) {
        for (int i = 0; i < 6; i++) initst[order[center(i+1)]] = i;
      }

      dd_edge initialStates(mdd);
      mdd->createEdge((int**)(&initst), 1, initialStates);

      delete[] initst;

      return initialStates;
    }


    void SetIntArray(int p[], const int p_size, const int c)
    {
      for (int i=0; i<p_size; i++) p[i] = c;
    }

    void PrintIntArray(int p[], const int p_size)
    {
      printf("[");
      for (int i=0; i<p_size; i++) printf("%d ", p[i]);
      printf("]\n");
    }


    const int* getModArray(int var, int offset) const {
      const int* modArray = 0;
      offset = offset % variableSize[var];
      switch (variableSize[var]) {
        case 2: modArray = (offset == 0)? plus0mod2: plus1mod2;
                break;
        case 3: modArray = (offset == 0)
                ? plus0mod3
                  : (offset == 1)
                  ? plus1mod3
                  : plus2mod3;
                break;
        case 4: modArray = (offset == 0)
                ? plus0mod4
                  : (offset == 1)
                  ? plus1mod4
                  : (offset == 2)
                  ? plus2mod4
                  : plus3mod4;
                break;
        case 8: assert(offset == 0);
                modArray = plus0mod8;
                break;
        case 12: assert(offset == 0);
                 modArray = plus0mod12;
                 break;
      }
      return modArray;
    }


    // order[x1] and order[x2] represent the actual variables.
    dd_edge buildPair(int x1, int x2, int offset = 0) {
      // Build ((x1 + offset) % varsize(x1)) == x2'

#ifdef DEBUG
      printf("building pair: %d %d\n", x1, x2);
      fflush(stdout);
#endif

      dd_edge r1(mtmxd);
      dd_edge r2(mtmxd);
      dd_edge r(mxd);
      const int* modArray = getModArray(x1, offset);
      mtmxd->createEdgeForVar(order[x1], false, modArray, r1);
      mtmxd->createEdgeForVar(order[x2], true, r2);
      apply(EQUAL, r1, r2, r);
      return r;
    }

    dd_edge buildRelevantVariables(
        int c1, int c2, int c3, int c4,
        int e1, int e2, int e3, int e4,
        int co1, int co2, int co3, int co4,
        int eo1, int eo2, int eo3, int eo4,
        int centero) {
#ifdef DEBUG
      printf("locations: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
          c1, c2, c3, c4, e1, e2, e3, e4,
          co1, co2, co3, co4, eo1, eo2, eo3, eo4, centero);
      fflush(stdout);
#endif

      int* from = (int *)malloc((1+nLevels) * sizeof(int));
      int* to = (int *)malloc((1+nLevels) * sizeof(int));
      for (int i = 1; i <= nLevels; i++) {
        from[i] = DONT_CARE;
        to[i] = DONT_CHANGE;
      }
      if (enableCorners) {
        to[order[c1]] = DONT_CARE;
        to[order[c2]] = DONT_CARE;
        to[order[c3]] = DONT_CARE;
        to[order[c4]] = DONT_CARE;
        if (enableCornerOrientations) {
          to[order[co1]] = DONT_CARE;
          to[order[co2]] = DONT_CARE;
          to[order[co3]] = DONT_CARE;
          to[order[co4]] = DONT_CARE;
        }
      }
      if (enableEdges) {
        to[order[e1]] = DONT_CARE;
        to[order[e2]] = DONT_CARE;
        to[order[e3]] = DONT_CARE;
        to[order[e4]] = DONT_CARE;
        if (enableEdgeOrientations) {
          to[order[eo1]] = DONT_CARE;
          to[order[eo2]] = DONT_CARE;
          to[order[eo3]] = DONT_CARE;
          to[order[eo4]] = DONT_CARE;
        }
      }
      if (enableCenterOrientations) {
        to[order[centero]] = DONT_CARE;
      }
      dd_edge r(mxd);
      mxd->createEdge((int**)(&from), (int**)(&to), 1, r);
      free(from);
      free(to);

      return r;
    }

    dd_edge buildMove(direction d, const int* f, const int* o) {
      // Translate ith variable of type j, to the actual variable 

      int c1 = c(f[0]);
      int c2 = c(f[2]);
      int c3 = c(f[4]);
      int c4 = c(f[6]);
      int co1 = co(f[0]);
      int co2 = co(f[2]);
      int co3 = co(f[4]);
      int co4 = co(f[6]);
      int co1offset = o[0];
      int co2offset = o[2];
      int co3offset = o[4];
      int co4offset = o[6];

      int e1 = e(f[1]);
      int e2 = e(f[3]);
      int e3 = e(f[5]);
      int e4 = e(f[7]);
      int eo1 = eo(f[1]);
      int eo2 = eo(f[3]);
      int eo3 = eo(f[5]);
      int eo4 = eo(f[7]);
      int eo1offset = o[1];
      int eo2offset = o[3];
      int eo3offset = o[5];
      int eo4offset = o[7];

      int centero = center(f[8]);
      int centeroffset = o[8];

      dd_edge r(mxd);
      r = buildRelevantVariables(c1, c2, c3, c4, e1, e2, e3, e4,
          co1, co2, co3, co4, eo1, eo2, eo3, eo4, centero);
      if (d == CW) {
        // clockwise 90 degrees.
        if (enableCorners) {
          r *= buildPair(c1, c2);
          r *= buildPair(c2, c3);
          r *= buildPair(c3, c4);
          r *= buildPair(c4, c1);
          if (enableCornerOrientations) {
            r *= buildPair(co1, co2, co1offset);
            r *= buildPair(co2, co3, co2offset);
            r *= buildPair(co3, co4, co3offset);
            r *= buildPair(co4, co1, co4offset);
          }
        }
        if (enableEdges) {
          r *= buildPair(e1, e2);
          r *= buildPair(e2, e3);
          r *= buildPair(e3, e4);
          r *= buildPair(e4, e1);
          if (enableEdgeOrientations) {
            r *= buildPair(eo1, eo2, eo1offset);
            r *= buildPair(eo2, eo3, eo2offset);
            r *= buildPair(eo3, eo4, eo3offset);
            r *= buildPair(eo4, eo1, eo4offset);
          }
        }
        if (enableCenterOrientations) {
          r *= buildPair(centero, centero, centeroffset);
        }
      } else if (d == FLIP) {
        // 180 degrees.
        if (enableCorners) {
          r *= buildPair(c1, c3);
          r *= buildPair(c2, c4);
          r *= buildPair(c3, c1);
          r *= buildPair(c4, c2);
          if (enableCornerOrientations) {
            r *= buildPair(co1, co3);
            r *= buildPair(co2, co4);
            r *= buildPair(co3, co1);
            r *= buildPair(co4, co2);
          }
        }
        if (enableEdges) {
          r *= buildPair(e1, e3);
          r *= buildPair(e2, e4);
          r *= buildPair(e3, e1);
          r *= buildPair(e4, e2);
          if (enableEdgeOrientations) {
            r *= buildPair(eo1, eo3);
            r *= buildPair(eo2, eo4);
            r *= buildPair(eo3, eo1);
            r *= buildPair(eo4, eo2);
          }
        }
        if (enableCenterOrientations) {
          r *= buildPair(centero, centero, 2*centeroffset);
        }
      } else {
        // clockwise 270 degrees.
        if (enableCorners) {
          r *= buildPair(c1, c4);
          r *= buildPair(c2, c1);
          r *= buildPair(c3, c2);
          r *= buildPair(c4, c3);
          if (enableCornerOrientations) {
            r *= buildPair(co1, co4, co1offset);
            r *= buildPair(co2, co1, co2offset);
            r *= buildPair(co3, co2, co3offset);
            r *= buildPair(co4, co3, co4offset);
          }
        }
        if (enableEdges) {
          r *= buildPair(e1, e4);
          r *= buildPair(e2, e1);
          r *= buildPair(e3, e2);
          r *= buildPair(e4, e3);
          if (enableEdgeOrientations) {
            r *= buildPair(eo1, eo4, eo1offset);
            r *= buildPair(eo2, eo1, eo2offset);
            r *= buildPair(eo3, eo2, eo3offset);
            r *= buildPair(eo4, eo3, eo4offset);
          }
        }
        if (enableCenterOrientations) {
          r *= buildPair(centero, centero, 3*centeroffset);
        }
      }
      return r;
    }


    dd_edge buildMove(face f, direction d)
    {
      // order: starting from top left corner, go clockwise. last is center.

      // F-CW: 1, 1, 2, 2, 3, 3, 4, 4
      static const int front[] = { 1, 1, 2, 2, 3, 3, 4, 4, 1};
      static const int forient[] = {0, 0, 0, 0, 0, 0, 0, 0, 1};

      // B-CW: 6, 5, 5, 8, 8, 7, 7, 6
      static const int back[] = { 6, 5, 5, 8, 8, 7, 7, 6, 2};
      static const int borient[] = {0, 0, 0, 0, 0, 0, 0, 0, 1};

      // L-CW: 5, 12, 1, 4, 4, 11, 8, 8
      static const int left[] = { 5, 12, 1, 4, 4, 11, 8, 8, 3};
      static const int lorient[] = {1, 1, 2, 1, 1, 1, 2, 1, 1};

      // R-CW: 2, 9, 6, 6, 7, 10, 3, 2
      static const int right[] = { 2, 9, 6, 6, 7, 10, 3, 2, 4};
      static const int rorient[] = {1, 1, 2, 1, 1, 1, 2, 1, 1};

      // U-CW: 5, 5, 6, 9, 2, 1, 1, 12
      static const int up[] = { 5, 5, 6, 9, 2, 1, 1, 12, 5};
      static const int uorient[] = {2, 0, 1, 0, 2, 0, 1, 0, 1};

      // D-CW: 4, 3, 3, 10, 7, 7, 8, 11
      static const int down[] = { 4, 3, 3, 10, 7, 7, 8, 11, 6};
      static const int dorient[] = {2, 0, 1, 0, 2, 0, 1, 0, 1};


      // F-CW: 1, 1, 2, 2, 3, 3, 4, 4
      if (f == F) return buildMove(d, front, forient);

      // L-CW: 5, 12, 1, 4, 4, 11, 8, 8
      if (f == L) return buildMove(d, left, lorient);

      // R-CW: 2, 9, 6, 6, 7, 10, 3, 2
      if (f == R) return buildMove(d, right, rorient);

      // U-CW: 5, 5, 6, 9, 2, 1, 1, 12
      if (f == U) return buildMove(d, up, uorient);

      // D-CW: 4, 3, 3, 10, 7, 7, 8, 11
      if (f == D) return buildMove(d, down, dorient);

      // B-CW: 6, 5, 5, 8, 8, 7, 7, 6
      assert(f == B);
      return buildMove(d, back, borient);
    }

    void buildNextStateFunction(const moves& m,
        satpregen_opname::pregen_relation& ensf, bool split)
    {
      // Build each move using buildMove().

      timer start;

      // Clock-wise moves
      if (m.FCW) ensf.addToRelation(buildMove(F, CW));
      if (m.BCW) ensf.addToRelation(buildMove(B, CW));
      if (m.UCW) ensf.addToRelation(buildMove(U, CW));
      if (m.DCW) ensf.addToRelation(buildMove(D, CW));
      if (m.LCW) ensf.addToRelation(buildMove(L, CW));
      if (m.RCW) ensf.addToRelation(buildMove(R, CW));

      // Counter clock-wise moves
      if (m.FCCW) ensf.addToRelation(buildMove(F, CCW));
      if (m.BCCW) ensf.addToRelation(buildMove(B, CCW));
      if (m.UCCW) ensf.addToRelation(buildMove(U, CCW));
      if (m.DCCW) ensf.addToRelation(buildMove(D, CCW));
      if (m.LCCW) ensf.addToRelation(buildMove(L, CCW));
      if (m.RCCW) ensf.addToRelation(buildMove(R, CCW));

      // Flip moves
      if (m.FF) ensf.addToRelation(buildMove(F, FLIP));
      if (m.BF) ensf.addToRelation(buildMove(B, FLIP));
      if (m.UF) ensf.addToRelation(buildMove(U, FLIP));
      if (m.DF) ensf.addToRelation(buildMove(D, FLIP));
      if (m.LF) ensf.addToRelation(buildMove(L, FLIP));
      if (m.RF) ensf.addToRelation(buildMove(R, FLIP));

      start.note_time();
      fprintf(stdout, "Time for building individual events: %.4e seconds\n",
          start.get_last_interval()/1000000.0);
      fflush(stdout);
      start.note_time();

      if (split) {
        //ensf.finalize(satpregen_opname::pregen_relation::SplitOnly);
        //ensf.finalize(satpregen_opname::pregen_relation::SplitSubtract);
        //ensf.finalize(satpregen_opname::pregen_relation::MonolithicSplit);
        ensf.finalize(satpregen_opname::pregen_relation::SplitSubtractAll);
      } else {
        ensf.finalize(satpregen_opname::pregen_relation::MonolithicSplit);
      }

      start.note_time();
      fprintf(stdout, "Time for splitting events: %.4e seconds\n",
          start.get_last_interval()/1000000.0);
      fflush(stdout);
    }

    dd_edge buildNextStateFunction(const moves& m)
    {
      // Build each move using buildMove().
      dd_edge r(mxd);

      // Clock-wise moves
      if (m.FCW) r += buildMove(F, CW);
      if (m.BCW) r += buildMove(B, CW);
      if (m.UCW) r += buildMove(U, CW);
      if (m.DCW) r += buildMove(D, CW);
      if (m.LCW) r += buildMove(L, CW);
      if (m.RCW) r += buildMove(R, CW);

      // Counter clock-wise moves
      if (m.FCCW) r += buildMove(F, CCW);
      if (m.BCCW) r += buildMove(B, CCW);
      if (m.UCCW) r += buildMove(U, CCW);
      if (m.DCCW) r += buildMove(D, CCW);
      if (m.LCCW) r += buildMove(L, CCW);
      if (m.RCCW) r += buildMove(R, CCW);

      // Flip moves
      if (m.FF) r += buildMove(F, FLIP);
      if (m.BF) r += buildMove(B, FLIP);
      if (m.UF) r += buildMove(U, FLIP);
      if (m.DF) r += buildMove(D, FLIP);
      if (m.LF) r += buildMove(L, FLIP);
      if (m.RF) r += buildMove(R, FLIP);

      return r;
    }

    int doBfs(const moves& m)
    {
      assert(mdd);
      assert(mxd);
      assert(order);

      // Build overall Next-State Function.
      dd_edge nsf = buildNextStateFunction(m);

      // Build initial state.
      dd_edge initial = buildInitialState();

      // Perform Reachability via "traditional" reachability algorithm.
      fprintf(stdout, "Started BFS Saturate...");
      fflush(stdout);
      timer start;
      apply(REACHABLE_STATES_BFS, initial, nsf, initial);
      start.note_time();
      fprintf(stdout, " done!\n");
      fflush(stdout);
      fprintf(stdout, "Time for constructing reachability set: %.4e seconds\n",
          start.get_last_interval()/1000000.0);
      fprintf(stdout, "# of reachable states: %1.6e\n",
          initial.getCardinality());
      fflush(stdout);

      return 0;
    }


    int doDfs(const moves& m, char saturation_type, bool split)
    {
      assert(mdd);
      assert(mxd);
      assert(order);

      satpregen_opname::pregen_relation *ensf = 0;
      dd_edge nsf(mxd);

      // Build initial state.
      dd_edge initial = buildInitialState();

      printf("Building transition diagrams...");
      fflush(stdout);

      timer start;

      // Build next state function.
      if (saturation_type == 'e') {
        ensf = new satpregen_opname::pregen_relation(mdd, mxd, mdd,
            m.countEnabledMoves());
        buildNextStateFunction(m, *ensf, split);
      } else if (saturation_type == 'k') {
        ensf = new satpregen_opname::pregen_relation(mdd, mxd, mdd);
        buildNextStateFunction(m, *ensf, split);
      } else {
        nsf = buildNextStateFunction(m);
      }
      
      start.note_time();
      fprintf(stdout,
          "Time for building next-state function: %.4e seconds\n",
          start.get_last_interval()/1000000.0);
      FILE_output myout(stdout);
      printStats(myout);
      fflush(stdout);

      mxd->removeAllComputeTableEntries();
      mtmxd->removeAllComputeTableEntries();

      // Identify the type of splitting to be performed for saturation
      printf("done.\n");
      if (saturation_type == 'e') {
        printf("Building reachability set: Event-wise saturation\n");
      } else if (saturation_type == 'k') {
        printf("Building reachability set: Level-wise saturation");
        if (split) printf(" with splitting");
        printf("\n");
      } else {
        printf("Building reachability set: Monolithic relation saturation\n");
      }
      fflush(stdout);
      
      // Perform Reacability via "saturation".
      start.note_time();
      if (ensf) {
        if (0==SATURATION_FORWARD) throw error(error::UNKNOWN_OPERATION);
        specialized_operation *sat = SATURATION_FORWARD->buildOperation(ensf);
        if (0==sat) throw error(error::INVALID_OPERATION);
        sat->compute(initial, initial);
      } else {
        apply(REACHABLE_STATES_DFS, initial, nsf, initial);
      }

      start.note_time();
      fprintf(stdout, "Time for constructing reachability set: %.4e seconds\n",
          start.get_last_interval()/1000000.0);
      fprintf(stdout, "# of reachable states: %1.6e\n",
          initial.getCardinality());
      fflush(stdout);

      return 0;
    }


#if 0
    int doChoice(const moves& m)
    {
      assert(mdd);
      assert(mxd);
      assert(order);

      // Build Next-State Function for each "move".
      // Moves 0-5 are CW, 6-11 are CCW, 12-17 are FLIP.
      // Order within each set: F, B, L, R, U, D.

      const int nFaces = 6;
      vector<dd_edge> nsf;

      // Clock-wise moves
      nsf.push_back(buildMove(F, CW));
      nsf.push_back(buildMove(B, CW));
      nsf.push_back(buildMove(L, CW));
      nsf.push_back(buildMove(R, CW));
      nsf.push_back(buildMove(U, CW));
      nsf.push_back(buildMove(D, CW));
      // Counter Clock-wise moves
      nsf.push_back(buildMove(F, CCW));
      nsf.push_back(buildMove(B, CCW));
      nsf.push_back(buildMove(L, CCW));
      nsf.push_back(buildMove(R, CCW));
      nsf.push_back(buildMove(U, CCW));
      nsf.push_back(buildMove(D, CCW));
      // Flip moves
      nsf.push_back(buildMove(F, FLIP));
      nsf.push_back(buildMove(B, FLIP));
      nsf.push_back(buildMove(L, FLIP));
      nsf.push_back(buildMove(R, FLIP));
      nsf.push_back(buildMove(U, FLIP));
      nsf.push_back(buildMove(D, FLIP));

      // Perform reachability via user-interaction.
      // Display menu.
      // Perform user command.
      // Repeat.

      // Display menu and get choice
      assert(mdd);

      dd_edge initial(mdd);
      mdd->createEdge(initst, 1, initial);

      dd_edge result = initial;
      dd_edge temp(mdd);
      bool continueLoop = true;
      int choice = 21;

      while (continueLoop)
      {
        fprintf(stdout, "------- Choices -------\n");
        fprintf(stdout, "CW(0:F, B, L, R, U, 5:D), CCW(6-11), FLIP(12-17).\n");
        fprintf(stdout, "18: Perform Garbage Collection.\n");
        fprintf(stdout, "19: Reset result to initial state.\n");
        fprintf(stdout, "20: Report.\n");
        fprintf(stdout, "21: Quit.\n");
        fprintf(stdout, "Enter choice (0-21): ");
        cin >> choice;

        switch (choice) {
          case 0:
          case 1:
          case 2:
          case 3:
          case 4:
          case 5:
          case 6:
          case 7:
          case 8:
          case 9:
          case 10:
          case 11:
          case 12:
          case 13:
          case 14:
          case 15:
          case 16:
          case 17:
            {
              face f = face(choice % nFaces);
              direction d = direction(choice / nFaces);
              printf("Choice: %d, Face: %d, Direction: %d\n", choice, f, d);
            }
            apply(POST_IMAGE, result, nsf[choice], temp);
            result += temp;
            break;
          case 18:
            // Perform garbage collection.
            mdd->garbageCollect();
            break;
          case 19:
            // Clear result.
            result = initial;
            break;
          case 20:
            // Print Report.
            // For now, print the cardinality of result.
            printf("Cardinality of result: %1.6e\n", result.getCardinality());
            break;
          case 21:
            // Quit.
            printf("Quit!\n");
            continueLoop = false;
            break;
          default:
            printf("Invalid choice: %d\n", choice);
            break;
        }
      }

      return 0;
    }


    int doSteppedBfs(const moves& m)
    {
      assert(mdd);
      assert(mxd);
      assert(order);

      // Performs DFS for each event starting with initial state.
      // At the end of each iteration, peform a union and repeat.

      // Build Next-State Function for each "move".
      // Moves 0-5 are CW, 6-11 are CCW, 12-17 are FLIP.
      // Order within each set: F, B, L, R, U, D.

      const int nFaces = 6;
      vector<dd_edge> nsf;

      // Clock-wise moves
      nsf.push_back(buildMove(F, CW));
      nsf.push_back(buildMove(B, CW));
      nsf.push_back(buildMove(L, CW));
      nsf.push_back(buildMove(R, CW));
      nsf.push_back(buildMove(U, CW));
      nsf.push_back(buildMove(D, CW));

      // Perform reachability via user-interaction.
      // Display menu.
      // Perform user command.
      // Repeat.

      // Display menu and get choice
      assert(mdd);

      dd_edge initial(mdd);
      mdd->createEdge(initst, 1, initial);

      dd_edge result = initial;
      dd_edge temp(mdd);
      vector<dd_edge> eventResults;
      for (int i = 0; i < nFaces; i++)
      {
        eventResults.push_back(initial);
      }

      bool continueLoop = true;
      int choice = 4;

      while (continueLoop)
      {
        fprintf(stdout, "------- Choices -------\n");
        fprintf(stdout, "0: Perform event-wise DFS.\n");
        fprintf(stdout, "1: Perform Union\n");
        fprintf(stdout, "2: Report cardinalities.\n");
        fprintf(stdout, "3: Garbage Collection.\n");
        fprintf(stdout, "4: Reset.\n");
        fprintf(stdout, "5: Quit.\n");
        fprintf(stdout, "Enter choice (0-5): ");
        cin >> choice;

        switch (choice) {
          case 0:
            // for each face perform dfs and store in eventResults.
            for (int i = 0; i < nFaces; i++)
            {
              printf("Processing event[%d]...", i);
              fflush(stdout);
              apply(
                  REACHABLE_STATES_DFS, eventResults[i], nsf[i], eventResults[i]
                  );
              printf("done.\n");
              fflush(stdout);
            }
            break;
          case 1:
            // Result = union of all eventResults.
            for (int i = 0; i < nFaces; i++)
            {
              result += eventResults[i];
            }
            for (int i = 0; i < nFaces; i++)
            {
              eventResults[i] = result - eventResults[i];
            }
            break;
          case 2:
            // Print Report.
            // For now, print the cardinality of result.
            printf("Cardinality of result: %1.6e\n", result.getCardinality());
            for (int i = 0; i < nFaces; i++)
            {
              printf("Cardinality of event[%d]: %1.6e\n",
                  i, eventResults[i].getCardinality());
            }
            break;
          case 3:
            // Perform garbage collection.
            mdd->garbageCollect();
            break;
          case 4:
            // Clear result.
            result = initial;
            for (int i = 0; i < nFaces; i++)
            {
              eventResults[i] = result;
            }
            break;
          case 5:
            // Quit.
            printf("Quit!\n");
            continueLoop = false;
            break;
          default:
            printf("Invalid choice: %d\n", choice);
            break;
        }
      }

      return 0;
    }
#endif

    void printStats(MEDDLY::output &out)
    {
      printStats(out, "Mdd", mdd);
      printStats(out, "Mxd", mxd);
      printStats(out, "MtMxd", mtmxd);
      MEDDLY::operation::showAllComputeTables(out, 3);
      out.flush();
    }

    void printStats(MEDDLY::output &out, const char* who, const forest* f)
    { 
      out << who << " stats:\n";
      const expert_forest* ef = (expert_forest*) f;
      ef->reportStats(out, "\t",
          expert_forest::HUMAN_READABLE_MEMORY  |
          expert_forest::BASIC_STATS | expert_forest::EXTRA_STATS |
          expert_forest::STORAGE_STATS | expert_forest::HOLE_MANAGER_STATS |
          expert_forest::UNIQUE_TABLE_STATS
          );
    }

};

const int rubiks::plus0mod2[2] = {0, 1};
const int rubiks::plus1mod2[2] = {1, 0};
const int rubiks::plus0mod3[3] = {0, 1, 2};
const int rubiks::plus1mod3[3] = {1, 2, 0};
const int rubiks::plus2mod3[3] = {2, 0, 1};
const int rubiks::plus0mod4[4] = {0, 1, 2, 3};
const int rubiks::plus1mod4[4] = {1, 2, 3, 0};
const int rubiks::plus2mod4[4] = {2, 3, 0, 1};
const int rubiks::plus3mod4[4] = {3, 0, 1, 2};
const int rubiks::plus0mod8[8] = {0, 1, 2, 3, 4, 5, 6, 7};
const int rubiks::plus0mod12[12] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

const int rubiks::defaultVariableOrdering[rubiks::nLocations+1] = {
  0,
  1, 2, 3, 4, 5, 6, 7, 8,
  9, 10, 11, 12, 13, 14, 15, 16,
  17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28,
  29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
  41, 42, 43, 44, 45, 46
};

const int rubiks::BenSmithsVariableOrdering[rubiks::nLocations+1] = {
  /*
  0,
  3, 5, 6, 4, 2, 8, 7, 1,
  12, 11, 9, 10, 15, 13, 14, 16,
  17, 21, 19, 18, 24, 22, 26, 25, 23, 20, 27, 28,
  29, 33, 31, 30, 36, 34, 38, 37, 35, 32, 39, 40,
  41, 42, 43, 44, 45, 46
  */
  0,
  4, 6, 5, 3, 1, 7, 8, 2,
  9, 10, 12, 11, 16, 14, 13, 15,
  18, 20, 19, 17, 24, 23, 26, 25, 22, 21, 27, 28,
  30, 32, 31, 29, 36, 35, 38, 37, 34, 33, 39, 40,
  41, 42, 43, 44, 45, 46
};

void usage() {
  fprintf(stderr, "Usage: rubik_cube [-bfs|-dfs|-l<key>|-p]\n");
  fprintf(stderr, "-bfs   : use traditional algorithm to compute reachable states\n");
  fprintf(stderr, "-msat  : use saturation with monolithic relation compute reachable states\n");
  fprintf(stderr, "-esat  : use saturation with event-wise relation compute reachable states\n");
  fprintf(stderr, "-ksat  : use saturation with level-wise relation compute reachable states\n");
  fprintf(stderr, "-kspsat: same as ksat, with additional pre-processing of events to improve saturation\n");
  fprintf(stderr, "-l<key>: key can be any combination of\n");
  fprintf(stderr, "         A: Front face clock-wise rotation,\n");
  fprintf(stderr, "         a: Front face counter clock-wise rotation,\n");
  fprintf(stderr, "         1: Front face flip,\n");
  fprintf(stderr, "         Back face (B, b, 2),\n");
  fprintf(stderr, "         Left face (C, c, 3),\n");
  fprintf(stderr, "         Right face (D, d, 4),\n");
  fprintf(stderr, "         Up face (E, e, 5),\n");
  fprintf(stderr, "         Down face (F, f, 6),\n");
  fprintf(stderr, "-p     : prints initial states, nsf, reachable states on stderr\n");
  fprintf(stderr, "-c     : enable corners\n");
  fprintf(stderr, "-co    : enable corners orientations\n");
  fprintf(stderr, "-e     : enable edges\n");
  fprintf(stderr, "-eo    : enable edges orientations\n");
  fprintf(stderr, "-center: enable center orientations\n");
  fprintf(stderr, "\n");
}



int main(int argc, char *argv[])
{
  moves enabled;
  bool dfs = false;
  bool bfs = false;
  char saturation_type = 'm';
  bool split = false;
  bool enableCorners = false;
  bool enableCornerOrientations = false;
  bool enableEdges = false;
  bool enableEdgeOrientations = false;
  bool enableCenterOrientations = false;

  if (argc > 1) {
    assert(argc <= 9);
    for (int i=1; i<argc; i++) {
      char *cmd = argv[i];
      if (strncmp(cmd, "-msat", 6) == 0) {
        dfs = true; saturation_type = 'm';
      }
      else if (strncmp(cmd, "-esat", 6) == 0) {
        dfs = true; saturation_type = 'e';
      }
      else if (strncmp(cmd, "-ksat", 6) == 0) {
        dfs = true; saturation_type = 'k'; split = false;
      }
      else if (strncmp(cmd, "-kspsat", 8) == 0) {
        dfs = true; saturation_type = 'k'; split = true;
      }
      else if (strncmp(cmd, "-bfs", 5) == 0) bfs = true;
      else if (strncmp(cmd, "-c", 3) == 0) enableCorners = true;
      else if (strncmp(cmd, "-co", 4) == 0) enableCornerOrientations = true;
      else if (strncmp(cmd, "-e", 3) == 0) enableEdges = true;
      else if (strncmp(cmd, "-eo", 4) == 0) enableEdgeOrientations = true;
      else if (strncmp(cmd, "-center", 8) == 0) enableCenterOrientations = true;
      else if (strncmp(cmd, "-l", 2) == 0) {
        for (unsigned j = 2; j < strlen(cmd); j++) {
          switch (cmd[j]) {
            case 'A': enabled.FCW = true; break;
            case 'a': enabled.FCCW = true; break;
            case '1': enabled.FF = true; break;
            case 'B': enabled.BCW = true; break;
            case 'b': enabled.BCCW = true; break;
            case '2': enabled.BF = true; break;
            case 'C': enabled.LCW = true; break;
            case 'c': enabled.LCCW = true; break;
            case '3': enabled.LF = true; break;
            case 'D': enabled.RCW = true; break;
            case 'd': enabled.RCCW = true; break;
            case '4': enabled.RF = true; break;
            case 'E': enabled.UCW = true; break;
            case 'e': enabled.UCCW = true; break;
            case '5': enabled.UF = true; break;
            case 'F': enabled.DCW = true; break;
            case 'f': enabled.DCCW = true; break;
            case '6': enabled.DF = true; break;
          }
        }
      }
      else {
        usage();
        exit(1);
      }
    }
  }

  if (enableCorners || enableCornerOrientations
      || enableEdges || enableEdgeOrientations || enableCenterOrientations) {
  } else {
    // set up defaults
    //
    printf("No locations were enabled by user. Enabling all components.\n");
    fflush(stdout);
    enableCorners = true;
    enableCornerOrientations = true;
    enableEdges = true;
    enableEdgeOrientations = true;
    enableCenterOrientations = true;
  }

  // set up arrays based on number of levels
  const int* order = rubiks::BenSmithsVariableOrdering;
  rubiks model(order, rubiks::nLocations + 1,
      enableCorners, enableCornerOrientations,
      enableEdges, enableEdgeOrientations, enableCenterOrientations);

  try {

    if (dfs) {
      model.doDfs(enabled, saturation_type, split);
    }

    if (bfs) {
      model.doBfs(enabled);
    }

    if (!dfs && !bfs) {
#if 0
#if 0
      model.doChoice(enabled);
#else
      model.doSteppedBfs(enabled);
#endif
#endif
    }
  }
  catch (MEDDLY::error& e) {
    fprintf(stderr, "Meddly error: %s\n", e.getName());
    fflush(stderr);
  } 
  catch (...) {
    fprintf(stderr, "Unknown error.\n");
    fflush(stderr);
  }

  FILE_output myout(stdout);
  model.printStats(myout);
  fflush(stdout);

  fprintf(stdout, "\n\nDONE\n");
  fflush(stdout);
  return 0;
}



