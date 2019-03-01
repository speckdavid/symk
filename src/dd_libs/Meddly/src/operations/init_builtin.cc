

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
#include "init_builtin.h"

#include "copy.h"
#include "cardinality.h"
#include "complement.h"
#include "maxmin_range.h"
#include "mdd2index.h"

#include "union.h"
#include "intersection.h"
#include "difference.h"
#include "cross.h"

#include "maxmin.h"
#include "plus.h"
#include "minus.h"
#include "multiply.h"
#include "divide.h"
#include "modulo.h"

#include "comp_eq.h"
#include "comp_ne.h"
#include "comp_lt.h"
#include "comp_le.h"
#include "comp_gt.h"
#include "comp_ge.h"

#include "prepostimage.h"
#include "reach_bfs.h"
#include "reach_dfs.h"
#include "sat_pregen.h"
#include "sat_otf.h"

#include "vect_matr.h"

#include "mm_mult.h"


#include "mpz_object.h"

namespace MEDDLY {

  // unary operation "codes"

  const unary_opname* COPY = 0;
  const unary_opname* CARDINALITY = 0;
  const unary_opname* COMPLEMENT = 0;
  const unary_opname* MAX_RANGE = 0;
  const unary_opname* MIN_RANGE = 0;
  const unary_opname* CONVERT_TO_INDEX_SET = 0;

  // binary operation "codes"

  const binary_opname* UNION = 0;
  const binary_opname* INTERSECTION = 0;
  const binary_opname* DIFFERENCE = 0;
  const binary_opname* CROSS = 0;

  const binary_opname* MINIMUM = 0;
  const binary_opname* MAXIMUM = 0;
  const binary_opname* PLUS = 0;
  const binary_opname* MINUS = 0;
  const binary_opname* MULTIPLY = 0;
  const binary_opname* DIVIDE = 0;
  const binary_opname* MODULO = 0;

  const binary_opname* EQUAL = 0;
  const binary_opname* NOT_EQUAL = 0;
  const binary_opname* LESS_THAN = 0;
  const binary_opname* LESS_THAN_EQUAL = 0;
  const binary_opname* GREATER_THAN = 0;
  const binary_opname* GREATER_THAN_EQUAL = 0;

  const binary_opname* PRE_IMAGE = 0;
  const binary_opname* POST_IMAGE = 0;
  const binary_opname* REACHABLE_STATES_DFS = 0;
  const binary_opname* REACHABLE_STATES_BFS = 0;
  const binary_opname* REVERSE_REACHABLE_DFS = 0;
  const binary_opname* REVERSE_REACHABLE_BFS = 0;

  const binary_opname* VM_MULTIPLY = 0;
  const binary_opname* MV_MULTIPLY = 0;

  const binary_opname* MM_MULTIPLY = 0;

  // numerical operation "codes"

  const numerical_opname* EXPLVECT_MATR_MULT = 0;
  const numerical_opname* MATR_EXPLVECT_MULT = 0;

  // saturation operation "codes"

  const satpregen_opname* SATURATION_FORWARD = 0;
  const satpregen_opname* SATURATION_BACKWARD = 0;
  const satotf_opname* SATURATION_OTF_FORWARD = 0;
};



MEDDLY::builtin_initializer::builtin_initializer(initializer_list *p)
 : initializer_list(p)
{
}



template <class T>
inline void initP(const T* &global, T* &local, T* init)
{
  global = (local = init);
}


void MEDDLY::builtin_initializer::setup()
{
  initP(MEDDLY::COPY,                 COPY,       initializeCopy()          );
  initP(MEDDLY::CARDINALITY,          CARD,       initializeCardinality()   );
  initP(MEDDLY::COMPLEMENT,           COMPL,      initializeComplement()    );
  initP(MEDDLY::MAX_RANGE,            MAXRANGE,   initializeMaxRange()      );
  initP(MEDDLY::MIN_RANGE,            MINRANGE,   initializeMaxRange()      );
  initP(MEDDLY::CONVERT_TO_INDEX_SET, MDD2INDEX,  initializeMDD2INDEX()     );

  initP(MEDDLY::UNION,                UNION,      initializeUnion()         );
  initP(MEDDLY::INTERSECTION,         INTERSECT,  initializeIntersection()  );
  initP(MEDDLY::DIFFERENCE,           DIFFERENCE, initializeDifference()    );
  initP(MEDDLY::CROSS,                CROSS,      initializeCross()         );

  initP(MEDDLY::MAXIMUM,              MAX,        initializeMaximum()       );
  initP(MEDDLY::MINIMUM,              MIN,        initializeMinimum()       );
  initP(MEDDLY::PLUS,                 PLUS,       initializePlus()          );
  initP(MEDDLY::MINUS,                MINUS,      initializeMinus()         );
  initP(MEDDLY::MULTIPLY,             MULTIPLY,   initializeMultiply()      );
  initP(MEDDLY::DIVIDE,               DIVIDE,     initializeDivide()        );
  initP(MEDDLY::MODULO,               MODULO,     initializeModulo()        );

  initP(MEDDLY::EQUAL,                EQ,           initializeEQ()          );
  initP(MEDDLY::NOT_EQUAL,            NE,           initializeNE()          );
  initP(MEDDLY::LESS_THAN,            LT,           initializeLT()          );
  initP(MEDDLY::LESS_THAN_EQUAL,      LE,           initializeLE()          );
  initP(MEDDLY::GREATER_THAN,         GT,           initializeGT()          );
  initP(MEDDLY::GREATER_THAN_EQUAL,   GE,           initializeGE()          );

  initP(MEDDLY::PRE_IMAGE,            PRE_IMAGE,    initializePreImage()    );
  initP(MEDDLY::POST_IMAGE,           POST_IMAGE,   initializePostImage()   );
  initP(MEDDLY::REACHABLE_STATES_DFS, FORWARD_DFS,  initializeForwardDFS()  );
  initP(MEDDLY::REACHABLE_STATES_BFS, FORWARD_BFS,  initializeForwardBFS()  );
  initP(MEDDLY::REVERSE_REACHABLE_DFS,BACKWARD_DFS, initializeBackwardDFS() );
  initP(MEDDLY::REVERSE_REACHABLE_BFS,BACKWARD_BFS, initializeBackwardBFS() );

  initP(MEDDLY::VM_MULTIPLY,          VM_MULTIPLY,  initializeVMmult()      );
  initP(MEDDLY::MV_MULTIPLY,          MV_MULTIPLY,  initializeMVmult()      );

  initP(MEDDLY::MM_MULTIPLY,          MM_MULTIPLY,  initializeMMMultiply()  );

  initP(MEDDLY::EXPLVECT_MATR_MULT, EXPLVECT_MATR_MULT, initExplVectorMatrixMult()  );
  initP(MEDDLY::MATR_EXPLVECT_MULT, MATR_EXPLVECT_MULT, initMatrixExplVectorMult()  );

  initP(MEDDLY::SATURATION_FORWARD,   SATURATION_FORWARD,   initSaturationForward()   );
  initP(MEDDLY::SATURATION_BACKWARD,  SATURATION_BACKWARD,  initSaturationBackward()  );
  initP(MEDDLY::SATURATION_OTF_FORWARD,   SATURATION_OTF_FORWARD,   initOtfSaturationForward()  );

#ifdef HAVE_LIBGMP
  mpz_object::initBuffer();
#endif
}



template <class T>
inline void cleanPair(T *local, const T* &global)
{
  delete local;
  global = 0;
}

void MEDDLY::builtin_initializer::cleanup()
{
  cleanPair(COPY,           MEDDLY::COPY);
  cleanPair(CARD,           MEDDLY::CARDINALITY);
  cleanPair(COMPL,          MEDDLY::COMPLEMENT);
  cleanPair(MAXRANGE,       MEDDLY::MAX_RANGE);
  cleanPair(MINRANGE,       MEDDLY::MIN_RANGE);
  cleanPair(MDD2INDEX,      MEDDLY::CONVERT_TO_INDEX_SET);

  cleanPair(UNION,          MEDDLY::UNION);
  cleanPair(INTERSECT,      MEDDLY::INTERSECTION);
  cleanPair(DIFFERENCE,     MEDDLY::DIFFERENCE);
  cleanPair(CROSS,          MEDDLY::CROSS);

  cleanPair(MAX,            MEDDLY::MAXIMUM);
  cleanPair(MIN,            MEDDLY::MINIMUM);
  cleanPair(PLUS,           MEDDLY::PLUS);
  cleanPair(MINUS,          MEDDLY::MINUS);
  cleanPair(MULTIPLY,       MEDDLY::MULTIPLY);
  cleanPair(DIVIDE,         MEDDLY::DIVIDE);
  cleanPair(MODULO,         MEDDLY::MODULO);

  cleanPair(EQ,             MEDDLY::EQUAL);
  cleanPair(NE,             MEDDLY::NOT_EQUAL);
  cleanPair(LT,             MEDDLY::LESS_THAN);
  cleanPair(LE,             MEDDLY::LESS_THAN_EQUAL);
  cleanPair(GT,             MEDDLY::GREATER_THAN);
  cleanPair(GE,             MEDDLY::GREATER_THAN_EQUAL);

  cleanPair(PRE_IMAGE,      MEDDLY::PRE_IMAGE);
  cleanPair(POST_IMAGE,     MEDDLY::POST_IMAGE);
  cleanPair(FORWARD_DFS,    MEDDLY::REACHABLE_STATES_DFS);
  cleanPair(FORWARD_BFS,    MEDDLY::REACHABLE_STATES_BFS);
  cleanPair(BACKWARD_DFS,   MEDDLY::REVERSE_REACHABLE_DFS);
  cleanPair(BACKWARD_BFS,   MEDDLY::REVERSE_REACHABLE_BFS);

  cleanPair(SATURATION_BACKWARD,      MEDDLY::SATURATION_BACKWARD );
  cleanPair(SATURATION_FORWARD,       MEDDLY::SATURATION_FORWARD  );
  cleanPair(SATURATION_OTF_FORWARD,   MEDDLY::SATURATION_OTF_FORWARD  );

  cleanPair(EXPLVECT_MATR_MULT, MEDDLY::EXPLVECT_MATR_MULT);
  cleanPair(MATR_EXPLVECT_MULT, MEDDLY::MATR_EXPLVECT_MULT);

  cleanPair(MM_MULTIPLY,    MEDDLY::MM_MULTIPLY);


#ifdef HAVE_LIBGMP
  mpz_object::clearBuffer();
#endif
}

