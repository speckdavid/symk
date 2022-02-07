#include "sym_solution_registry.h"
#include "../searches/uniform_cost_search.h"

using namespace std;

namespace symbolic {
//////////////// Plan Reconstruction /////////////////////////

void SymSolutionRegistry::add_plan(const Plan &plan) const {
    plan_data_base->add_plan(plan);
    if (!plan_data_base->found_enough_plans() && task_has_zero_costs()
        && plan_data_base->has_zero_cost_loop(plan)) {
        pair<int, int> zero_cost_op_seq =
            plan_data_base->get_first_zero_cost_loop(plan);
        Plan cur_plan = plan;
        utils::g_log << "Zero cost loop detected!" << endl;
        while (!plan_data_base->found_enough_plans()) {
            cur_plan.insert(cur_plan.begin() + zero_cost_op_seq.first,
                            plan.begin() + zero_cost_op_seq.first,
                            plan.begin() + zero_cost_op_seq.second + 1);
            plan_data_base->add_plan(cur_plan);
        }
    }
}

void SymSolutionRegistry::reconstruct_plans(const SymSolutionCut &cut) {
    Plan plan;
    SymSolutionCut modifiable_cut = cut;

    if (fw_search && !bw_search) {
        modifiable_cut.set_h(0);
    }
    if (!fw_search && bw_search) {
        modifiable_cut.set_g(0);
    }

    if (fw_search) {
        extract_all_plans(modifiable_cut, true, plan);
    } else {
        extract_all_plans(modifiable_cut, false, plan);
    }
}

void SymSolutionRegistry::extract_all_plans(SymSolutionCut &sym_cut, bool fw,
                                            Plan plan) {
    if (!plan_data_base->reconstruct_solutions(sym_cut)) {
        return;
    }

    if (!task_has_zero_costs()) {
        extract_all_cost_plans(sym_cut, fw, plan);
    } else {
        extract_all_zero_plans(sym_cut, fw, plan);
    }
}

void SymSolutionRegistry::extract_all_cost_plans(SymSolutionCut &sym_cut,
                                                 bool fw, Plan &plan) {
    // utils::g_log << sym_cut << endl;
    if (sym_cut.get_g() == 0 && sym_cut.get_h() == 0) {
        add_plan(plan);
        return;
    }

    // Resolve cost action
    if (fw) {
        if (sym_cut.get_g() > 0) {
            reconstruct_cost_action(sym_cut, true, fw_search->getClosedShared(),
                                    plan);
        } else {
            SymSolutionCut new_cut(0, sym_cut.get_h(),
                                   plan_data_base->get_final_state(plan),
                                   sym_cut.get_sol_cost());
            reconstruct_cost_action(new_cut, false, bw_search->getClosedShared(),
                                    plan);
        }
    } else {
        reconstruct_cost_action(sym_cut, false, bw_search->getClosedShared(), plan);
    }
}

void SymSolutionRegistry::extract_all_zero_plans(SymSolutionCut &sym_cut,
                                                 bool fw, Plan &plan) {
    BDD intersection;
    // Only zero costs left!
    if (sym_cut.get_g() == 0 && sym_cut.get_h() == 0) {
        // Check wether we are really in a initial or goal state
        if (fw && !bw_search) {
            intersection =
                sym_cut.get_cut() * fw_search->getClosedShared()->get_start_states();
            if (!intersection.IsZero()) {
                add_plan(plan);
                if (!plan_data_base->reconstruct_solutions(sym_cut)) {
                    return;
                }
            }
            reconstruct_zero_action(sym_cut, true, fw_search->getClosedShared(),
                                    plan);
        } else if (fw && bw_search) {
            intersection =
                sym_cut.get_cut() * fw_search->getClosedShared()->get_start_states();
            if (!intersection.IsZero()) {
                SymSolutionCut new_cut(0, sym_cut.get_h(),
                                       plan_data_base->get_final_state(plan),
                                       sym_cut.get_sol_cost());

                intersection = new_cut.get_cut() *
                    bw_search->getClosedShared()->get_start_states();
                if (!intersection.IsZero()) {
                    add_plan(plan);
                    if (!plan_data_base->reconstruct_solutions(sym_cut)) {
                        return;
                    }
                }
                reconstruct_zero_action(new_cut, false, bw_search->getClosedShared(),
                                        plan);
            }
            reconstruct_zero_action(sym_cut, true, fw_search->getClosedShared(),
                                    plan);
        } else { // bw
            intersection =
                sym_cut.get_cut() * bw_search->getClosedShared()->get_start_states();
            if (!intersection.IsZero()) {
                add_plan(plan);
                if (!plan_data_base->reconstruct_solutions(sym_cut)) {
                    return;
                }
            }
            reconstruct_zero_action(sym_cut, false, bw_search->getClosedShared(),
                                    plan);
        }
    } else {
        // Some cost left!
        if (fw) {
            if (sym_cut.get_g() > 0) {
                reconstruct_cost_action(sym_cut, true, fw_search->getClosedShared(),
                                        plan);
            } else {
                intersection = sym_cut.get_cut() *
                    fw_search->getClosedShared()->get_start_states();
                if (!intersection.IsZero()) {
                    SymSolutionCut new_cut(0, sym_cut.get_h(), plan_data_base->get_final_state(plan), sym_cut.get_sol_cost());
                    reconstruct_cost_action(new_cut, false, bw_search->getClosedShared(),
                                            plan);
                    reconstruct_zero_action(new_cut, false, bw_search->getClosedShared(),
                                            plan);
                }
            }
            reconstruct_zero_action(sym_cut, true, fw_search->getClosedShared(),
                                    plan);
        } else {
            reconstruct_cost_action(sym_cut, false, bw_search->getClosedShared(),
                                    plan);
            reconstruct_zero_action(sym_cut, false, bw_search->getClosedShared(),
                                    plan);
        }
    }
}

void SymSolutionRegistry::reconstruct_zero_action(
    SymSolutionCut &sym_cut, bool fw, shared_ptr<ClosedList> closed,
    const Plan &plan) {
    int cur_cost = fw ? sym_cut.get_g() : sym_cut.get_h();
    BDD cut = sym_cut.get_cut();

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
                if (fw) {
                    new_plan.insert(new_plan.begin(), *(tr.getOpsIds().begin()));
                } else {
                    new_plan.push_back(*(tr.getOpsIds().begin()));
                }
                SymSolutionCut new_cut(sym_cut.get_g(), sym_cut.get_h(),
                                       intersection, sym_cut.get_sol_cost());
                extract_all_plans(new_cut, fw, new_plan);

                if (!plan_data_base->reconstruct_solutions(new_cut)) {
                    return;
                }
            }
        }
    }
}

void SymSolutionRegistry::reconstruct_cost_action(
    SymSolutionCut &sym_cut, bool fw, shared_ptr<ClosedList> closed,
    const Plan &plan) {
    int cur_cost = fw ? sym_cut.get_g() : sym_cut.get_h();

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
            SymSolutionCut new_cut(0, 0, intersection, sym_cut.get_sol_cost());
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

            if (!plan_data_base->reconstruct_solutions(new_cut)) {
                return;
            }
        }
    }
}

////// Plan registry

SymSolutionRegistry::SymSolutionRegistry()
    : single_solution(true), sym_vars(nullptr), fw_search(nullptr),
      bw_search(nullptr), plan_data_base(nullptr), plan_cost_bound(-1) {}

void SymSolutionRegistry::init(shared_ptr<SymVariables> sym_vars,
                               UniformCostSearch *fwd_search,
                               UniformCostSearch *bwd_search,
                               shared_ptr<PlanDataBase> plan_data_base,
                               bool single_solution) {
    this->sym_vars = sym_vars;
    this->plan_data_base = plan_data_base;
    this->fw_search = fwd_search;
    this->bw_search = bwd_search;
    this->trs = fwd_search
        ? fwd_search->getStateSpaceShared()->getIndividualTRs()
        : bwd_search->getStateSpaceShared()->getIndividualTRs();
    this->single_solution = single_solution;
}

void SymSolutionRegistry::register_solution(const SymSolutionCut &solution) {
    if (single_solution) {
        if (sym_cuts.empty()) {
            sym_cuts.push_back(solution);
        } else {
            sym_cuts[0] = solution;
        }
        return;
    }

    bool merged = false;
    size_t pos = 0;
    for (; pos < sym_cuts.size(); pos++) {
        // a cut with same g and h values exist
        // => we combine the cut to avoid multiple cuts with same solutions
        if (sym_cuts[pos] == solution) {
            sym_cuts[pos].merge(solution);
            merged = true;
            break;
        }
        if (sym_cuts[pos] > solution) {
            break;
        }
    }
    if (!merged) {
        sym_cuts.insert(sym_cuts.begin() + pos, solution);
    }
}

void SymSolutionRegistry::construct_cheaper_solutions(int bound) {
    bool bound_used = false;
    int min_plan_bound = numeric_limits<int>::max();

    while (sym_cuts.size() > 0 && sym_cuts.at(0).get_f() < bound
           && !found_all_plans()) {
        // Ignore cuts with costs smaller than the proven cost bound
        // This occurs only in bidirectional search
        if (sym_cuts.at(0).get_f() < plan_cost_bound) {
            sym_cuts.erase(sym_cuts.begin());
        } else {
            min_plan_bound = min(min_plan_bound, sym_cuts.at(0).get_f());
            bound_used = true;

            reconstruct_plans(sym_cuts[0]);
            sym_cuts.erase(sym_cuts.begin());
        }
    }

    // Update the plan bound
    if (bound_used) {
        plan_cost_bound = min_plan_bound;
    }
}
} // namespace symbolic
