#include "sym_plan_reconstruction.h"
#include "closed_list.h"
#include "transition_relation.h"

namespace symbolic {

void SymPlanReconstructor::add_plan(const Plan &plan) const {
  plan_data_base->add_plan(plan);
  if (!plan_data_base->found_enough_plans() && task_has_zero_costs() &&
      plan_data_base->has_zero_cost_loop(plan)) {
    std::pair<int, int> zero_cost_op_seq =
        plan_data_base->get_first_zero_cost_loop(plan);
    Plan cur_plan = plan;
    std::cout << " => zero cost loop detected =>" << std::flush;
    while (!plan_data_base->found_enough_plans()) {
      cur_plan.insert(cur_plan.begin() + zero_cost_op_seq.first,
                      plan.begin() + zero_cost_op_seq.first,
                      plan.begin() + zero_cost_op_seq.second + 1);
      plan_data_base->add_plan(cur_plan);
    }
  }
}

BDD SymPlanReconstructor::get_resulting_state(const Plan &plan) const {
  GlobalState cur = sym_vars->get_state_registry()->get_initial_state();
  for (auto &op : plan) {
    cur = sym_vars->get_state_registry()->get_successor_state(
        cur,
        sym_vars->get_state_registry()->get_task_proxy().get_operators()[op]);
  }
  return sym_vars->getStateBDD(cur);
}

void SymPlanReconstructor::extract_all_plans(SymSolutionCut &sym_cut, bool fw,
                                             Plan plan) {
  if (plan_data_base->found_enough_plans()) {
    return;
  }

  if (!task_has_zero_costs()) {
    extract_all_cost_plans(sym_cut, fw, plan);
  } else {
    extract_all_zero_plans(sym_cut, fw, plan);
  }
}

void SymPlanReconstructor::extract_all_cost_plans(SymSolutionCut &sym_cut,
                                                  bool fw, Plan &plan) {
  // std::cout << sym_cut << std::endl;
  if (sym_cut.get_g() == 0 && sym_cut.get_h() == 0) {
    add_plan(plan);
    return;
  }

  // Resolve cost action
  if (fw) {
    if (sym_cut.get_g() > 0) {
      reconstruct_cost_action(sym_cut, true, uni_search_fw->getClosedShared(),
                              plan);
    } else {
      SymSolutionCut new_cut(0, sym_cut.get_h(), get_resulting_state(plan));
      reconstruct_cost_action(new_cut, false, uni_search_bw->getClosedShared(),
                              plan);
    }
  } else {
    reconstruct_cost_action(sym_cut, false, uni_search_bw->getClosedShared(),
                            plan);
  }
}

void SymPlanReconstructor::extract_all_zero_plans(SymSolutionCut &sym_cut,
                                                  bool fw, Plan &plan) {
  BDD intersection;
  // Only zero costs left!
  if (sym_cut.get_g() == 0 && sym_cut.get_h() == 0) {
    // Check wether we are really in a initial or goal state
    if (fw && !uni_search_bw) {
      intersection = sym_cut.get_cut() *
                     uni_search_fw->getClosedShared()->get_start_states();
      if (!intersection.IsZero()) {
        add_plan(plan);
        if (plan_data_base->found_enough_plans()) {
          return;
        }
      }
      reconstruct_zero_action(sym_cut, true, uni_search_fw->getClosedShared(),
                              plan);
    } else if (fw && uni_search_bw) {
      intersection = sym_cut.get_cut() *
                     uni_search_fw->getClosedShared()->get_start_states();
      if (!intersection.IsZero()) {
        SymSolutionCut new_cut(0, sym_cut.get_h(), get_resulting_state(plan));

        intersection = new_cut.get_cut() *
                       uni_search_bw->getClosedShared()->get_start_states();
        if (!intersection.IsZero()) {
          add_plan(plan);
          if (plan_data_base->found_enough_plans()) {
            return;
          }
        }
        reconstruct_zero_action(new_cut, false,
                                uni_search_bw->getClosedShared(), plan);
      }
      reconstruct_zero_action(sym_cut, true, uni_search_fw->getClosedShared(),
                              plan);
    } else { // bw
      intersection = sym_cut.get_cut() *
                     uni_search_bw->getClosedShared()->get_start_states();
      if (!intersection.IsZero()) {
        add_plan(plan);
        if (plan_data_base->found_enough_plans()) {
          return;
        }
      }
      reconstruct_zero_action(sym_cut, false, uni_search_bw->getClosedShared(),
                              plan);
    }
  } else {
    // Some cost left!
    if (fw) {
      if (sym_cut.get_g() > 0) {
        reconstruct_cost_action(sym_cut, true, uni_search_fw->getClosedShared(),
                                plan);
      } else {
        intersection = sym_cut.get_cut() *
                       uni_search_fw->getClosedShared()->get_start_states();
        if (!intersection.IsZero()) {
          SymSolutionCut new_cut(0, sym_cut.get_h(), get_resulting_state(plan));
          reconstruct_cost_action(new_cut, false,
                                  uni_search_bw->getClosedShared(), plan);
          reconstruct_zero_action(new_cut, false,
                                  uni_search_bw->getClosedShared(), plan);
        }
      }
      reconstruct_zero_action(sym_cut, true, uni_search_fw->getClosedShared(),
                              plan);
    } else {
      reconstruct_cost_action(sym_cut, false, uni_search_bw->getClosedShared(),
                              plan);
      reconstruct_zero_action(sym_cut, false, uni_search_bw->getClosedShared(),
                              plan);
    }
  }
}

bool SymPlanReconstructor::reconstruct_zero_action(
    SymSolutionCut &sym_cut, bool fw, std::shared_ptr<ClosedList> closed,
    const Plan &plan) {
  int cur_cost = fw ? sym_cut.get_g() : sym_cut.get_h();
  BDD cut = sym_cut.get_cut();

  bool some_action_found = false;
  BDD succ;
  for (size_t newSteps0 = 0;
       newSteps0 < closed->get_num_zero_closed_layers(cur_cost); newSteps0++) {
    for (const TransitionRelation &tr : trs.at(0)) {
      succ = fw ? tr.preimage(cut) : tr.image(cut);
      if (succ.IsZero()) {
        continue;
      }

      BDD intersection = succ * closed->get_zero_closed_at(cur_cost, newSteps0);
      if (!intersection.IsZero()) {
        Plan new_plan = plan;
        some_action_found = true;
        if (fw) {
          new_plan.insert(new_plan.begin(), *(tr.getOpsIds().begin()));
        } else {
          new_plan.push_back(*(tr.getOpsIds().begin()));
        }
        SymSolutionCut new_cut(sym_cut.get_g(), sym_cut.get_h(), intersection);
        extract_all_plans(new_cut, fw, new_plan);

        if (plan_data_base->found_enough_plans()) {
          return true;
        }
      }
    }
  }
  return some_action_found;
}

bool SymPlanReconstructor::reconstruct_cost_action(
    SymSolutionCut &sym_cut, bool fw, std::shared_ptr<ClosedList> closed,
    const Plan &plan) {
  int cur_cost = fw ? sym_cut.get_g() : sym_cut.get_h();
  bool some_action_found = false;

  for (auto key : trs) {
    int new_cost = cur_cost - key.first;
    if (key.first == 0 || new_cost < 0) {
      continue;
    }
    for (TransitionRelation &tr : key.second) {
      BDD succ =
          fw ? tr.preimage(sym_cut.get_cut()) : tr.image(sym_cut.get_cut());
      BDD intersection = succ * closed->get_closed_at(new_cost);
      if (intersection.IsZero()) {
        continue;
      }
      Plan new_plan = plan;
      some_action_found = true;
      SymSolutionCut new_cut(0, 0, intersection);
      if (fw) {
        new_plan.insert(new_plan.begin(), *(tr.getOpsIds().begin()));
        new_cut.set_g(new_cost);
        new_cut.set_h(sym_cut.get_h());
      } else {
        new_plan.push_back(*(tr.getOpsIds().begin()));
        new_cut.set_g(sym_cut.get_g());
        new_cut.set_h(new_cost);
      }
      extract_all_plans(new_cut, fw, new_plan);

      if (plan_data_base->found_enough_plans()) {
        return true;
      }
    }
  }
  return some_action_found;
}

SymPlanReconstructor::SymPlanReconstructor(
    UniformCostSearch *uni_search_fw, UniformCostSearch *uni_search_bw,
    std::shared_ptr<SymVariables> sym_vars,
    std::shared_ptr<PlanDataBase> plan_data_base)
    : uni_search_fw(uni_search_fw), uni_search_bw(uni_search_bw),
      sym_vars(sym_vars), plan_data_base(plan_data_base) {
  auto cur_search = uni_search_fw ? uni_search_fw : uni_search_bw;
  auto cur_closed = cur_search->getClosedShared();
  trs = cur_search->getStateSpaceShared()->getIndividualTRs();
}

void SymPlanReconstructor::reconstruct_plans(const SymSolutionCut &cut) {
  Plan plan;
  SymSolutionCut modifiable_cut = cut;

  if (uni_search_fw && !uni_search_bw) {
    modifiable_cut.set_h(0);
  }
  if (!uni_search_fw && uni_search_bw) {
    modifiable_cut.set_g(0);
  }

  if (uni_search_fw) {
    extract_all_plans(modifiable_cut, true, plan);
  } else {
    extract_all_plans(modifiable_cut, false, plan);
  }
}

} // namespace symbolic
