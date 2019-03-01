/* Implemented by Vidal Alcazar Saiz.*/

#ifndef H2_MUTEXES_H
#define H2_MUTEXES_H

#include <ctime>
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>

#include "variable.h"
#include "state.h"
#include "operator.h"
#include "axiom.h"
#include "mutex_group.h"


using namespace std;

enum Reachability {SPURIOUS, REACHED, NOT_REACHED};

static const int UNSOLVABLE = -2;
static const int  TIMEOUT = -1;

class Op_h2 {
public:
    Op_h2(const Operator &op,
          const vector< vector<unsigned>> &p_index,
          const std::vector<std::vector<std::set<std::pair<int, int>>>> &inconsistent_facts,
          bool regression);

    vector<unsigned> pre;
    vector<unsigned> add;
    vector<unsigned> del;
    Reachability triggered;

private:
    inline void push_pre(const vector< vector<unsigned>> &p_index, Variable *var, int val) {
        if (var->get_level() >= 0) {
            pre.push_back(p_index[var->get_level()][val]);
        }
    }

    inline void push_add(const vector< vector<unsigned>> &p_index, Variable *var, int val) {
        if (var->get_level() >= 0) {
            add.push_back(p_index[var->get_level()][val]);
        }
    }

    void instantiate_operator_backward(const Operator &op, const vector< vector<unsigned>> &p_index,
                                       const std::vector<std::vector<std::set<std::pair<int, int>>>> &inconsistent_facts);
    void instantiate_operator_forward(const Operator &op, const vector< vector<unsigned>> &p_index,
                                      const std::vector<std::vector<std::set<std::pair<int, int>>>> &inconsistent_facts);
};


class H2Mutexes {
    bool check_initial_state_is_dead_end(const vector <Variable *> &variables,
						    const State &initial_state) const;

    bool check_goal_state_is_unreachable(const vector<pair<Variable *, int>> &goal) const;
public:
    H2Mutexes(int t = -1) : limit_seconds(t) {
        if (limit_seconds != -1)
            time(&start);
    }
    virtual ~H2Mutexes() {}

    int compute(const vector <Variable *> &variables,
                vector<Operator> &operators, //not const because may be detected to be spurious
                const vector<Axiom> &axioms,
                const State &initial_state,
                const vector<pair<Variable *, int>> &goal,
                vector<MutexGroup> &mutexes,
                bool regression);

    void print_mutexes(const std::vector <Variable *> &variables);

    inline bool are_mutex(int var1, int val1, int var2, int val2) const {
        if (val1 == -1 || val2 == -1)
            return false;

        if (var1 == var2) // same variable: mutex iff different value
            return val1 != val2;  //TODO: || unreachable[var1][val1];
        unsigned p1 = p_index[var1][val1];
        unsigned p2 = p_index[var2][val2];
        return m_values[position(p1, p2)] == SPURIOUS;
    }

    inline int num_variables() const {
        return num_vars;
    }

    inline int num_values(int var) const {
        return num_vals[var];
    }

    inline bool is_unreachable(int var, int val) const {
        return unreachable[var][val];
    }


    int detect_unreachable_fluents(const vector <Variable *> &variables, 
				    const State &initial_state, 
				   const vector<pair<Variable *, int>> &goal);

    bool remove_spurious_operators(vector<Operator> &operators);
    void set_unreachable_propositions(const vector <Variable *> &variables);

    bool initialize(const vector <Variable *> &variables,
                    const vector<MutexGroup> &mutexes);

protected:
    int num_vars;
    std::vector<int> num_vals;

    std::set<std::pair<int, int>> static_fluents;
    std::vector <std::vector <bool >> unreachable;
    std::vector<std::vector<std::set<std::pair<int, int>>>> inconsistent_facts;

    unsigned number_props;
    vector<unsigned> m_values;
    vector<Op_h2> m_ops;

    vector< vector<unsigned>> p_index;
    vector< pair<unsigned, unsigned>> p_index_reverse;

    Reachability eval_propositions(const vector<unsigned> & props);

    inline unsigned position(unsigned a, unsigned b) const {
        return (a * number_props) + b;
    }

    bool set_unreachable(int var, int val, const vector <Variable *> &variables, 
			 const State &initial_state, 
			 const vector<pair<Variable *, int>> &goal); 

    void print_pair(unsigned pair);

    int limit_seconds;
    time_t start;
    bool time_exceeded();

    bool init_values_progression(const vector <Variable *> &variables,
                                 const State &initial_state);
    bool init_values_regression(const vector<pair<Variable *, int>> &goal);
    void init_h2_operators(const vector<Operator> &operators,
                           const vector<Axiom> &axioms, bool regression);

    void setPropositionNotReached(int prop_index);
};

//Computes h2 mutexes, and removes every unnecessary variables, operators, axioms, initial state and goal.
extern bool compute_h2_mutexes(const vector <Variable *> &variables,
                               vector<Operator> &operators,
                               vector<Axiom> &axioms,
                               vector<MutexGroup> &mutexes,
                               State &initial_state,
                               const vector<pair<Variable *, int>> &goal,
                               int limit_seconds, bool disable_bw_h2);



#endif
