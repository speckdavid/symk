#ifndef HELPER_FUNCTIONS_H
#define HELPER_FUNCTIONS_H

#include "state.h"
#include "variable.h"

#include <string>
#include <vector>
#include <iostream>

using namespace std;

class State;
class MutexGroup;
class Operator;
class Axiom;

//void read_everything
void read_preprocessed_problem_description(istream & in,
                                           bool &metric,
                                           vector<Variable> &internal_variables,
                                           vector<Variable *> &variables,
                                           vector<MutexGroup> &mutexes,
                                           State & initial_state,
                                           vector<pair<Variable *, int>> &goals,
                                           vector<Operator> &operators,
                                           vector<Axiom> &axioms);

//void dump_everything
void dump_preprocessed_problem_description(const vector<Variable *> &variables,
                                           const State &initial_state,
                                           const vector<pair<Variable *, int>> &goals,
                                           const vector<Operator> &operators,
                                           const vector<Axiom> &axioms);

void generate_unsolvable_cpp_input();
void generate_cpp_input(const vector<Variable *> &ordered_var,
                        const bool &metric,
                        const vector<MutexGroup> &mutexes,
                        const State &initial_state,
                        const vector<pair<Variable *, int>> &goals,
                        const vector<Operator> &operators,
                        const vector<Axiom> &axioms);
void check_magic(istream & in, string magic);

#endif
