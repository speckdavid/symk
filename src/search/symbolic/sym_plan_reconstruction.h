#ifndef SYMBOLIC_SYM_PLAN_RECONSTRUCTION_H
#define SYMBOLIC_SYM_PLAN_RECONSTRUCTION_H

#include "../plan_manager.h"
#include "sym_solution_registry.h"
#include "unidirectional_search.h"
#include "plan_selection/plan_database.h"

namespace symbolic
{

class ClosedList;
class PlanDataBase;

class SymPlanReconstructor
{
protected:
  UnidirectionalSearch *uni_search_fw;
  UnidirectionalSearch *uni_search_bw;
  std::shared_ptr<SymVariables> sym_vars;
  std::map<int, std::vector<TransitionRelation>> trs;
  
  std::shared_ptr<PlanDataBase> plan_data_base;

  bool task_has_zero_costs() const
  {
    return trs.count(0) > 0;
  }

  bool bw_reconstruction_necessary() const;

  BDD get_resulting_state(const Plan &plan) const;

  // Extracts all plans by a DFS, we copy the current plan suffix by every
  // recusive call which is why we don't use any reference for plan
  // BID: After reconstruction of the forward part we reverse the plan and
  // call extract_all_plans in bw direction which completes the plan
  // After completing a plan we store it in found plans!
  void extract_all_plans(SymSolutionCut &sym_cut, bool fw, Plan plan);

  void extract_all_cost_plans(SymSolutionCut &sym_cut, bool fw, Plan &plan);
  void extract_all_zero_plans(SymSolutionCut &sym_cut, bool fw, Plan &plan);

  // Return wether a zero cost reconstruction step was necessary
  bool reconstruct_zero_action(SymSolutionCut &sym_cut, bool fw,
                               std::shared_ptr<ClosedList> closed,
                               const Plan &plan);
  bool reconstruct_cost_action(SymSolutionCut &sym_cut, bool fw,
                               std::shared_ptr<ClosedList> closed,
                               const Plan &plan);

public:
  SymPlanReconstructor(UnidirectionalSearch *uni_search_fw,
                    UnidirectionalSearch *uni_search_bw,
                    std::shared_ptr<SymVariables> sym_vars,
                    std::shared_ptr<PlanDataBase> plan_data_base);

  void reconstruct_plans(const SymSolutionCut &cut);
};

} // namespace symbolic

#endif
