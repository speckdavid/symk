
// $Id: rubiks_cube_phase.cc 721 2016-09-30 20:12:24Z cjiang1209 $

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

	Up:     In order (going right starting from left-upper corner)
          of components (type:id:var),
          (3:4:17, 2:5:6, 3:5:18, 2:6:7, 3:1:14, 2:0:1, 3:0:13, 2:4:5)
          Note: (1:0) also belongs to this row but since it stays in the
          same location when this row is moved left or right, it is ignored.
	Down:   (3:3:16, 2:2:3, 3:2:15, 2:8:9, 3:6:19, 2:9:10, 3:7:20, 2:10:11) (1:5 ignored)
	Left:   (3:4:17, 2:4:5, 3:0:13, 2:3:4, 3:3:16, 2:10:11, 3:7:20, 2:11:12) (1:4 ignored)
	Right:  (3:1:14, 2:6:7, 3:5:18, 2:7:8, 3:6:19, 2:8:9, 3:2:15, 2:1:2) (1:2 ignored)
	Front:  (3:0:13, 2:0:1, 3:1:14, 2:1:2, 3:2:15, 2:2:3, 3:3:16, 2:3:4) (1:1 ignored)
	Back:   (3:5:18, 2:5:6, 3:4:17, 2:11:12, 3:7:20, 2:9:10, 3:6:19, 2:7:8) (1:3 ignored)

	Initially components are placed in components locations that match their
	Ids.
*/

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <cassert>

#include "meddly.h"
#include "meddly_expert.h"
#include "timer.h"

using namespace std;
using namespace MEDDLY;

enum class Face { F, B, L, R, U, D };
enum class Direction { CW, CCW, FLIP };

// Number of variables of each type
const int NUM_TYPE1 = 6;
const int NUM_TYPE2 = 12;
const int NUM_TYPE3 = 8;

struct Move
{
  Face face;
  Direction direction;
};

struct Component
{
  int type;
  int id;
};

enum class RubiksCubeModelType
{
  DEFAULT,
  CORNER_ONLY
};

struct RubiksCubeModelConfig
{
  vector<int> comp_type;
  vector<int> comp3_map;
  vector<int> comp2_map;

  int num_phases;
  int num_type2;
  int num_type3;

  vector<vector<Component>> level2components;
  vector<vector<Move>> moves;

  bool reorder_relation;

  int num_components() const {
    return num_type2 + num_type3;
  }
};

RubiksCubeModelConfig buildModelConfig(int num_phases, RubiksCubeModelType type)
{
  RubiksCubeModelConfig config;

  if (num_phases == 1) {
    config.num_phases = 1;
    config.moves = {
      {
        { Face::F, Direction::CW },
        { Face::B, Direction::CW },
        { Face::L, Direction::CW },
        { Face::R, Direction::CW },
        { Face::U, Direction::CW },
        { Face::D, Direction::CW }
      }
    };
  }
  else if (num_phases == 2) {
    config.num_phases = 2;
    config.moves = {
      {
        { Face::F, Direction::CW },
        { Face::L, Direction::CW },
        { Face::U, Direction::CW }
      },
      {
        { Face::B, Direction::CW },
        { Face::R, Direction::CW },
        { Face::D, Direction::CW }
      }
    };
  }
  else if (num_phases == 3) {
    config.num_phases = 3;
    config.moves = {
      {
        { Face::F, Direction::CW },
        { Face::B, Direction::CW }
      },
      {
        { Face::L, Direction::CW },
        { Face::R, Direction::CW }
      },
      {
        { Face::U, Direction::CW },
        { Face::D, Direction::CW }
      }
    };
  }
  else if (num_phases == 6) {
    config.num_phases = 6;
    config.moves = {
      {
        { Face::F, Direction::CW }
      },
      {
        { Face::B, Direction::CW }
      },
      {
        { Face::L, Direction::CW }
      },
      {
        { Face::R, Direction::CW }
      },
      {
        { Face::U, Direction::CW }
      },
      {
        { Face::D, Direction::CW }
      }
    };
  }
  else {
    exit(1);
  }

  if (type == RubiksCubeModelType::DEFAULT) {
    config.num_type2 = NUM_TYPE2;
    config.num_type3 = NUM_TYPE3;
    config.comp_type = { 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3 };
    config.comp2_map = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
    config.comp3_map = { 13, 14, 15, 16, 17, 18, 19, 20 };
  }
  else if (type == RubiksCubeModelType::CORNER_ONLY) {
    config.num_type2 = 0;
    config.num_type3 = NUM_TYPE3;
    config.comp_type = { 0, 3, 3, 3, 3, 3, 3, 3, 3 };
    config.comp2_map = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    config.comp3_map = { 1, 2, 3, 4, 5, 6, 7, 8 };
  }
  else {
    exit(1);
  }

  if (num_phases == 1 && type == RubiksCubeModelType::DEFAULT) {
    config.level2components = {
      {
        // Not used
        { 0, 0 },
        { 2, 2 }, { 2, 1 }, { 2, 0 }, { 2, 3 },
        { 2, 6 }, { 2, 5 }, { 2, 4 }, { 2, 8 },
        { 2, 7 }, { 2, 10 }, { 2, 9 }, { 2, 11 },
        { 3, 2 }, { 3, 1 }, { 3, 0 }, { 3, 3 },
        { 3, 4 }, { 3, 5 }, { 3, 6 }, { 3, 7 }
      },
    };
  }
  else if (num_phases == 1 && type == RubiksCubeModelType::CORNER_ONLY) {
    config.level2components = {
      {
        // Not used
        { 0, 0 },
        { 3, 2 }, { 3, 1 }, { 3, 0 }, { 3, 3 },
        { 3, 4 }, { 3, 5 }, { 3, 6 }, { 3, 7 }
      },
    };
  }
  else if (num_phases == 2 && type == RubiksCubeModelType::DEFAULT) {
    config.level2components = {
      {
        // Not used
        { 0, 0 },
        // Front, Up and Right
        { 3, 0 }, { 3, 1 }, { 3, 3 }, { 3, 4 },
        { 3, 2 }, { 3, 5 }, { 3, 7 },
        { 2, 0 }, { 2, 3 }, { 2, 4 },
        { 2, 1 }, { 2, 2 },
        { 2, 6 }, { 2, 5 },
        { 2, 10 }, { 2, 11 },
        // Not affected
        { 3, 6 },
        { 2, 7 }, { 2, 8 }, { 2, 9 }
      },
      {
        // Not used
        { 0, 0 },
        // Back, Down and Left
        { 3, 6 }, { 3, 2 }, { 3, 5 }, { 3, 7 },
        { 3, 1 }, { 3, 3 }, { 3, 4 },
        { 2, 7 }, { 2, 8 }, { 2, 9 },
        { 2, 1 }, { 2, 6 },
        { 2, 2 }, { 2, 10 },
        { 2, 5 }, { 2, 11 },
        // Not affected
        { 3, 0 },
        { 2, 0 }, { 2, 3 }, { 2, 4 }
      }
    };
  }
  else if (num_phases == 2 && type == RubiksCubeModelType::CORNER_ONLY) {
    config.level2components = {
      {
        // Not used
        { 0, 0 },
        // Front, Up and Right
        { 3, 0 }, { 3, 1 }, { 3, 3 }, { 3, 4 },
        { 3, 2 }, { 3, 5 }, { 3, 7 },
        // Not affected
        { 3, 6 }
      },
      {
        // Not used
        { 0, 0 },
        // Back, Down and Left
        { 3, 6 }, { 3, 2 }, { 3, 5 }, { 3, 7 },
        { 3, 1 }, { 3, 3 }, { 3, 4 },
        // Not affected
        { 3, 0 }
      }
    };
  }
  else if (num_phases == 3 && type == RubiksCubeModelType::DEFAULT) {
    config.level2components = {
      {
        // Not used
        { 0, 0 },
        // Front
        { 3, 0 }, { 3, 1 }, { 3, 2 }, { 3, 3 },
        { 2, 0 }, { 2, 1 }, { 2, 2 }, { 2, 3 },
        // Back
        { 3, 5 }, { 3, 4 }, { 3, 7 }, { 3, 6 },
        { 2, 5 }, { 2, 11}, { 2, 9 }, { 2, 7 },
        // Not affected
        { 2, 4 }, { 2, 10 }, { 2, 8 }, { 2, 6 }
      },
      {
        // Not used
        { 0, 0 },
        // Left
        { 3, 4 }, { 3, 0 }, { 3, 3 }, { 3, 7 },
        { 2, 4 }, { 2, 3 }, { 2, 10 }, { 2, 11 },
        // Right
        { 3, 1 }, { 3, 5 }, { 3, 6 }, { 3, 2 },
        { 2, 6 }, { 2, 7 }, { 2, 8 }, { 2, 1 },
        // Not affected
        { 2, 5 }, { 2, 9 }, { 2, 2 }, { 2, 0 }
      },
      {
        // Not used
        { 0, 0 },
        // Up
        { 3, 4 }, { 3, 5 }, { 3, 1 }, { 3, 0 },
        { 2, 5 }, { 2, 6 }, { 2, 0 }, { 2, 4 },
        // Down
        { 3, 6 }, { 3, 7 }, { 3, 3 }, { 3, 2 },
        { 2, 9 }, { 2, 10 }, { 2, 2 }, { 2, 8 },
        // Not affected
        { 2, 11 }, { 2, 3 }, { 2, 1 }, { 2, 7 }
      }
    };
  }
  else if (num_phases == 3 && type == RubiksCubeModelType::CORNER_ONLY) {
    config.level2components = {
      {
        // Not used
        { 0, 0 },
        // Front
        { 3, 0 }, { 3, 1 }, { 3, 2 }, { 3, 3 },
        // Back
        { 3, 5 }, { 3, 4 }, { 3, 7 }, { 3, 6 },
      },
      {
        // Not used
        { 0, 0 },
        // Left
        { 3, 4 }, { 3, 0 }, { 3, 3 }, { 3, 7 },
        // Right
        { 3, 1 }, { 3, 5 }, { 3, 6 }, { 3, 2 },
      },
      {
        // Not used
        { 0, 0 },
        // Up
        { 3, 4 }, { 3, 5 }, { 3, 1 }, { 3, 0 },
        // Down
        { 3, 6 }, { 3, 7 }, { 3, 3 }, { 3, 2 },
      }
    };
  }
  else if (num_phases == 6 && type == RubiksCubeModelType::DEFAULT) {
    config.level2components = {
      {
        // Not used
        { 0, 0 },
        // Front
        { 3, 0 }, { 3, 1 }, { 3, 2 }, { 3, 3 },
        { 2, 0 }, { 2, 1 }, { 2, 2 }, { 2, 3 },
        // Back
        { 3, 5 }, { 3, 4 }, { 3, 7 }, { 3, 6 },
        { 2, 5 }, { 2, 11}, { 2, 9 }, { 2, 7 },
        // Not affected
        { 2, 4 }, { 2, 10 }, { 2, 8 }, { 2, 6 }
      },
      {
        // Not used
        { 0, 0 },
        // Back
        { 3, 5 }, { 3, 4 }, { 3, 7 }, { 3, 6 },
        { 2, 5 }, { 2, 11}, { 2, 9 }, { 2, 7 },
        // Front
        { 3, 0 }, { 3, 1 }, { 3, 2 }, { 3, 3 },
        { 2, 0 }, { 2, 1 }, { 2, 2 }, { 2, 3 },
        // Not affected
        { 2, 4 }, { 2, 10 }, { 2, 8 }, { 2, 6 }
      },
      {
        // Not used
        { 0, 0 },
        // Left
        { 3, 4 }, { 3, 0 }, { 3, 3 }, { 3, 7 },
        { 2, 4 }, { 2, 3 }, { 2, 10 }, { 2, 11 },
        // Right
        { 3, 1 }, { 3, 5 }, { 3, 6 }, { 3, 2 },
        { 2, 6 }, { 2, 7 }, { 2, 8 }, { 2, 1 },
        // Not affected
        { 2, 5 }, { 2, 9 }, { 2, 2 }, { 2, 0 }
      },
      {
        // Not used
        { 0, 0 },
        // Right
        { 3, 1 }, { 3, 5 }, { 3, 6 }, { 3, 2 },
        { 2, 6 }, { 2, 7 }, { 2, 8 }, { 2, 1 },
        // Left
        { 3, 4 }, { 3, 0 }, { 3, 3 }, { 3, 7 },
        { 2, 4 }, { 2, 3 }, { 2, 10 }, { 2, 11 },
        // Not affected
        { 2, 5 }, { 2, 9 }, { 2, 2 }, { 2, 0 }
      },
      {
        // Not used
        { 0, 0 },
        // Up
        { 3, 4 }, { 3, 5 }, { 3, 1 }, { 3, 0 },
        { 2, 5 }, { 2, 6 }, { 2, 0 }, { 2, 4 },
        // Down
        { 3, 6 }, { 3, 7 }, { 3, 3 }, { 3, 2 },
        { 2, 9 }, { 2, 10 }, { 2, 2 }, { 2, 8 },
        // Not affected
        { 2, 11 }, { 2, 3 }, { 2, 1 }, { 2, 7 }
      },
      {
        // Not used
        { 0, 0 },
        // Down
        { 3, 6 }, { 3, 7 }, { 3, 3 }, { 3, 2 },
        { 2, 9 }, { 2, 10 }, { 2, 2 }, { 2, 8 },
        // Up
        { 3, 4 }, { 3, 5 }, { 3, 1 }, { 3, 0 },
        { 2, 5 }, { 2, 6 }, { 2, 0 }, { 2, 4 },
        // Not affected
        { 2, 11 }, { 2, 3 }, { 2, 1 }, { 2, 7 }
      }
    };
  }
  else if (num_phases == 6 && type == RubiksCubeModelType::CORNER_ONLY) {
    config.level2components = {
      {
        // Not used
        { 0, 0 },
        // Front
        { 3, 0 }, { 3, 1 }, { 3, 2 }, { 3, 3 },
        // Back
        { 3, 5 }, { 3, 4 }, { 3, 7 }, { 3, 6 },
      },
      {
        // Not used
        { 0, 0 },
        // Back
        { 3, 5 }, { 3, 4 }, { 3, 7 }, { 3, 6 },
        // Front
        { 3, 0 }, { 3, 1 }, { 3, 2 }, { 3, 3 },
      },
      {
        // Not used
        { 0, 0 },
        // Left
        { 3, 4 }, { 3, 0 }, { 3, 3 }, { 3, 7 },
        // Right
        { 3, 1 }, { 3, 5 }, { 3, 6 }, { 3, 2 },
      },
      {
        // Not used
        { 0, 0 },
        // Right
        { 3, 1 }, { 3, 5 }, { 3, 6 }, { 3, 2 },
        // Left
        { 3, 4 }, { 3, 0 }, { 3, 3 }, { 3, 7 },
      },
      {
        // Not used
        { 0, 0 },
        // Up
        { 3, 4 }, { 3, 5 }, { 3, 1 }, { 3, 0 },
        // Down
        { 3, 6 }, { 3, 7 }, { 3, 3 }, { 3, 2 },
      },
      {
        // Not used
        { 0, 0 },
        // Down
        { 3, 6 }, { 3, 7 }, { 3, 3 }, { 3, 2 },
        // Up
        { 3, 4 }, { 3, 5 }, { 3, 1 }, { 3, 0 },
      }
    };
  }
  else {
    exit(1);
  }

  config.reorder_relation = false;

  assert(config.num_phases == config.moves.size());
  assert(config.num_phases == config.level2components.size());

  return config;
}

class RubiksCubeModel
{
private:
  const RubiksCubeModelConfig& _config;

  // Domain handle
  domain* _domain;

  // Forest storing the next state function
  vector<forest*> _relations;

  // Forest storing the set of states
  forest* _state;

  int _phase;

  int get_component_type(int var)
  {
    assert(var >= 1 && var <= _config.num_components());
    return _config.comp_type[var];
  }

  int get_component_size(int type)
  {
    assert(type == 2 || type == 3);
    return (type == 3) ? NUM_TYPE3 : NUM_TYPE2;
  }

  int get_var_of_component(int c_type, int index)
  {
    assert(c_type == 2 || c_type == 3);
    assert((c_type == 3 && index >= 0 && index < NUM_TYPE3) ||
        (c_type == 2 && index >= 0 && index < NUM_TYPE2));

    return (c_type == 3) ? _config.comp3_map[index] : _config.comp2_map[index];
  }

  int getVarSize(int var)
  {
    return get_component_size(get_component_type(var));
  }

  void buildDomain();
  void destroy();
  void buildForests();
  vector<int> convertToVariableOrder(const vector<Component>& level2component);
  dd_edge buildInitial();
  vector<dd_edge> buildNextStateFunctions();

  dd_edge buildMove(forest* relation, Face f, Direction d);
  dd_edge buildMoveHelper(forest* relation, int type3_a, int type2_a,
      int type3_b, int type2_b, int type3_c, int type2_c, int type3_d,
      int type2_d);
  dd_edge buildFlipMoveHelper(forest* relation, int type3_a, int type2_a,
      int type3_b, int type2_b, int type3_c, int type2_c, int type3_d,
      int type2_d);

  void compute_intermediate_order(const int* state_level2var, const int* rel_level2var, int* level2var);

  void execute_phase(const dd_edge& initial, const dd_edge& nsf, dd_edge& result);

  void show_node(const dd_edge& e);

public:
  RubiksCubeModel(const RubiksCubeModelConfig& config)
    : _config(config),
      _domain(nullptr), _state(nullptr),
      _phase(0)
  {
  }

  void execute();
};

void RubiksCubeModel::buildDomain()
{
  int* sizes = new int[_config.num_components()];
  assert(sizes != nullptr);
  for (int i = 0; i < _config.num_components(); i++) {
    sizes[i] = getVarSize(i + 1);
  }

  _domain = createDomainBottomUp(sizes, _config.num_components());
  if (_domain == nullptr) {
    cerr << "Couldn't create domain" << endl;
    exit(1);
  }

  delete[] sizes;
}

void RubiksCubeModel::destroy()
{
  destroyDomain(_domain);
}

void RubiksCubeModel::buildForests()
{
  forest::policies p(false);
  p.setSinkDown();

  _state = _domain->createForest(false, forest::BOOLEAN, forest::MULTI_TERMINAL, p);
  if (_state == nullptr) {
    cerr << "Couldn't create forest of states" << endl;
    exit(1);
  }
  cout << "Created forest of states" << endl;

  for (int i = 0; i < _config.num_phases; i++) {
    forest* relation = _domain->createForest(true, forest::BOOLEAN, forest::MULTI_TERMINAL);
    if (relation == nullptr) {
      cerr << "Couldn't create forest of relations" << endl;
      exit(1);
    }

    _relations.push_back(relation);
  }
  cout << "Created forest of relations" << endl;
}

vector<int> RubiksCubeModel::convertToVariableOrder(const vector<Component>& level2component)
{
  assert(level2component.size() - 1 == _config.num_components());

  vector<int> level2var;
  level2var.push_back(0);
  for (int i = 1; i <= _config.num_components(); i++) {
    const Component& comp = level2component[i];
    level2var.push_back(get_var_of_component(comp.type, comp.id));
  }
  return level2var;
}

dd_edge RubiksCubeModel::buildInitial()
{
  int** initst = new int*[1];
  initst[0] = new int[_config.num_components() + 1];
  initst[0][0] = 0;
  for (int j = 0; j < NUM_TYPE2; j++) {
    initst[0][get_var_of_component(2, j)] = j;
  }
  for (int j = 0; j < NUM_TYPE3; j++) {
    initst[0][get_var_of_component(3, j)] = j;
  }

  assert(_state != nullptr);

  dd_edge initial(_state);
  _state->createEdge(initst, 1, initial);

  delete[] initst[0];
  delete[] initst;

  return initial;
}

vector<dd_edge> RubiksCubeModel::buildNextStateFunctions()
{
  vector<vector<int>> level2vars;
  for (int i = 0; i< _config.num_phases; i++) {
    level2vars.push_back(convertToVariableOrder(_config.level2components[i]));
  }

  assert(_relations.size() == _config.num_phases);

  // Reorder the relations
  for (int i = 0; i < _config.num_phases; i++) {
    static_cast<expert_forest*>(_relations[i])->reorderVariables(level2vars[i].data());
  }

  vector<dd_edge> nsfs;
  for (int i = 0; i < _config.num_phases; i++) {
    assert(_relations[i] != nullptr);

    dd_edge nsf(_relations[i]);
    for (const auto& move : _config.moves[i]) {
      nsf += buildMove(_relations[i], move.face, move.direction);
    }
    nsfs.push_back(nsf);

    show_node(nsf);
  }
  return nsfs;
}

void RubiksCubeModel::compute_intermediate_order(const int* state_level2var, const int* rel_level2var, int* level2var)
{
//  int num_fixed = 8;
//  vector<bool> is_fixed(num_fixed + 1, false);
//  level2var[0] = 0;
//  for (int i = 1; i <= num_fixed; i++) {
//    is_fixed[rel_level2var[i]] = true;
//    level2var[i] = rel_level2var[i];
//  }
//
//  int idx = num_fixed + 1;
//  for (int i = 1; i <= type2 + type3; i++) {
//    if (!is_fixed[state_level2var[i]]) {
//      level2var[idx++] = state_level2var[i];
//    }
//  }

  // TODO
  std::copy(&rel_level2var[0], &rel_level2var[_config.num_components() + 1], level2var);
}

void RubiksCubeModel::execute()
{
  _phase = 0;

  buildDomain();
  buildForests();

  dd_edge initial = buildInitial();
  vector<dd_edge> nsfs = buildNextStateFunctions();

  timer start;
  start.note_time();

  // Fixed point
  int fp = 0;
  while (fp != _config.num_phases) {
    cout << "-------------------------------------------------------------------" << endl;
    cout << "Phase " << _phase << endl;

    dd_edge result = initial;
    execute_phase(initial, nsfs[_phase % _config.num_phases], result);
    _phase++;

    if (result != initial) {
      fp = 0;
      initial = result;
    }
    else {
      fp++;
    }

    show_node(initial);
  }

  start.note_time();

  cout << "Done" << endl;
  cout << "Time for constructing reachability set: "
      << static_cast<double>(start.get_last_interval()) / 1000000.0 << " s"
      << endl;
  cout << "# Reachable states: " << initial.getCardinality() << endl,

  destroy();
}

void RubiksCubeModel::execute_phase(const dd_edge& initial, const dd_edge& nsf, dd_edge& result)
{
  expert_forest* relation = static_cast<expert_forest*>(nsf.getForest());
  int* rel_level2var = new int[relation->getNumVariables() + 1];
  relation->getVariableOrder(rel_level2var);

  timer start;
  start.note_time();

  cout << "Reordering..." << endl;

  expert_forest* state = static_cast<expert_forest*>(initial.getForest());

  if (_config.reorder_relation) {
    int* state_level2var = new int[relation->getNumVariables() + 1];
    state->getVariableOrder(state_level2var);

    int* level2var = new int[relation->getNumVariables() + 1];
    compute_intermediate_order(state_level2var, rel_level2var, level2var);
    relation->reorderVariables(level2var);

    delete[] rel_level2var;
    delete[] state_level2var;
    delete[] level2var;
  }
  else {
    state->reorderVariables(rel_level2var);
    delete[] rel_level2var;
  }

  start.note_time();
  cout << "Time: "
      << static_cast<double>(start.get_last_interval()) / 1000000.0 << " s"
      << endl;

  cout << "Computing the reachable states..." << endl;
  show_node(initial);

  start.note_time();
  apply(REACHABLE_STATES_DFS, initial, nsf, result);
  start.note_time();

  cout << "Time: "
      << static_cast<double>(start.get_last_interval()) / 1000000.0 << " s"
      << endl;
}

dd_edge RubiksCubeModel::buildMove(forest* relation, Face f, Direction d)
{
  assert(relation->isForRelations());

  dd_edge result(relation);
  switch (f) {
    case Face::U:
      if (d == Direction::CW) {
        result = buildMoveHelper(relation, 4, 5, 5, 6, 1, 0, 0, 4);
      } else if (d == Direction::CCW) {
        result = buildMoveHelper(relation, 4, 4, 0, 0, 1, 6, 5, 5);
      } else {
        result = buildFlipMoveHelper(relation, 4, 5, 5, 6, 1, 0, 0, 4);
      }
      break;
    case Face::D:
      if (d == Direction::CW) {
        result =  buildMoveHelper(relation, 3, 2, 2, 8, 6, 9, 7, 10);
      } else if (d == Direction::CCW) {
        result =  buildMoveHelper(relation, 3, 10, 7, 9, 6, 8, 2, 2);
      } else {
        result =  buildFlipMoveHelper(relation, 3, 2, 2, 8, 6, 9, 7, 10);
      }
      break;
    case Face::L:
      if (d == Direction::CW) {
        result =  buildMoveHelper(relation, 4, 4, 0, 3, 3, 10, 7, 11);
      } else if (d == Direction::CCW) {
        result =  buildMoveHelper(relation, 4, 11, 7, 10, 3, 3, 0, 4);
      } else {
        result =  buildFlipMoveHelper(relation, 4, 4, 0, 3, 3, 10, 7, 11);
      }
      break;
    case Face::R:
      if (d == Direction::CW) {
        result =  buildMoveHelper(relation, 1, 6, 5, 7, 6, 8, 2, 1);
      } else if (d == Direction::CCW) {
        result =  buildMoveHelper(relation, 1, 1, 2, 8, 6, 7, 5, 6);
      } else {
        result =  buildFlipMoveHelper(relation, 1, 6, 5, 7, 6, 8, 2, 1);
      }
      break;
    case Face::F:
      if (d == Direction::CW) {
        result = buildMoveHelper(relation, 0, 0, 1, 1, 2, 2, 3, 3);
      } else if (d == Direction::CCW) {
        result = buildMoveHelper(relation, 0, 3, 3, 2, 2, 1, 1, 0);
      } else {
        result = buildFlipMoveHelper(relation, 0, 0, 1, 1, 2, 2, 3, 3);
      }
      break;
    case Face::B:
      if (d == Direction::CW) {
        result =  buildMoveHelper(relation, 5, 5, 4, 11, 7, 9, 6, 7);
      } else if (d == Direction::CCW) {
        result =  buildMoveHelper(relation, 5, 7, 6, 9, 7, 11, 4, 5);
      } else {
        result =  buildFlipMoveHelper(relation, 5, 5, 4, 11, 7, 9, 6, 7);
      }
      break;
  }
  return result;
}

dd_edge RubiksCubeModel::buildMoveHelper(forest* relation, int type3_a,
    int type2_a, int type3_b, int type2_b, int type3_c, int type2_c,
    int type3_d, int type2_d) {
  assert(relation->isForRelations());

  // transform to variables
  int a2 = get_var_of_component(2, type2_a);
  int b2 = get_var_of_component(2, type2_b);
  int c2 = get_var_of_component(2, type2_c);
  int d2 = get_var_of_component(2, type2_d);
  int a3 = get_var_of_component(3, type3_a);
  int b3 = get_var_of_component(3, type3_b);
  int c3 = get_var_of_component(3, type3_c);
  int d3 = get_var_of_component(3, type3_d);

  // face is ordered like this:
  // type3, type2, type3, type2, type3, type2, type3, type2

  // Adding all the elements in one go
  //
  // 4 type 2 additions = 4 * 12 = 48
  // 4 type 3 additions = 4 * 8 = 32
  // total additions = 4 * 12 + 4 * 8 = 4 * 20 = 80
  //
  int nElements = 4 * _config.num_type2 + 4 * _config.num_type3;

  int** from = new int*[nElements];
  int** to = new int*[nElements];
  for (int i = 0; i < nElements; i++) {
    // allocate elements
    from[i] = new int[_config.num_components() + 1];
    to[i] = new int[_config.num_components() + 1];

    // initialize elements
    from[i][0] = 0;
    to[i][0] = 0;

    std::fill_n(&from[i][1], _config.num_components(), DONT_CARE);
    std::fill_n(&to[i][1], _config.num_components(), DONT_CHANGE);

    if (_config.num_type3 > 0) {
      to[i][a3] = DONT_CARE;
      to[i][b3] = DONT_CARE;
      to[i][c3] = DONT_CARE;
      to[i][d3] = DONT_CARE;
    }
    if (_config.num_type2 > 0) {
      to[i][a2] = DONT_CARE;
      to[i][b2] = DONT_CARE;
      to[i][c2] = DONT_CARE;
      to[i][d2] = DONT_CARE;
    }
  }

  int currElement = 0;

  if (_config.num_type3 > 0) {
    // a3' <- d3
    for (int i = 0; i < NUM_TYPE3; i++) {
      from[currElement][d3] = i;
      to[currElement][a3] = i;
      currElement++;
    }

    // b3' <- a3
    for (int i = 0; i < NUM_TYPE3; i++) {
      from[currElement][a3] = i;
      to[currElement][b3] = i;
      currElement++;
    }

    // c3' <- b3
    for (int i = 0; i < NUM_TYPE3; i++) {
      from[currElement][b3] = i;
      to[currElement][c3] = i;
      currElement++;
    }

    // d3' <- c3
    for (int i = 0; i < NUM_TYPE3; i++) {
      from[currElement][c3] = i;
      to[currElement][d3] = i;
      currElement++;
    }
  }

  if (_config.num_type2 > 0) {
    // a2' <- d2
    for (int i = 0; i < NUM_TYPE2; i++) {
      from[currElement][d2] = i;
      to[currElement][a2] = i;
      currElement++;
    }

    // b2' <- a2
    for (int i = 0; i < NUM_TYPE2; i++) {
      from[currElement][a2] = i;
      to[currElement][b2] = i;
      currElement++;
    }

    // c2' <- b2
    for (int i = 0; i < NUM_TYPE2; i++) {
      from[currElement][b2] = i;
      to[currElement][c2] = i;
      currElement++;
    }

    // d2' <- c2
    for (int i = 0; i < NUM_TYPE2; i++) {
      from[currElement][c2] = i;
      to[currElement][d2] = i;
      currElement++;
    }
  }

  // compute result = union elements to create an mxd for each component
  // and then intersect the mxds.

  dd_edge result(relation);
  relation->createEdge(true, result);
  dd_edge temp(relation);
  int offset = 0;

  if (_config.num_type3 > 0) {
    // a3' <- d3
    // b3' <- a3
    // c3' <- b3
    // d3' <- c3
    for (int i = 0; i < 4; i++) {
      relation->createEdge(from + offset, to + offset, NUM_TYPE3, temp);
      result *= temp;
      offset += NUM_TYPE3;
    }
  }

  if (_config.num_type2 > 0) {
    // a2' <- d2
    // b2' <- a2
    // c2' <- b2
    // d2' <- c2
    for (int i = 0; i < 4; i++) {
      relation->createEdge(from + offset, to + offset, NUM_TYPE2, temp);
      result *= temp;
      offset += NUM_TYPE2;
    }
  }

  assert(offset == nElements);

  for (int i = 0; i < nElements; i++) {
    delete[] from[i];
    delete[] to[i];
  }
  delete[] from;
  delete[] to;

  return result;
}

dd_edge RubiksCubeModel::buildFlipMoveHelper(forest* relation, int type3_a,
    int type2_a, int type3_b, int type2_b, int type3_c, int type2_c,
    int type3_d, int type2_d) {
  assert(relation->isForRelations());

  // transform to levels
  int a2 = get_var_of_component(2, type2_a);
  int b2 = get_var_of_component(2, type2_b);
  int c2 = get_var_of_component(2, type2_c);
  int d2 = get_var_of_component(2, type2_d);
  int a3 = get_var_of_component(3, type3_a);
  int b3 = get_var_of_component(3, type3_b);
  int c3 = get_var_of_component(3, type3_c);
  int d3 = get_var_of_component(3, type3_d);

  // face is ordered like this:
  // type3, type2, type3, type2, type3, type2, type3, type2

  // Adding all the elements in one go
  //
  // 4 type 2 additions = 4 * 12 = 48
  // 4 type 3 additions = 4 * 8 = 32
  // total additions = 4 * 12 + 4 * 8 = 4 * 20 = 80
  //
  int nElements = 4 * _config.num_type2 + 4 * _config.num_type3;
  int** from = new int*[nElements];
  int** to = new int*[nElements];
  for (int i = 0; i < nElements; i++) {
    // allocate elements
    from[i] =  new int[_config.num_components() + 1];
    to[i] = new int[_config.num_components() + 1];

    // initialize elements
    from[i][0] = 0;
    to[i][0] = 0;

    std::fill_n(&from[i][1], _config.num_components(), DONT_CARE);
    std::fill_n(&to[i][1], _config.num_components(), DONT_CHANGE);

    if (_config.num_type3 > 0) {
      to[i][a3] = DONT_CARE;
      to[i][b3] = DONT_CARE;
      to[i][c3] = DONT_CARE;
      to[i][d3] = DONT_CARE;
    }
    if (_config.num_type2 > 0) {
      to[i][a2] = DONT_CARE;
      to[i][b2] = DONT_CARE;
      to[i][c2] = DONT_CARE;
      to[i][d2] = DONT_CARE;
    }
  }

  int currElement = 0;

  if (_config.num_type3 > 0) {
    // a3' <- c3
    for (int i = 0; i < NUM_TYPE3; i++) {
      from[currElement][c3] = i;
      to[currElement][a3] = i;
      currElement++;
    }

    // b3' <- d3
    for (int i = 0; i < NUM_TYPE3; i++) {
      from[currElement][d3] = i;
      to[currElement][b3] = i;
      currElement++;
    }

    // c3' <- a3
    for (int i = 0; i < NUM_TYPE3; i++) {
      from[currElement][a3] = i;
      to[currElement][c3] = i;
      currElement++;
    }

    // d3' <- b3
    for (int i = 0; i < NUM_TYPE3; i++) {
      from[currElement][b3] = i;
      to[currElement][d3] = i;
      currElement++;
    }
  }

  if (_config.num_type2 > 0) {
    // a2' <- c2
    for (int i = 0; i < NUM_TYPE2; i++) {
      from[currElement][c2] = i;
      to[currElement][a2] = i;
      currElement++;
    }

    // b2' <- d2
    for (int i = 0; i < NUM_TYPE2; i++) {
      from[currElement][d2] = i;
      to[currElement][b2] = i;
      currElement++;
    }

    // c2' <- a2
    for (int i = 0; i < NUM_TYPE2; i++) {
      from[currElement][a2] = i;
      to[currElement][c2] = i;
      currElement++;
    }

    // d2' <- b2
    for (int i = 0; i < NUM_TYPE2; i++) {
      from[currElement][b2] = i;
      to[currElement][d2] = i;
      currElement++;
    }
  }

  // compute result = union elements to create an mxd for each component
  // and then intersect the mxds.

  dd_edge result(relation);
  relation->createEdge(true, result);
  int offset = 0;

  if (_config.num_type3 > 0) {
    // a3' <- c3
    // b3' <- d3
    // c3' <- a3
    // d3' <- b3
    for (int i = 0; i < 4; i++) {
      dd_edge temp(relation);
      relation->createEdge(from + offset, to + offset, NUM_TYPE3, temp);
      result *= temp;
      offset += NUM_TYPE3;
    }
  }

  if (_config.num_type2 > 0) {
    // a2' <- c2
    // b2' <- d2
    // c2' <- a2
    // d2' <- b2
    for (int i = 0; i < 4; i++) {
      dd_edge temp(relation);
      relation->createEdge(from + offset, to + offset, NUM_TYPE2, temp);
      result *= temp;
      offset += NUM_TYPE2;
    }
  }

  assert(offset == nElements);

  for (int i = 0; i < nElements; i++) {
    free(from[i]);
    free(to[i]);
  }
  free(from);
  free(to);

  return result;
}

void RubiksCubeModel::show_node(const dd_edge& e)
{
  static_cast<expert_forest*>(e.getForest())->removeAllComputeTableEntries();

  cout << "# Nodes: " << e.getForest()->getCurrentNumNodes() << endl;
  FILE_output out(stdout);
  e.show(out, 2);
  cout << "# States: " << e.getCardinality() << endl;
}

const char* face_to_string(Face f){
  switch(f) {
    case Face::F: return "Front";
    case Face::B: return "Back";
    case Face::L: return "Left";
    case Face::R: return "Right";
    case Face::U: return "Up";
    case Face::D: return "Down";
    default: return "Invalid Face";
  }
}

void usage() {
  cout << "Usage: rubik_cube [-p1|-p3|-p6] [-corner]" << endl;
  cout << "-p1  : use 1-phase phased saturation (normal saturation)" << endl;
  cout << "-p3  : use 3-phases phased saturation" << endl;
  cout << "-p6  : use 6-phases phased saturation" << endl;
  cout << "-corner  : use model with corners only" << endl;
  cout << endl;
}

int main(int argc, char *argv[])
{
  int num_phases = 3;
  RubiksCubeModelType type = RubiksCubeModelType::DEFAULT;

  if (argc > 1) {
    for (int i = 1; i < argc; i++) {
      char *cmd = argv[i];
      if (strncmp(cmd, "-p1", strlen("-p1")) == 0) {
        num_phases = 1;
      }
      else if (strncmp(cmd, "-p2", strlen("-p2")) == 0) {
        num_phases = 2;
      }
      else if (strncmp(cmd, "-p3", strlen("-p3")) == 0) {
        num_phases = 3;
      }
      else if (strncmp(cmd, "-p6", strlen("-p6")) == 0) {
        num_phases = 6;
      }
      else if (strncmp(cmd, "-corner", strlen("-corner")) == 0) {
        type = RubiksCubeModelType::CORNER_ONLY;
      }
      else {
        usage();
        exit(1);
      }
    }
  }

  // Initialize MEDDLY
  MEDDLY::initializer_list* L = defaultInitializerList(0);
  ct_initializer::setBuiltinStyle(ct_initializer::MonolithicUnchainedHash);
  ct_initializer::setMaxSize(16 * 16777216);
  MEDDLY::initialize(L);

  RubiksCubeModelConfig config = buildModelConfig(num_phases, type);
  RubiksCubeModel model(config);
  model.execute();

  cleanup();
  return 0;
}
