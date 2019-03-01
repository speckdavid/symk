
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

	Type 1: Single square (6 such squares)  ,  6 * 1 =  6
	Type 2: Two squares (12 such L-corners) , 12 * 2 = 24
	Type 3: Three squares (8 such corners)  ,  8 * 3 = 24
	
	The model thus has 26 components and 26 component locations.
	Each component location is represented as a MDD level. The size of the MDD
	level depends on the number of components that can fit into that location.
	For example, Type 2 component locations are of size 12.
  
	Level 0        : Terminals
	Level 01 - 12  : Type 2 component locations (size 12)
	Level 13 - 20  : Type 3 component locations (size 8)

  The 6 Type 1 component locations (size 6) are not represented since they
  never move.	Previously at levels 21 - 26.

	Up:     In order (going right starting from front face left-upper corner)
          of components (Type:id),
          (3:0, 2:0, 3:1, 2:1, 3:2, 2:2, 3:3, 2:3)
          Note: (1:0) also belongs to this row but since it stays in the
          same location when this row is moved left or right, it is ignored.
	Down:   (3:4, 2:8, 3:5, 2:9, 3:6, 2:10, 3:7, 2:11) (1:5 ignored)
	Left:   (3:0, 2:4, 3:4, 2:11, 3:7, 2:7, 3:3, 2:3) (1:4 ignored)
	Right:  (3:1, 2:5, 3:5, 2:9, 3:6, 2:6, 3:2, 2:1) (1:2 ignored)
	Front:  (3:0, 2:0, 3:1, 2:5, 3:5, 2:8, 3:4, 2:4) (1:1 ignored)
	Back:   (3:3, 2:2, 3:2, 2:6, 3:6, 2:10, 3:7, 2:7) (1:3 ignored)

	Initially components are placed in components locations that match their
	Ids.
*/

#define SATURATE 1
#define NSF_CARDINALITY 1

#define REORDER_CUBE 0
#define FACED_ORDERING 0

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "meddly.h"
#include "meddly_expert.h"
#include "timer.h"

using namespace std;
using namespace MEDDLY;

typedef enum {F, B, L, R, U, D} face;
typedef enum {CW, CCW, FLIP} direction;

// level *variables = NULL;
int* sizes = NULL;
int** initst = NULL;

// Number of variables of each type
const int NUM_TYPE1 = 6;
const int NUM_TYPE2 = 12;
const int NUM_TYPE3 = 8;

// Number of levels
const int num_levels = NUM_TYPE2 + NUM_TYPE3;

// Domain handle
domain *d;

// Forest storing the next state function
// forest_hndl relation;
forest* relation;

// Forest storing the set of states
// forest_hndl states;
forest* states;

static int comp_type[] =
  {0, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2};
static int comp3_map[] = {13, 14, 15, 16, 17, 18, 19, 20};
static int comp2_map[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
static int order[] =
//  {0, 14, 13, 12, 15, 16, 17, 18, 19, 2, 1, 0, 3, 6, 5, 4, 8, 7, 10, 9, 11};
//  {0, 0, 19, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 1};
// 8
//  {0, 0, 13, 1, 14, 2, 15, 3, 16, 4, 17, 5, 18, 6, 7, 19, 8, 9, 20, 10, 11};
// 7
//  {0, 2, 9, 11, 16, 15, 4, 19, 12, 3, 10, 18, 17, 7, 5, 14, 8, 1, 13, 6, 0};
// 6 F , f, Ff {390,212: 9,170,252}
//  {0, 12, 13, 14, 15, 16, 17, 18, 19, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
// 5 F , f, Ff {388,171: 9,082,150} (<600M 250s FURLBD, <500M 520s furlbd)
//  {0, 14, 13, 12, 15, 18, 17, 16, 19, 2, 1, 0, 3, 6, 5, 4, 7, 10, 9, 8, 11};
// 4 F , f, Ff {454,671 : 7,944,522}
//  {0, 2, 1, 0, 3, 6, 5, 4, 7, 10, 9, 8, 11, 14, 13, 12, 15, 18, 17, 16, 19};
// 3 F , f, Ff {420,490 : 7,343,762} -- running -F -lFURLBDfurlbd (on crow)
// almost 1 week, 1.65 GB
//  {0, 2, 1, 0, 3, 6, 5, 4, 8, 7, 10, 9, 11, 14, 13, 12, 17, 16, 15, 19, 18};
// 2 F , f, Ff {422,643 : 7,406,322}
//  {0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
// 1, F , f, Ff {422,643 : 7,406,322} -- running -F -lFURLBDfurlbd (on gypsy)
// almost 1 day, 2.5 GB
  {0, 2, 1, 0, 3, 6, 5, 4, 8, 7, 10, 9, 11, 14, 13, 12, 15, 16, 17, 18, 19};
// Andy's PN
//  {0, 0, 1, 2, 3, 5, 7, 9, 11, 4, 6, 10, 8, 13, 12, 15, 14, 17, 16, 19, 18};
// best: 1, 2, 3,...

void SetUpArrays()
{
  // Set up comp_type arrays based on order[]
  for (int i = 1; i < num_levels + 1; i++) {
    comp_type[i] = order[i] > 11? 3: 2;
    if (comp_type[i] == 3) {
      comp3_map[order[i] - 12] = i;
    } else {
      comp2_map[order[i]] = i;
    }
  }

  for (int i = 0; i < 12; i++) {
    fprintf(stderr, "%d, ", comp2_map[i]);
  }
  fprintf(stderr, "\n");
  for (int i = 0; i < 8; i++) {
    fprintf(stderr, "%d, ", comp3_map[i]);
  }
  fprintf(stderr, "\n");
}

int get_component_type (int comp_level)
{
  assert(comp_level > 0 && comp_level <= num_levels);
  return comp_type[comp_level];
}

int get_component_size (int c_type)
{
  assert(c_type == 2 || c_type == 3);
  return (c_type == 3)? NUM_TYPE3: NUM_TYPE2;
}

int getLevelSize (int comp_level)
{
  return get_component_size(get_component_type(comp_level));
}

int get_component_level (int c_type, int index)
{
  assert(c_type == 2 || c_type == 3);
  assert((c_type == 3 && index >= 0 && index < 8) ||
      (c_type == 2 && index >= 0 && index < 12));

  return (c_type == 3)? comp3_map[index]: comp2_map[index];
}


void Init()
{
  assert(num_levels == (NUM_TYPE2 + NUM_TYPE3));

  // node size for each level
  sizes = (int *) malloc(num_levels * sizeof(int));
  assert(sizes != NULL);
  
  assert(num_levels == 20);
  for (int i = 0; i < num_levels; i++) {
    sizes[i] = getLevelSize(i+1);
    fprintf(stderr, "sizes[%d] = %d\n", i, sizes[i]);
  }
  fflush(stderr);

  // sets of states
  initst = (int**) malloc(1 * sizeof(int*));
  assert(initst != NULL);
  initst[0] = (int *) malloc((num_levels + 1) * sizeof(int));
  assert(initst[0] != NULL);
  // all start at state 0
  memset(initst[0], 0, (num_levels + 1) * sizeof(int));
}


void CheckVars(domain *d)
{
  // Make sure that the level handles are the same as the level heights.
  // If this is not the case, the program needs to be modified to be
  // more flexible.
  int nVars = d->getNumVariables();
  int currHeight = nVars;
  int topVar = d->getNumVariables();
  while (topVar > 0)
  {
    if (topVar != currHeight) {
      // complain
      exit(1);
    }
    topVar--;
    currHeight--;
  }
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


dd_edge BuildMoveHelper(
  int type3_a,
  int type2_a,
  int type3_b,
  int type2_b,
  int type3_c,
  int type2_c,
  int type3_d,
  int type2_d
  )
{
  // transform to levels
  int a2 = get_component_level(2, type2_a);
  int b2 = get_component_level(2, type2_b);
  int c2 = get_component_level(2, type2_c);
  int d2 = get_component_level(2, type2_d);
  int a3 = get_component_level(3, type3_a);
  int b3 = get_component_level(3, type3_b);
  int c3 = get_component_level(3, type3_c);
  int d3 = get_component_level(3, type3_d);

  fprintf(stderr, "type2_a, a2 = %d, %d\n", type2_a, a2);
  fprintf(stderr, "type2_b, b2 = %d, %d\n", type2_b, b2);
  fprintf(stderr, "type2_c, c2 = %d, %d\n", type2_c, c2);
  fprintf(stderr, "type2_d, d2 = %d, %d\n", type2_d, d2);
  fprintf(stderr, "type3_a, a3 = %d, %d\n", type3_a, a3);
  fprintf(stderr, "type3_b, b3 = %d, %d\n", type3_b, b3);
  fprintf(stderr, "type3_c, c3 = %d, %d\n", type3_c, c3);
  fprintf(stderr, "type3_d, d3 = %d, %d\n", type3_d, d3);

  // face is ordered like this:
  // type3, type2, type3, type2, type3, type2, type3, type2

  const int sz = num_levels + 1;

  // Adding all the elements in one go
  //
  // 4 type 2 additions = 4 * 12 = 48
  // 4 type 3 additions = 4 * 8 = 32
  // total additions = 4 * 12 + 4 * 8 = 4 * 20 = 80
  //
  int nElements = 4 * NUM_TYPE2 + 4 * NUM_TYPE3;
  assert(nElements == 80);
  int** from = (int**) malloc(nElements * sizeof(int *));
  int** to = (int**) malloc(nElements * sizeof(int *));
  for (int i = 0; i < nElements; i++)
  {
    // allocate elements
    from[i] = (int*) malloc(sz * sizeof(int));
    to[i] = (int*) malloc(sz * sizeof(int));

    // initialize elements
    from[i][0] = 0;
    to[i][0] = 0;
    SetIntArray(from[i] + 1, sz - 1, DONT_CARE);
    SetIntArray(to[i] + 1, sz - 1, DONT_CHANGE);
    to[i][a3] = DONT_CARE;
    to[i][b3] = DONT_CARE;
    to[i][c3] = DONT_CARE;
    to[i][d3] = DONT_CARE;
    to[i][a2] = DONT_CARE;
    to[i][b2] = DONT_CARE;
    to[i][c2] = DONT_CARE;
    to[i][d2] = DONT_CARE;
  }

  int currElement = 0;

  // a3' <- d3

  for (int i = 0; i < NUM_TYPE3; i++)
  {
    from[currElement][d3] = i;
    to[currElement][a3] = i;
    currElement++;
  }

  // b3' <- a3

  for (int i = 0; i < NUM_TYPE3; i++)
  {
    from[currElement][a3] = i;
    to[currElement][b3] = i;
    currElement++;
  }

  // c3' <- b3

  for (int i = 0; i < NUM_TYPE3; i++)
  {
    from[currElement][b3] = i;
    to[currElement][c3] = i;
    currElement++;
  }

  // d3' <- c3

  for (int i = 0; i < NUM_TYPE3; i++)
  {
    from[currElement][c3] = i;
    to[currElement][d3] = i;
    currElement++;
  }


  // a2' <- d2

  for (int i = 0; i < NUM_TYPE2; i++)
  {
    from[currElement][d2] = i;
    to[currElement][a2] = i;
    currElement++;
  }

  // b2' <- a2

  for (int i = 0; i < NUM_TYPE2; i++)
  {
    from[currElement][a2] = i;
    to[currElement][b2] = i;
    currElement++;
  }

  // c2' <- b2

  for (int i = 0; i < NUM_TYPE2; i++)
  {
    from[currElement][b2] = i;
    to[currElement][c2] = i;
    currElement++;
  }

  // d2' <- c2

  for (int i = 0; i < NUM_TYPE2; i++)
  {
    from[currElement][c2] = i;
    to[currElement][d2] = i;
    currElement++;
  }

  // compute result = union elements to create an mxd for each component
  // and then intersect the mxds.

  dd_edge result(relation);
  dd_edge temp(relation);
  int offset = 0;

  // a3' <- d3
  relation->createEdge(from + offset, to + offset, NUM_TYPE3, result);
  offset += NUM_TYPE3;

  // b3' <- a3
  relation->createEdge(from + offset, to + offset, NUM_TYPE3, temp);
  result *= temp;
  offset += NUM_TYPE3;

  // c3' <- b3
  relation->createEdge(from + offset, to + offset, NUM_TYPE3, temp);
  result *= temp;
  offset += NUM_TYPE3;

  // d3' <- c3
  relation->createEdge(from + offset, to + offset, NUM_TYPE3, temp);
  result *= temp;
  offset += NUM_TYPE3;

  // a2' <- d2
  relation->createEdge(from + offset, to + offset, NUM_TYPE2, temp);
  result *= temp;
  offset += NUM_TYPE2;

  // b2' <- a2
  relation->createEdge(from + offset, to + offset, NUM_TYPE2, temp);
  result *= temp;
  offset += NUM_TYPE2;

  // c2' <- b2
  relation->createEdge(from + offset, to + offset, NUM_TYPE2, temp);
  result *= temp;
  offset += NUM_TYPE2;

  // d2' <- c2
  relation->createEdge(from + offset, to + offset, NUM_TYPE2, temp);
  result *= temp;
  offset += NUM_TYPE2;

  assert(offset == nElements);

  // delete arrays
  for (int i = 0; i < nElements; i++)
  {
    free(from[i]);
    free(to[i]);
  }
  free(from);
  free(to);
  
  return result;
}


// Modified
dd_edge BuildFlipMoveHelper(
  int type3_a,
  int type2_a,
  int type3_b,
  int type2_b,
  int type3_c,
  int type2_c,
  int type3_d,
  int type2_d
  )
{
  // transform to levels
  int a2 = get_component_level(2, type2_a);
  int b2 = get_component_level(2, type2_b);
  int c2 = get_component_level(2, type2_c);
  int d2 = get_component_level(2, type2_d);
  int a3 = get_component_level(3, type3_a);
  int b3 = get_component_level(3, type3_b);
  int c3 = get_component_level(3, type3_c);
  int d3 = get_component_level(3, type3_d);

  fprintf(stderr, "type2_a, a2 = %d, %d\n", type2_a, a2);
  fprintf(stderr, "type2_b, b2 = %d, %d\n", type2_b, b2);
  fprintf(stderr, "type2_c, c2 = %d, %d\n", type2_c, c2);
  fprintf(stderr, "type2_d, d2 = %d, %d\n", type2_d, d2);
  fprintf(stderr, "type3_a, a3 = %d, %d\n", type3_a, a3);
  fprintf(stderr, "type3_b, b3 = %d, %d\n", type3_b, b3);
  fprintf(stderr, "type3_c, c3 = %d, %d\n", type3_c, c3);
  fprintf(stderr, "type3_d, d3 = %d, %d\n", type3_d, d3);

  // face is ordered like this:
  // type3, type2, type3, type2, type3, type2, type3, type2

  const int sz = num_levels + 1;

  // Adding all the elements in one go
  //
  // 4 type 2 additions = 4 * 12 = 48
  // 4 type 3 additions = 4 * 8 = 32
  // total additions = 4 * 12 + 4 * 8 = 4 * 20 = 80
  //
  int nElements = 4 * NUM_TYPE2 + 4 * NUM_TYPE3;
  assert(nElements == 80);
  int** from = (int**) malloc(nElements * sizeof(int *));
  int** to = (int**) malloc(nElements * sizeof(int *));
  for (int i = 0; i < nElements; i++)
  {
    // allocate elements
    from[i] = (int*) malloc(sz * sizeof(int));
    to[i] = (int*) malloc(sz * sizeof(int));

    // initialize elements
    from[i][0] = 0;
    to[i][0] = 0;
    SetIntArray(from[i] + 1, sz - 1, DONT_CARE);
    SetIntArray(to[i] + 1, sz - 1, DONT_CHANGE);
    to[i][a3] = DONT_CARE;
    to[i][b3] = DONT_CARE;
    to[i][c3] = DONT_CARE;
    to[i][d3] = DONT_CARE;
    to[i][a2] = DONT_CARE;
    to[i][b2] = DONT_CARE;
    to[i][c2] = DONT_CARE;
    to[i][d2] = DONT_CARE;
  }


  int currElement = 0;

  // a3' <- c3

  for (int i = 0; i < NUM_TYPE3; i++)
  {
    from[currElement][c3] = i;
    to[currElement][a3] = i;
    currElement++;
  }

  // b3' <- d3

  for (int i = 0; i < NUM_TYPE3; i++)
  {
    from[currElement][d3] = i;
    to[currElement][b3] = i;
    currElement++;
  }

  // c3' <- a3

  for (int i = 0; i < NUM_TYPE3; i++)
  {
    from[currElement][a3] = i;
    to[currElement][c3] = i;
    currElement++;
  }

  // d3' <- b3

  for (int i = 0; i < NUM_TYPE3; i++)
  {
    from[currElement][b3] = i;
    to[currElement][d3] = i;
    currElement++;
  }


  // a2' <- c2

  for (int i = 0; i < NUM_TYPE2; i++)
  {
    from[currElement][c2] = i;
    to[currElement][a2] = i;
    currElement++;
  }

  // b2' <- d2

  for (int i = 0; i < NUM_TYPE2; i++)
  {
    from[currElement][d2] = i;
    to[currElement][b2] = i;
    currElement++;
  }

  // c2' <- a2

  for (int i = 0; i < NUM_TYPE2; i++)
  {
    from[currElement][a2] = i;
    to[currElement][c2] = i;
    currElement++;
  }

  // d2' <- b2

  for (int i = 0; i < NUM_TYPE2; i++)
  {
    from[currElement][b2] = i;
    to[currElement][d2] = i;
    currElement++;
  }


  // compute result = union elements to create an mxd for each component
  // and then intersect the mxds.

  dd_edge result(relation);
  int offset = 0;

  // a3' <- c3
  {
    dd_edge temp(relation);
    relation->createEdge(from + offset, to + offset, NUM_TYPE3, temp);
    result = temp;
    offset += NUM_TYPE3;
  }

  // b3' <- d3
  // c3' <- a3
  // d3' <- b3
  for (int i = 0; i < 3; i++)
  {
    dd_edge temp(relation);
    relation->createEdge(from + offset, to + offset, NUM_TYPE3, temp);
    result *= temp;
    offset += NUM_TYPE3;
  }

  // a2' <- c2
  // b2' <- d2
  // c2' <- a2
  // d2' <- b2
  for (int i = 0; i < 4; i++)
  {
    dd_edge temp(relation);
    relation->createEdge(from + offset, to + offset, NUM_TYPE2, temp);
    result *= temp;
    offset += NUM_TYPE2;
  }

  assert(offset == nElements);

  // delete arrays
  for (int i = 0; i < nElements; i++)
  {
    free(from[i]);
    free(to[i]);
  }
  free(from);
  free(to);
  
  return result;
}


dd_edge BuildMove(face f, direction d) {
  dd_edge result(relation);
  switch (f) {
    case U:
      if (d == CW) {
        result = BuildMoveHelper(4, 5, 5, 6, 1, 0, 0, 4);
      } else if (d == CCW) {
        result = BuildMoveHelper(0, 4, 1, 0, 5, 6, 4, 5);
      } else {
        result = BuildFlipMoveHelper(4, 5, 5, 6, 1, 0, 0, 4);
      }
      break;
    case D:
      if (d == CW) {
        result =  BuildMoveHelper(3, 2, 2, 8, 6, 9, 7, 10);
      } else if (d == CCW) {
        result =  BuildMoveHelper(7, 10, 6, 9, 2, 8, 3, 2);
      } else {
        result =  BuildFlipMoveHelper(3, 2, 2, 8, 6, 9, 7, 10);
      }
      break;
    case L:
      if (d == CW) {
        result =  BuildMoveHelper(0, 3, 3, 10, 7, 11, 4, 4);
      } else if (d == CCW) {
        result =  BuildMoveHelper(4, 4, 7, 11, 3, 10, 0, 3);
      } else {
        result =  BuildFlipMoveHelper(0, 3, 3, 10, 7, 11, 4, 4);
      }
      break;
    case R:
      if (d == CW) {
        result =  BuildMoveHelper(1, 1, 5, 6, 6, 7, 2, 8);
      } else if (d == CCW) {
        result =  BuildMoveHelper(2, 8, 6, 7, 5, 6, 1, 1);
      } else {
        result =  BuildFlipMoveHelper(1, 1, 5, 6, 6, 7, 2, 8);
      }
      break;
    case F:
      if (d == CW) {
        result = BuildMoveHelper(0, 0, 1, 1, 2, 2, 3, 3);
      } else if (d == CCW) {
        result = BuildMoveHelper(3, 3, 2, 2, 1, 1, 0, 0);
      } else {
        result = BuildFlipMoveHelper(0, 0, 1, 1, 2, 2, 3, 3);
      }
      break;
    case B:
      if (d == CW) {
        result =  BuildMoveHelper(5, 5, 4, 11, 7, 9, 6, 7);
      } else if (d == CCW) {
        result =  BuildMoveHelper(6, 7, 7, 9, 4, 11, 5, 5);
      } else {
        result =  BuildFlipMoveHelper(5, 5, 4, 11, 7, 9, 6, 7);
      }
      break;
  }
  return result;
}


const char* face_to_string(face f){
  switch(f) {
    case F: return "Front";
    case B: return "Back";
    case L: return "Left";
    case R: return "Right";
    case U: return "Up";
    case D: return "Down";
    default: return "Invalid Face";
  }
}


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
  fprintf(stderr, "\n");
}


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


void buildOverallNextStateFunction(const moves& m,
    satpregen_opname::pregen_relation& ensf, bool split)
{
  // Build each move using BuildMove().
  
  // Clock-wise moves
  if (m.FCW) ensf.addToRelation(BuildMove(F, CW));
  if (m.BCW) ensf.addToRelation(BuildMove(B, CW));
  if (m.UCW) ensf.addToRelation(BuildMove(U, CW));
  if (m.DCW) ensf.addToRelation(BuildMove(D, CW));
  if (m.LCW) ensf.addToRelation(BuildMove(L, CW));
  if (m.RCW) ensf.addToRelation(BuildMove(R, CW));

  // Counter clock-wise moves
  if (m.FCCW) ensf.addToRelation(BuildMove(F, CCW));
  if (m.BCCW) ensf.addToRelation(BuildMove(B, CCW));
  if (m.UCCW) ensf.addToRelation(BuildMove(U, CCW));
  if (m.DCCW) ensf.addToRelation(BuildMove(D, CCW));
  if (m.LCCW) ensf.addToRelation(BuildMove(L, CCW));
  if (m.RCCW) ensf.addToRelation(BuildMove(R, CCW));

  // Flip moves
  if (m.FF) ensf.addToRelation(BuildMove(F, FLIP));
  if (m.BF) ensf.addToRelation(BuildMove(B, FLIP));
  if (m.UF) ensf.addToRelation(BuildMove(U, FLIP));
  if (m.DF) ensf.addToRelation(BuildMove(D, FLIP));
  if (m.LF) ensf.addToRelation(BuildMove(L, FLIP));
  if (m.RF) ensf.addToRelation(BuildMove(R, FLIP));

  if (split) {
    //ensf.finalize(satpregen_opname::pregen_relation::SplitOnly);
    //ensf.finalize(satpregen_opname::pregen_relation::SplitSubtract);
    ensf.finalize(satpregen_opname::pregen_relation::SplitSubtractAll);
  } else {
    ensf.finalize(satpregen_opname::pregen_relation::MonolithicSplit);
  }
}


dd_edge buildOverallNextStateFunction(const moves& m)
{
  // Build each move using BuildMove() and combine using union.
  
  dd_edge result(relation);
  dd_edge temp(relation);

  // Clock-wise moves
  if (m.FCW) {
    temp = BuildMove(F, CW);
    result += temp;
  }

  if (m.BCW) {
    temp = BuildMove(B, CW);
    result += temp;
  }

  if (m.UCW) {
    temp = BuildMove(U, CW);
    result += temp;
  }

  if (m.DCW) {
    temp = BuildMove(D, CW);
    result += temp;
  }

  if (m.LCW) {
    temp = BuildMove(L, CW);
    result += temp;
  }

  if (m.RCW) {
    temp = BuildMove(R, CW);
    result += temp;
  }


  // Counter clock-wise moves
  if (m.FCCW) {
    temp = BuildMove(F, CCW);
    result += temp;
  }

  if (m.BCCW) {
    temp = BuildMove(B, CCW);
    result += temp;
  }

  if (m.UCCW) {
    temp = BuildMove(U, CCW);
    result += temp;
  }

  if (m.DCCW) {
    temp = BuildMove(D, CCW);
    result += temp;
  }

  if (m.LCCW) {
    temp = BuildMove(L, CCW);
    result += temp;
  }

  if (m.RCCW) {
    temp = BuildMove(R, CCW);
    result += temp;
  }


  // Flip moves
  if (m.FF) {
    temp = BuildMove(F, FLIP);
    result += temp;
  }

  if (m.BF) {
    temp = BuildMove(B, FLIP);
    result += temp;
  }

  if (m.UF) {
    temp = BuildMove(U, FLIP);
    result += temp;
  }

  if (m.DF) {
    temp = BuildMove(D, FLIP);
    result += temp;
  }

  if (m.LF) {
    temp = BuildMove(L, FLIP);
    result += temp;
  }

  if (m.RF) {
    temp = BuildMove(R, FLIP);
    result += temp;
  }

  return result;
}


int doBfs(const moves& m)
{
  // Build overall Next-State Function.
  dd_edge nsf = buildOverallNextStateFunction(m);

  // Build initial state.
  assert(states);
  dd_edge initial(states);
  states->createEdge(initst, 1, initial);

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
  satpregen_opname::pregen_relation *ensf = 0;
  dd_edge nsf(relation);

  // Build initial state.
  assert(states);
  dd_edge initial(states);
  states->createEdge(initst, 1, initial);

  timer start;

  if (saturation_type == 'e') {
    ensf = new satpregen_opname::pregen_relation(states, relation, states,
        m.countEnabledMoves());
    buildOverallNextStateFunction(m, *ensf, split);
    printf("Building reachability set using saturation, relation by events\n");
    fflush(stdout);
  } else if (saturation_type == 'k') {
    ensf = new satpregen_opname::pregen_relation(states, relation, states);
    buildOverallNextStateFunction(m, *ensf, split);
    printf("Building reachability set using saturation, relation by levels");
    if (split) printf(" with splitting");
    printf("\n");
    fflush(stdout);
  } else {
    nsf = buildOverallNextStateFunction(m);
    printf("Building reachability set using saturation, monolithic relation\n");
    fflush(stdout);
  }

  start.note_time();

  // Perform Reacability via "saturation".
  if (ensf) {
    if (0==SATURATION_FORWARD) throw error(error::UNKNOWN_OPERATION);
    specialized_operation *sat = SATURATION_FORWARD->buildOperation(ensf);
    if (0==sat) throw error(error::INVALID_OPERATION);
    sat->compute(initial, initial);
  } else {
    apply(REACHABLE_STATES_DFS, initial, nsf, initial);
  }

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


int doChoice()
{
  // Build Next-State Function for each "move".
  // Moves 0-5 are CW, 6-11 are CCW, 12-17 are FLIP.
  // Order within each set: F, B, L, R, U, D.

  const int nFaces = 6;
  vector<dd_edge> nsf;

#if 1

  // Clock-wise moves
  nsf.push_back(BuildMove(F, CW));
  nsf.push_back(BuildMove(B, CW));
  nsf.push_back(BuildMove(L, CW));
  nsf.push_back(BuildMove(R, CW));
  nsf.push_back(BuildMove(U, CW));
  nsf.push_back(BuildMove(D, CW));
  // Counter Clock-wise moves
  nsf.push_back(BuildMove(F, CCW));
  nsf.push_back(BuildMove(B, CCW));
  nsf.push_back(BuildMove(L, CCW));
  nsf.push_back(BuildMove(R, CCW));
  nsf.push_back(BuildMove(U, CCW));
  nsf.push_back(BuildMove(D, CCW));
  // Flip moves
  nsf.push_back(BuildMove(F, FLIP));
  nsf.push_back(BuildMove(B, FLIP));
  nsf.push_back(BuildMove(L, FLIP));
  nsf.push_back(BuildMove(R, FLIP));
  nsf.push_back(BuildMove(U, FLIP));
  nsf.push_back(BuildMove(D, FLIP));

#endif

  // Perform reachability via user-interaction.
  // Display menu.
  // Perform user command.
  // Repeat.

  // Display menu and get choice
  assert(states);

  dd_edge initial(states);
  states->createEdge(initst, 1, initial);

  dd_edge result = initial;
  dd_edge temp(states);
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
        states->garbageCollect();
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


int doSteppedBfs()
{
  // Performs DFS for each event starting with initial state.
  // At the end of each iteration, peform a union and repeat.

  // Build Next-State Function for each "move".
  // Moves 0-5 are CW, 6-11 are CCW, 12-17 are FLIP.
  // Order within each set: F, B, L, R, U, D.

  const int nFaces = 6;
  vector<dd_edge> nsf;

  // Clock-wise moves
  nsf.push_back(BuildMove(F, CW));
  nsf.push_back(BuildMove(B, CW));
  nsf.push_back(BuildMove(L, CW));
  nsf.push_back(BuildMove(R, CW));
  nsf.push_back(BuildMove(U, CW));
  nsf.push_back(BuildMove(D, CW));

  // Perform reachability via user-interaction.
  // Display menu.
  // Perform user command.
  // Repeat.

  // Display menu and get choice
  assert(states);

  dd_edge initial(states);
  states->createEdge(initst, 1, initial);

  dd_edge result = initial;
  dd_edge temp(states);
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
        states->garbageCollect();
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


int main(int argc, char *argv[])
{
  moves enabled;
  bool dfs = false;
  bool bfs = false;
  char saturation_type = 'm';
  bool split = true;

  if (argc > 1) {
    assert(argc <= 5);
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

  if (dfs) { bfs = false; }

  // set up arrays based on number of levels
  SetUpArrays();
  Init();

  // Initialize MEDDLY
  MEDDLY::initializer_list* L = defaultInitializerList(0);
  ct_initializer::setBuiltinStyle(ct_initializer::MonolithicChainedHash);
  ct_initializer::setMaxSize(16 * 16777216);
  // ct_initializer::setStaleRemoval(ct_initializer::Lazy);
  // ct_initializer::setStaleRemoval(ct_initializer::Moderate);
  // ct_initializer::setStaleRemoval(ct_initializer::Aggressive);
  MEDDLY::initialize(L);

  // Set up the state variables, as described earlier
  d = createDomainBottomUp(sizes, num_levels);
  if (NULL == d) {
    fprintf(stderr, "Couldn't create domain\n");
    return 1;
  }
  CheckVars(d);

  int topVar = d->getNumVariables();
  while (topVar > 0)
  {
    printf("level %d, size %d\n", topVar,
        static_cast<expert_domain*>(d)->getVariableBound(topVar));
    topVar--;
  }

  // Create forests
  states = d->createForest(false, forest::BOOLEAN, forest::MULTI_TERMINAL);
  relation = d->createForest(true, forest::BOOLEAN, forest::MULTI_TERMINAL);

  if (NULL == states) {
    fprintf(stderr, "Couldn't create forest of states\n");
    return 1;
  } else {
    fprintf(stderr, "Created forest of states\n");
  }
  if (NULL == relation) {
    fprintf(stderr, "Couldn't create forest of relations\n");
    return 1;
  } else {
    fprintf(stderr, "Created forest of relations\n");
  }

  // Build set of initial states

  for (int j = 0; j < NUM_TYPE2; j++) {
    initst[0][get_component_level(2, j)] = j;
  }
  for (int j = 0; j < NUM_TYPE3; j++) {
    initst[0][get_component_level(3, j)] = j;
  }
  initst[0][0] = 0;

  dd_edge initial(states);
  states->createEdge(initst, 1, initial);

  if (dfs) {
    doDfs(enabled, saturation_type, split);
  } else if (bfs) {
    doBfs(enabled);
  } else {
#if 0
    doChoice();
#else
    doSteppedBfs();
#endif
  }

  destroyDomain(d);
  cleanup();
  fprintf(stderr, "\n\nDONE\n");
  return 0;
}


