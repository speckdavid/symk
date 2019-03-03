/*
 * This class is mainly taken from
 * https://bitbucket.org/wintered/kstar/src/default/
 * Michael Katz, Shirin  * Sohrabi, Octavian Udrea and Dominik Winterer
 * A Novel Iterative Approach to Top-k Planning
 * In ICAPS 2018
 */

#ifndef TASKS_PLAN_FORBID_REFORMULATED_TASK_H
#define TASKS_PLAN_FORBID_REFORMULATED_TASK_H

#include "../task_utils/plan_graph.h"
#include "delegating_task.h"

#include <vector>

namespace extra_tasks {

class PlanForbidReformulatedTask : public tasks::DelegatingTask {
  plan_graph::PlanGraph plan_graph;
  //	std::vector<int> reformulated_operator_indexes;
  int operators_in_plan_plan;
  std::vector<int> initial_state_values;
  // Every entry maps to the parent op
  std::vector<int> index_op_type0_modified;
  std::vector<int> index_op_type1_modified;
  std::vector<int> index_op_type2_modified;
  std::vector<int> index_op_type3_modified;
  // Stores for every op id which type it is
  std::vector<int> index_to_op_type;
  int num_op_type3;

  int get_parent_index(int index) const;
  int get_possible_var_index() const;
  int get_following_var_index(int state) const;

public:
  PlanForbidReformulatedTask(const std::shared_ptr<AbstractTask> parent,
                             const std::vector<Plan> &plans);
  virtual ~PlanForbidReformulatedTask() override = default;

  virtual int get_num_variables() const override;
  virtual std::string get_variable_name(int var) const override;
  virtual int get_variable_domain_size(int var) const override;
  virtual int get_variable_axiom_layer(int var) const override;
  virtual int get_variable_default_axiom_value(int var) const override;
  virtual std::string get_fact_name(const FactPair &fact) const override;
  virtual bool are_facts_mutex(const FactPair &fact1,
                               const FactPair &fact2) const override;

  virtual int get_operator_cost(int index, bool is_axiom) const override;
  virtual std::string get_operator_name(int index,
                                        bool is_axiom) const override;
  virtual int get_num_operators() const override;
  virtual int get_num_operator_preconditions(int index,
                                             bool is_axiom) const override;
  virtual FactPair get_operator_precondition(int op_index, int fact_index,
                                             bool is_axiom) const override;
  virtual int get_num_operator_effects(int op_index,
                                       bool is_axiom) const override;
  virtual int get_num_operator_effect_conditions(int op_index, int eff_index,
                                                 bool is_axiom) const override;
  virtual FactPair get_operator_effect_condition(int op_index, int eff_index,
                                                 int cond_index,
                                                 bool is_axiom) const override;
  virtual FactPair get_operator_effect(int op_index, int eff_index,
                                       bool is_axiom) const override;

  virtual int get_num_goals() const override;
  virtual FactPair get_goal_fact(int index) const override;

  virtual std::vector<int> get_initial_state_values() const override;
  virtual void
  convert_state_values_from_parent(std::vector<int> &values) const override;
};
} // namespace extra_tasks

#endif
