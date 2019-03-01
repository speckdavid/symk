/*
 * This class is mainly taken from
 * https://bitbucket.org/wintered/kstar/src/default/
 * Michael Katz, Shirin  * Sohrabi, Octavian Udrea and Dominik Winterer
 * A Novel Iterative Approach to Top-k Planning
 * In ICAPS 2018
 */

#include "plan_forbid_reformulated_task.h"

#include <cassert>
#include <sstream>

#include "../utils/system.h"

using namespace std;

namespace extra_tasks {
PlanForbidReformulatedTask::PlanForbidReformulatedTask(
    const shared_ptr<AbstractTask> parent, std::vector<int> &&plan)
    : DelegatingTask(parent),
      forbidding_plan(move(plan)),
      operators_on_plan(0) {
  // Compute the indexes for new operators
  // First go all operators that are not on the plan
  // Then go all ops from plan with index 1, then 2, then 3.
  on_plan.assign(parent->get_num_operators(), false);
  for (int op_no : forbidding_plan) {
    if (!on_plan[op_no]) {
      operators_on_plan++;
      on_plan[op_no] = true;
      plan_operators_indexes.push_back(op_no);
    }
  }
  // Storing plan indexes per plan operator
  plan_operators_indexes_by_parent_operator.assign(parent->get_num_operators(),
                                                   vector<int>());
  for (size_t i = 0; i < forbidding_plan.size(); ++i) {
    int op_no = forbidding_plan[i];
    plan_operators_indexes_by_parent_operator[op_no].push_back(i);
  }

  // Creating initial state values by copying from the parent and pushing the
  // new variables initial values
  initial_state_values = parent->get_initial_state_values();
  initial_state_values.push_back(1);
  initial_state_values.push_back(1);
  initial_state_values.insert(initial_state_values.end(),
                              forbidding_plan.size(), 0);
}

bool PlanForbidReformulatedTask::is_operator_on_plan(int op_no) const {
  return on_plan[op_no];
}

int PlanForbidReformulatedTask::get_num_operator_appearances_on_plan(
    int op_no) const {
  return plan_operators_indexes_by_parent_operator[op_no].size();
}

int PlanForbidReformulatedTask::get_plan_index_ordered(
    int op_no, int appearance_index) const {
  return plan_operators_indexes_by_parent_operator[op_no][appearance_index];
}

int PlanForbidReformulatedTask::get_parent_op_index(int index) const {
  // Getting the index of the corresponding operator in the parent task
  // First go all the operators from the parent task (type 0 for non-plan
  // operators, type 1 for plan opreators) Then all the plan operators (no
  // repetitions), with type 2 Then all the plan operators with repetitions,
  // with type 3
  int num_parent_ops = parent->get_num_operators();
  if (index < num_parent_ops) return index;
  int relative_index = index - num_parent_ops;
  if (relative_index < operators_on_plan)
    return plan_operators_indexes[relative_index];

  relative_index -= operators_on_plan;
  return forbidding_plan[relative_index];
}

int PlanForbidReformulatedTask::get_op_type(int index) const {
  // Returns type 0 for operators not on plan
  //         type 1 for operators "already discarded"
  //         type 2 for operators "discarding pi"
  //         type 3 for operators "following pi"
  int num_parent_ops = parent->get_num_operators();
  if (index < num_parent_ops) {
    return is_operator_on_plan(index);
  }

  int relative_index = index - num_parent_ops;
  if (relative_index < operators_on_plan) return 2;

  return 3;
}

int PlanForbidReformulatedTask::get_possible_var_index() const {
  return parent->get_num_variables();
}

int PlanForbidReformulatedTask::get_following_var_index(int op_index) const {
  // This is valid for type 2 and 3 only.
  // TODO: Fix this
  int op_type = get_op_type(op_index);
  assert(op_type == 2 || op_type == 3);
  int num_parent_ops = parent->get_num_operators();
  int plan_op_index = op_index - num_parent_ops;  // For type 2
  if (op_type == 3) {
    plan_op_index -= operators_on_plan;
  }
  return parent->get_num_variables() + 1 + plan_op_index;
}

int PlanForbidReformulatedTask::get_num_variables() const {
  return parent->get_num_variables() + forbidding_plan.size() + 2;
}

string PlanForbidReformulatedTask::get_variable_name(int var) const {
  if (var < parent->get_num_variables()) return parent->get_variable_name(var);
  if (var == parent->get_num_variables()) return "possible";
  int ind = var - parent->get_num_variables() - 1;
  return ("following" +
          static_cast<ostringstream *>(&(ostringstream() << ind))->str());
}

int PlanForbidReformulatedTask::get_variable_domain_size(int var) const {
  if (var < parent->get_num_variables())
    return parent->get_variable_domain_size(var);

  return 2;
}

int PlanForbidReformulatedTask::get_variable_axiom_layer(int var) const {
  if (var < parent->get_num_variables())
    return parent->get_variable_axiom_layer(var);

  return -1;
}

int PlanForbidReformulatedTask::get_variable_default_axiom_value(
    int var) const {
  if (var < parent->get_num_variables())
    return parent->get_variable_default_axiom_value(var);
  if (var <= parent->get_num_variables() + 1) return 1;
  return 0;
}

string PlanForbidReformulatedTask::get_fact_name(const FactPair &fact) const {
  if (fact.var < parent->get_num_variables())
    return parent->get_fact_name(fact);
  assert(fact.value >= 0 && fact.value <= 1);
  if (fact.value == 0) return "false";
  return "true";
}

bool PlanForbidReformulatedTask::are_facts_mutex(const FactPair &fact1,
                                                 const FactPair &fact2) const {
  if (fact1.var < parent->get_num_variables() &&
      fact2.var < parent->get_num_variables())
    return parent->are_facts_mutex(fact1, fact2);

  if (fact1.var >= parent->get_num_variables() && fact1.var == fact2.var &&
      fact1.value != fact2.value)
    return true;

  return false;
  //    ABORT("PlanForbidReformulatedTask doesn't support querying mutexes.");
}

int PlanForbidReformulatedTask::get_operator_cost(int index,
                                                  bool is_axiom) const {
  if (is_axiom) return parent->get_operator_cost(index, is_axiom);

  int parent_op_index = get_parent_op_index(index);
  return parent->get_operator_cost(parent_op_index, is_axiom);
}

string PlanForbidReformulatedTask::get_operator_name(int index,
                                                     bool is_axiom) const {
  if (is_axiom) return parent->get_operator_name(index, is_axiom);

  int parent_op_index = get_parent_op_index(index);
  string name = parent->get_operator_name(parent_op_index, is_axiom);
  int op_type = get_op_type(index);

  return (name + " " +
          static_cast<ostringstream *>(&(ostringstream() << op_type))->str());
}

int PlanForbidReformulatedTask::get_num_operators() const {
  // The number of operators according to the fixed reformulation is
  // number of operators + number of operators on the plan + plan length (number
  // of operators on the plan with repetitions) First, we have all parent
  // operators (non-plan operators of type 0 and plan operators of type 1), then
  // operators of type 2 (no repetition), and finally operators of type 3 (with
  // repetitions).

  return parent->get_num_operators() + operators_on_plan +
         forbidding_plan.size();
}

int PlanForbidReformulatedTask::get_num_operator_preconditions(
    int index, bool is_axiom) const {
  if (is_axiom) return parent->get_num_operator_preconditions(index, is_axiom);

  int op_type = get_op_type(index);
  assert(op_type >= 0 && op_type <= 3);

  int parent_op_index = get_parent_op_index(index);
  if (op_type == 0)
    return parent->get_num_operator_preconditions(parent_op_index, is_axiom);

  if (op_type == 1)
    return parent->get_num_operator_preconditions(parent_op_index, is_axiom) +
           1;

  if (op_type == 2)
    return parent->get_num_operator_preconditions(parent_op_index, is_axiom) +
           1 + get_num_operator_appearances_on_plan(parent_op_index);

  assert(op_type == 3);
  return parent->get_num_operator_preconditions(parent_op_index, is_axiom) + 2;
}

FactPair PlanForbidReformulatedTask::get_operator_precondition(
    int op_index, int fact_index, bool is_axiom) const {
  if (is_axiom)
    return parent->get_operator_precondition(op_index, fact_index, is_axiom);

  int parent_op_index = get_parent_op_index(op_index);
  int parent_num_pre =
      parent->get_num_operator_preconditions(parent_op_index, is_axiom);
  if (fact_index < parent_num_pre)
    return parent->get_operator_precondition(parent_op_index, fact_index,
                                             is_axiom);

  int op_type = get_op_type(op_index);
  assert(op_type >= 1 && op_type <= 3);

  if (op_type == 1) return FactPair(get_possible_var_index(), 0);

  // op_type == 2 || op_type == 3
  // first additional precondition is the same for both cases
  if (fact_index == parent_num_pre)
    return FactPair(get_possible_var_index(), 1);

  // next additional preconditions
  if (op_type == 2) {
    int relative_fact_index = fact_index - parent_num_pre - 1;
    int op_on_plan_index =
        parent->get_num_variables() + 1 +
        get_plan_index_ordered(parent_op_index, relative_fact_index);
    return FactPair(op_on_plan_index, 0);
  }
  assert(op_type == 3);
  return FactPair(get_following_var_index(op_index), 1);
}

int PlanForbidReformulatedTask::get_num_operator_effects(int op_index,
                                                         bool is_axiom) const {
  if (is_axiom) return parent->get_num_operator_effects(op_index, is_axiom);

  int op_type = get_op_type(op_index);
  assert(op_type >= 0 && op_type <= 3);
  int parent_op_index = get_parent_op_index(op_index);
  if (op_type == 0 || op_type == 2)
    return parent->get_num_operator_effects(parent_op_index, is_axiom) + 1;

  if (op_type == 1)
    return parent->get_num_operator_effects(parent_op_index, is_axiom);

  assert(op_type == 3);
  return parent->get_num_operator_effects(parent_op_index, is_axiom) + 2;
}

int PlanForbidReformulatedTask::get_num_operator_effect_conditions(
    int op_index, int eff_index, bool is_axiom) const {
  if (is_axiom)
    return parent->get_num_operator_effect_conditions(op_index, eff_index,
                                                      is_axiom);

  int parent_op_index = get_parent_op_index(op_index);
  int parent_num_effs =
      parent->get_num_operator_effects(parent_op_index, is_axiom);
  if (eff_index < parent_num_effs)
    return parent->get_num_operator_effect_conditions(parent_op_index,
                                                      eff_index, is_axiom);

  // The additional effects are unconditional
  return 0;
}

FactPair PlanForbidReformulatedTask::get_operator_effect_condition(
    int op_index, int eff_index, int cond_index, bool is_axiom) const {
  int parent_op_index = op_index;
  if (!is_axiom) parent_op_index = get_parent_op_index(op_index);
  return parent->get_operator_effect_condition(parent_op_index, eff_index,
                                               cond_index, is_axiom);
}

FactPair PlanForbidReformulatedTask::get_operator_effect(int op_index,
                                                         int eff_index,
                                                         bool is_axiom) const {
  if (is_axiom)
    return parent->get_operator_effect(op_index, eff_index, is_axiom);

  int parent_op_index = get_parent_op_index(op_index);
  int parent_num_effs =
      parent->get_num_operator_effects(parent_op_index, is_axiom);
  if (eff_index < parent_num_effs)
    return parent->get_operator_effect(parent_op_index, eff_index, is_axiom);

  int op_type = get_op_type(op_index);
  assert(op_type >= 0 && op_type <= 3);

  if (op_type == 0 || op_type == 2)
    return FactPair(get_possible_var_index(), 0);

  assert(op_type == 3);

  if (eff_index == parent_num_effs)
    return FactPair(get_following_var_index(op_index), 0);
  return FactPair(get_following_var_index(op_index) + 1, 1);
}

int PlanForbidReformulatedTask::get_num_goals() const {
  return parent->get_num_goals() + 1;
}

FactPair PlanForbidReformulatedTask::get_goal_fact(int index) const {
  if (index < parent->get_num_goals()) return parent->get_goal_fact(index);

  return FactPair(get_possible_var_index(), 0);
}

vector<int> PlanForbidReformulatedTask::get_initial_state_values() const {
  return initial_state_values;
}

void PlanForbidReformulatedTask::convert_state_values_from_parent(
    vector<int> &) const {
  ABORT(
      "PlanForbidReformulatedTask doesn't support getting a state from the "
      "parent state.");
}

}  // namespace extra_tasks
