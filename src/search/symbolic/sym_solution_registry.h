#ifndef SYMBOLIC_SYM_SOLUTION_REGISTRY_H
#define SYMBOLIC_SYM_SOLUTION_REGISTRY_H

#include "../plan_manager.h"
#include "../state_registry.h"
#include "../task_proxy.h"
#include "sym_solution.h"
#include "sym_variables.h"

namespace symbolic
{
class PlanReconstructor;

class SymCut
{
protected:
  int g;
  int h;
  BDD cut;

public:
  SymCut(int g, int h, BDD cut);

  int get_g() const;
  int get_h() const;
  int get_f() const;
  BDD get_cut() const;
  void merge(const SymCut &other);

  void set_g(int g);
  void set_h(int h);
  void set_cut(BDD cut);

  // Here we only compare g and h values!!!
  bool operator<(const SymCut &other) const;
  bool operator>(const SymCut &other) const;
  bool operator==(const SymCut &other) const;
  bool operator!=(const SymCut &other) const;

  friend std::ostream &operator<<(std::ostream &os, const SymCut &sym_cut)
  {
    return os << "symcut{g=" << sym_cut.get_g() << ", h=" << sym_cut.get_h()
              << ", f=" << sym_cut.get_f()
              << ", nodes=" << sym_cut.get_cut().nodeCount() << "}";
  }
};

class SymSolutionRegistry
{
protected:
  std::shared_ptr<PlanReconstructor> plan_reconstructor;
  int num_target_plans;
  int num_found_plans;
  std::vector<SymCut> sym_cuts; // always sorted in ascending order!!!

  TaskProxy relevant_task;
  std::shared_ptr<StateRegistry> state_registry;
  std::shared_ptr<SymVariables> sym_vars;
  BDD states_on_goal_paths;
  int plan_cost_bound;

  int missing_plans() const { return num_target_plans - num_found_plans; }

public:
  SymSolutionRegistry(int target_num_plans);

  void init(std::shared_ptr<SymVariables> sym_vars)
  {
    this->sym_vars = sym_vars;
    state_registry = std::make_shared<StateRegistry>(relevant_task),
    states_on_goal_paths = sym_vars->zeroBDD();
  }

  void register_solution(const SymSolution &solution);
  void construct_cheaper_solutions(int bound);

  bool found_all_plans() const { return missing_plans() <= 0; }

  int get_num_found_plans() const { return num_found_plans; }

  BDD get_states_on_goal_paths() const { return states_on_goal_paths; }
};
} // namespace symbolic

#endif
