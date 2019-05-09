#ifndef SYMBOLIC_SYM_PLAN_RECONSTRUCTION_H
#define SYMBOLIC_SYM_PLAN_RECONSTRUCTION_H

#include "../plan_manager.h"
#include "sym_solution_registry.h"
#include "unidirectional_search.h"

namespace symbolic
{

class ClosedList;

class PlanReconstructor
{
protected:
  UnidirectionalSearch *uni_search_fw;
  UnidirectionalSearch *uni_search_bw;
  std::shared_ptr<SymVariables> sym_vars;
  std::shared_ptr<StateRegistry> state_registry;
  std::map<int, std::vector<TransitionRelation>> trs;
  std::vector<Plan> found_plans;
  std::unordered_set<size_t> hashes_found_plans;
  size_t desired_num_plans;

  size_t get_hash_value(const Plan &plan) const;

  void add_plan(const Plan &plan);

  bool found_enough_plans() const
  {
    return found_plans.size() >= desired_num_plans;
  }

  bool task_has_zero_costs() const
  {
    return trs.count(0) > 0;
  }

  bool bw_reconstruction_necessary() const;

  Bdd get_resulting_state(const Plan &plan) const;

  // Returns zero Bdd if we dont need zero reconstruction. otherwise it returns
  // the correct cutted bdd!
  // First we need to check if it is contained in any 0 bucket or if the
  // pre/succcessor is contained in any bucket
  Bdd bdd_for_zero_reconstruction(const Bdd &cut, int cost,
                                  std::shared_ptr<ClosedList> closed) const;

  // Extracts all plans by a DFS, we copy the current plan suffix by every
  // recusive call which is why we don't use any reference for plan
  // BID: After reconstruction of the forward part we reverse the plan and
  // call extract_all_plans in bw direction which completes the plan
  // After completing a plan we store it in found plans!
  void extract_all_plans(SymCut &sym_cut, bool fw, Plan plan);

  void extract_all_cost_plans(SymCut &sym_cut, bool fw, Plan &plan);
  void extract_all_zero_plans(SymCut &sym_cut, bool fw, Plan &plan);

  // Return wether a zero cost reconstruction step was necessary
  bool reconstruct_zero_action(SymCut &sym_cut, bool fw,
                               std::shared_ptr<ClosedList> closed,
                               const Plan &plan);
  bool reconstruct_cost_action(SymCut &sym_cut, bool fw,
                               std::shared_ptr<ClosedList> closed,
                               const Plan &plan);

public:
  PlanReconstructor(UnidirectionalSearch *uni_search_fw,
                    UnidirectionalSearch *uni_search_bw,
                    std::shared_ptr<SymVariables> sym_vars,
                    std::shared_ptr<StateRegistry> state_registry);

  // Resets found plans and desired_num_plans which are helper functions
  void reconstruct_plans(const SymCut &cut, size_t desired_num_plans,
                         std::vector<Plan> &plans);
};

} // namespace symbolic

#endif