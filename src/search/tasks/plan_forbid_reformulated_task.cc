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

namespace extra_tasks
{
PlanForbidReformulatedTask::PlanForbidReformulatedTask(
    const shared_ptr<AbstractTask> parent, const std::vector<Plan> &plans)
    : DelegatingTask(parent), plan_graph(plans, TaskProxy(*parent)),
      num_op_type3(0)
{
  TaskProxy cur_task(*parent);
  // Filling the index vectors
  for (const auto &op : cur_task.get_operators())
  {
    if (!plan_graph.is_contained(OperatorID(op.get_id())))
    {
      index_op_type0_modified.push_back(op.get_id());
      index_op_type1_modified.push_back(-1);
      index_op_type2_modified.push_back(-1);
      index_op_type3_modified.push_back(-1);
      index_to_op_type.push_back(0);
    }
    else
    {
      index_op_type0_modified.push_back(-1);
      index_op_type1_modified.push_back(op.get_id());
      index_op_type2_modified.push_back(-1);
      index_op_type3_modified.push_back(-1);
      index_to_op_type.push_back(1);

      index_op_type0_modified.push_back(-1);
      index_op_type1_modified.push_back(-1);
      index_op_type2_modified.push_back(op.get_id());
      index_op_type3_modified.push_back(-1);
      index_to_op_type.push_back(2);
    }
  }

  // Create type3 operators
  for (size_t i = 0; i < plan_graph.get_num_edges(); i++)
  {
    index_op_type0_modified.push_back(-1);
    index_op_type1_modified.push_back(-1);
    index_op_type2_modified.push_back(-1);
    index_op_type3_modified.push_back(plan_graph.get_op(i));
    index_to_op_type.push_back(3);
    num_op_type3++;
  }
}

int PlanForbidReformulatedTask::get_parent_index(int index) const
{
  int type = index_to_op_type.at(index);
  int parent_op_index = -1;
  switch (type)
  {
  case 0:
    parent_op_index = index_op_type0_modified.at(index);
    break;
  case 1:
    parent_op_index = index_op_type1_modified.at(index);
    break;
  case 2:
    parent_op_index = index_op_type2_modified.at(index);
    break;
  case 3:
    parent_op_index = index_op_type3_modified.at(index);
    break;
  }
  return parent_op_index;
}

int PlanForbidReformulatedTask::get_possible_var_index() const
{
  // First value
  return parent->get_num_variables();
}

int PlanForbidReformulatedTask::get_following_var_index(int state) const
{
  // Second value
  return parent->get_num_variables() + 1 + state;
}

int PlanForbidReformulatedTask::get_num_variables() const
{
  return parent->get_num_variables() + plan_graph.get_num_nodes() + 1;
}

string PlanForbidReformulatedTask::get_variable_name(int var) const
{
  if (var < parent->get_num_variables())
    return parent->get_variable_name(var);
  if (var == parent->get_num_variables())
    return "possible";
  int ind = var - parent->get_num_variables() - 1;
  return ("following" +
          static_cast<ostringstream *>(&(ostringstream() << ind))->str());
}

int PlanForbidReformulatedTask::get_variable_domain_size(int var) const
{
  if (var < parent->get_num_variables())
    return parent->get_variable_domain_size(var);

  return 2;
}

int PlanForbidReformulatedTask::get_variable_axiom_layer(int var) const
{
  if (var < parent->get_num_variables())
    return parent->get_variable_axiom_layer(var);

  return -1;
}

int PlanForbidReformulatedTask::get_variable_default_axiom_value(
    int var) const
{
  if (var < parent->get_num_variables())
    return parent->get_variable_default_axiom_value(var);
  if (var <= parent->get_num_variables() + 1)
    return 1;
  return 0;
}

string PlanForbidReformulatedTask::get_fact_name(const FactPair &fact) const
{
  if (fact.var < parent->get_num_variables())
    return parent->get_fact_name(fact);
  assert(fact.value >= 0 && fact.value <= 1);
  if (fact.value == 0)
    return "false";
  return "true";
}

bool PlanForbidReformulatedTask::are_facts_mutex(const FactPair &fact1,
                                                 const FactPair &fact2) const
{
  if (fact1.var < parent->get_num_variables() &&
      fact2.var < parent->get_num_variables())
    return parent->are_facts_mutex(fact1, fact2);

  if (fact1.var >= parent->get_num_variables() && fact1.var == fact2.var &&
      fact1.value != fact2.value)
    return true;

  return false;
}

int PlanForbidReformulatedTask::get_operator_cost(int index,
                                                  bool is_axiom) const
{
  if (is_axiom)
  {
    return parent->get_operator_cost(index, is_axiom);
  }

  int parent_op_index = get_parent_index(index);
  return parent->get_operator_cost(parent_op_index, is_axiom);
}

string PlanForbidReformulatedTask::get_operator_name(int index,
                                                     bool is_axiom) const
{
  if (is_axiom)
    return parent->get_operator_name(index, is_axiom);

  int parent_op_index = get_parent_index(index);
  return parent->get_operator_name(parent_op_index, is_axiom) + "_type_" +
         std::to_string(index_to_op_type.at(index));
}

int PlanForbidReformulatedTask::get_num_operators() const
{
  return index_to_op_type.size();
}

int PlanForbidReformulatedTask::get_num_operator_preconditions(
    int index, bool is_axiom) const
{
  if (is_axiom)
    return parent->get_num_operator_preconditions(index, is_axiom);

  int op_type = index_to_op_type.at(index);
  assert(op_type >= 0 && op_type <= 3);

  int parent_op_index = get_parent_index(index);
  if (op_type == 0)
    return parent->get_num_operator_preconditions(parent_op_index, is_axiom);

  if (op_type == 1)
    return parent->get_num_operator_preconditions(parent_op_index, is_axiom) +
           1;

  if (op_type == 2)
    return parent->get_num_operator_preconditions(parent_op_index, is_axiom) +
           1 + plan_graph.get_num_of_edges(OperatorID(parent_op_index));

  assert(op_type == 3);
  return parent->get_num_operator_preconditions(parent_op_index, is_axiom) + 2;
}

FactPair PlanForbidReformulatedTask::get_operator_precondition(
    int op_index, int fact_index, bool is_axiom) const
{
  if (is_axiom)
    return parent->get_operator_precondition(op_index, fact_index, is_axiom);

  int parent_op_index = get_parent_index(op_index);
  int parent_num_pre =
      parent->get_num_operator_preconditions(parent_op_index, is_axiom);
  if (fact_index < parent_num_pre)
    return parent->get_operator_precondition(parent_op_index, fact_index,
                                             is_axiom);

  int op_type = index_to_op_type.at(op_index);
  assert(op_type >= 1 && op_type <= 3);

  if (op_type == 1)
    return FactPair(get_possible_var_index(), 0);

  // op_type == 2 || op_type == 3
  // first additional precondition is the same for both cases
  if (fact_index == parent_num_pre)
    return FactPair(get_possible_var_index(), 1);

  // next additional preconditions
  if (op_type == 2)
  {
    int relative_fact_index = fact_index - parent_num_pre - 1;
    int state = plan_graph.get_source_state(OperatorID(parent_op_index),
                                            relative_fact_index);
    return FactPair(get_following_var_index(state), 0);
  }
  assert(op_type == 3);
  int edge_id = op_index - (get_num_operators() - num_op_type3);
  int state = plan_graph.get_source_state(edge_id);
  return FactPair(get_following_var_index(state), 1);
}

int PlanForbidReformulatedTask::get_num_operator_effects(int op_index,
                                                         bool is_axiom) const
{
  if (is_axiom)
    return parent->get_num_operator_effects(op_index, is_axiom);

  int op_type = index_to_op_type.at(op_index);
  assert(op_type >= 0 && op_type <= 3);
  int parent_op_index = get_parent_index(op_index);
  if (op_type == 0 || op_type == 2)
    return parent->get_num_operator_effects(parent_op_index, is_axiom) + 1;

  if (op_type == 1)
    return parent->get_num_operator_effects(parent_op_index, is_axiom);

  assert(op_type == 3);
  return parent->get_num_operator_effects(parent_op_index, is_axiom) + 2;
}

int PlanForbidReformulatedTask::get_num_operator_effect_conditions(
    int op_index, int eff_index, bool is_axiom) const
{
  if (is_axiom)
    return parent->get_num_operator_effect_conditions(op_index, eff_index,
                                                      is_axiom);

  int parent_op_index = get_parent_index(op_index);
  int parent_num_effs =
      parent->get_num_operator_effects(parent_op_index, is_axiom);
  if (eff_index < parent_num_effs)
    return parent->get_num_operator_effect_conditions(parent_op_index,
                                                      eff_index, is_axiom);

  // The additional effects are unconditional
  return 0;
}

FactPair PlanForbidReformulatedTask::get_operator_effect_condition(
    int op_index, int eff_index, int cond_index, bool is_axiom) const
{
  int parent_op_index = op_index;
  if (!is_axiom)
    parent_op_index = get_parent_index(op_index);
  return parent->get_operator_effect_condition(parent_op_index, eff_index,
                                               cond_index, is_axiom);
}

FactPair PlanForbidReformulatedTask::get_operator_effect(int op_index,
                                                         int eff_index,
                                                         bool is_axiom) const
{
  if (is_axiom)
    return parent->get_operator_effect(op_index, eff_index, is_axiom);

  int parent_op_index = get_parent_index(op_index);
  int parent_num_effs =
      parent->get_num_operator_effects(parent_op_index, is_axiom);
  if (eff_index < parent_num_effs)
    return parent->get_operator_effect(parent_op_index, eff_index, is_axiom);

  int op_type = index_to_op_type.at(op_index);
  assert(op_type >= 0 && op_type <= 3);

  if (op_type == 0 || op_type == 2)
    return FactPair(get_possible_var_index(), 0);

  assert(op_type == 3);
  int edge_id = op_index - (get_num_operators() - num_op_type3);
  if (eff_index == parent_num_effs)
  {
    int state = plan_graph.get_source_state(edge_id);
    return FactPair(get_following_var_index(state), 0);
  }
  int state = plan_graph.get_target_state(edge_id);

  return FactPair(get_following_var_index(state) + 1, 1);
}

int PlanForbidReformulatedTask::get_num_goals() const
{
  return parent->get_num_goals() + 1;
}

FactPair PlanForbidReformulatedTask::get_goal_fact(int index) const
{
  if (index < parent->get_num_goals())
    return parent->get_goal_fact(index);

  return FactPair(get_possible_var_index(), 0);
}

vector<int> PlanForbidReformulatedTask::get_initial_state_values() const
{
  return initial_state_values;
}

void PlanForbidReformulatedTask::convert_state_values_from_parent(
    vector<int> &) const
{
  ABORT("PlanForbidReformulatedTask doesn't support getting a state from the "
        "parent state.");
}

} // namespace extra_tasks
