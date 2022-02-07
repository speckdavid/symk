#include "simple_sym_solution_registry.h"
#include "../../task_utils/task_properties.h"
#include "../searches/uniform_cost_search.h"
#include "../tasks/root_task.h"

using namespace std;

namespace symbolic {
BDD SimpleSymSolutionRegistry::get_visited_states(const Plan &plan) const {
    return plan_data_base->states_on_path(plan);
}

void SimpleSymSolutionRegistry::add_plan(const Plan &plan) const {
    plan_data_base->add_plan(plan);
    assert(!plan_data_base->has_zero_cost_loop(plan));
}

void SimpleSymSolutionRegistry::reconstruct_plans(
    const SymSolutionCut &sym_cut) {
    Plan plan;
    SimpleSymSolutionCut new_simple_cut(sym_cut, sym_cut.get_cut());

    if (fw_search && !bw_search) {
        new_simple_cut.set_h(0);
    }
    if (!fw_search && bw_search) {
        new_simple_cut.set_g(0);
    }

    // early pruning:
    if (fw_search && (sym_cut.get_g() > 0)) { // remove s_0 from cut in fw search
        new_simple_cut.set_cut(new_simple_cut.get_cut() *
                               !fw_search->getClosedShared()->get_start_states());
        new_simple_cut.set_visited_states(new_simple_cut.get_cut());
    }

    if (fw_search) {
        double nr_states = sym_vars->numStates(new_simple_cut.get_cut());
        if (nr_states <= 1) {
            extract_all_plans(new_simple_cut, true, plan);
        } else {
            BDD visited = sym_vars->zeroBDD();
            BDD states = new_simple_cut.get_cut();
            extract_one_by_one(states, visited, new_simple_cut, plan);
        }
    } else {
        extract_all_plans(new_simple_cut, false, plan);
    }
}

void SimpleSymSolutionRegistry::extract_all_plans(SimpleSymSolutionCut &sym_cut,
                                                  bool fw, Plan plan) {
    // From here on sym_cut contains only one state! (made sure by
    // extract_one_by_one in fw search)
    if (!plan_data_base->reconstruct_solutions(sym_cut)) {
        return;
    }
    if (!task_has_zero_costs()) {
        extract_all_cost_plans(sym_cut, fw, plan);
    } else {
        extract_all_zero_plans(sym_cut, fw, plan);
    }
}

void SimpleSymSolutionRegistry::extract_all_cost_plans(
    SimpleSymSolutionCut &simple_cut, bool fw, Plan &plan) {
    if (simple_cut.get_g() == 0 && simple_cut.get_h() == 0) {
        add_plan(plan);
        return;
    }

    // Resolve cost action
    if (fw) {
        if (simple_cut.get_g() > 0) {
            reconstruct_cost_action(simple_cut, true, fw_search->getClosedShared(),
                                    plan);
        } else {
            // The left part of plan is complete, but the right hand side is missing.
            BDD resulting_state = plan_data_base->get_final_state(plan);

            SimpleSymSolutionCut new_cut(0, simple_cut.get_h(),
                                         resulting_state,
                                         simple_cut.get_sol_cost(),
                                         get_visited_states(plan));
            reconstruct_cost_action(new_cut, false, bw_search->getClosedShared(),
                                    plan);
        }
    } else {
        reconstruct_cost_action(simple_cut, false, bw_search->getClosedShared(),
                                plan);
    }
}

void SimpleSymSolutionRegistry::extract_all_zero_plans(
    SimpleSymSolutionCut &simple_cut, bool fw, Plan &plan) {
    BDD intersection;
    // Only zero costs left!
    if (simple_cut.get_g() == 0 && simple_cut.get_h() == 0) {
        // Check wether we are really in a initial or goal state
        if (fw && !bw_search) {
            intersection = simple_cut.get_cut() *
                fw_search->getClosedShared()->get_start_states();
            if (!intersection.IsZero()) {
                add_plan(plan);
                return; // No simple plans possible any more, as s_0 already found!
            }
            reconstruct_zero_action(simple_cut, true, fw_search->getClosedShared(),
                                    plan);
        } else if (fw && bw_search) { // Bidirectional search
            intersection = simple_cut.get_cut() *
                fw_search->getClosedShared()->get_start_states();
            if (!intersection.IsZero()) {
                // The left part of plan is complete, but the right hand side is
                // missing.
                BDD resulting_state = plan_data_base->get_final_state(plan);
                intersection =
                    resulting_state * bw_search->getClosedShared()->get_start_states();
                // bw_start_states == goals, so no intersection with goals necessary
                if (!intersection.IsZero()) {
                    add_plan(plan);
                    if (!plan_data_base->reconstruct_solutions(simple_cut)) {
                        return;
                    }
                }
                SimpleSymSolutionCut new_simple_cut(
                    0, simple_cut.get_h(), resulting_state,
                    simple_cut.get_sol_cost(), get_visited_states(plan));
                reconstruct_zero_action(new_simple_cut, false,
                                        bw_search->getClosedShared(), plan);
            }
            reconstruct_zero_action(simple_cut, true, fw_search->getClosedShared(),
                                    plan);
        } else { // bw search
            intersection = simple_cut.get_cut() *
                bw_search->getClosedShared()->get_start_states();
            if (!intersection.IsZero()) {
                add_plan(plan);
                if (!plan_data_base->reconstruct_solutions(simple_cut)) {
                    return;
                }
            }
            reconstruct_zero_action(simple_cut, false, bw_search->getClosedShared(),
                                    plan);
        }
    } else { // g>0 or h>0
        if (fw) {
            if (simple_cut.get_g() > 0) {
                reconstruct_cost_action(simple_cut, true, fw_search->getClosedShared(),
                                        plan);
            } else { // g==0 and h>0
                intersection = simple_cut.get_cut() *
                    fw_search->getClosedShared()->get_start_states();
                if (!intersection.IsZero()) {
                    SimpleSymSolutionCut new_simple_cut(
                        0,
                        simple_cut.get_h(),
                        plan_data_base->get_final_state(plan),
                        simple_cut.get_sol_cost(),
                        get_visited_states(plan));
                    reconstruct_cost_action(new_simple_cut, false,
                                            bw_search->getClosedShared(), plan);
                    reconstruct_zero_action(new_simple_cut, false,
                                            bw_search->getClosedShared(), plan);
                }
            }
            reconstruct_zero_action(simple_cut, true, fw_search->getClosedShared(),
                                    plan);
        } else { // bw search
            reconstruct_cost_action(simple_cut, false, bw_search->getClosedShared(),
                                    plan);
            reconstruct_zero_action(simple_cut, false, bw_search->getClosedShared(),
                                    plan);
        }
    }
}

void SimpleSymSolutionRegistry::reconstruct_cost_action(
    SimpleSymSolutionCut &cur_cut, bool fw, shared_ptr<ClosedList> closed,
    const Plan &plan) {
    int cur_cost = fw ? cur_cut.get_g() : cur_cut.get_h();

    for (auto key : trs) {
        int new_cost = cur_cost - key.first;
        if (key.first == 0 || new_cost < 0) {
            continue;
        }
        for (TransitionRelation &tr : key.second) {
            BDD succ_states =
                fw ? tr.preimage(cur_cut.get_cut()) : tr.image(cur_cut.get_cut());
            succ_states = succ_states * closed->get_closed_at(new_cost) *
                !cur_cut.get_visited_states();
            if (fw && (new_cost > 0)) {
                // will always form a loop
                succ_states *= !fw_search->getClosedShared()->get_start_states();
            }
            if (succ_states.IsZero()) {
                continue;
            }

            Plan new_plan = plan;
            SimpleSymSolutionCut new_simple_cut(
                0, 0, succ_states, cur_cut.get_sol_cost(),
                cur_cut.get_visited_states() + succ_states);
            if (fw) {
                new_plan.insert(new_plan.begin(), *(tr.getOpsIds().begin()));
                new_simple_cut.set_g(new_cost);
                new_simple_cut.set_h(cur_cut.get_h());
            } else {
                new_plan.push_back(*(tr.getOpsIds().begin()));
                new_simple_cut.set_g(cur_cut.get_g());
                new_simple_cut.set_h(new_cost);
            }

            if (fw) {
                double nr_states = sym_vars->numStates(succ_states);
                if (nr_states <= 1) {
                    extract_all_plans(new_simple_cut, true, new_plan);
                } else {
                    BDD visited = cur_cut.get_visited_states();
                    extract_one_by_one(succ_states, visited, new_simple_cut, new_plan);
                }
            } else {
                extract_all_plans(new_simple_cut, false, new_plan);
            }

            if (!plan_data_base->reconstruct_solutions(new_simple_cut)) {
                return;
            }
        }
    }
}

void SimpleSymSolutionRegistry::reconstruct_zero_action(
    SimpleSymSolutionCut &cur_cut, bool fw, shared_ptr<ClosedList> closed,
    const Plan &plan) {
    int cur_cost = fw ? cur_cut.get_g() : cur_cut.get_h();
    BDD cur_states = cur_cut.get_cut();

    BDD succ_states;
    for (size_t newSteps0 = 0;
         newSteps0 < closed->get_num_zero_closed_layers(cur_cost); newSteps0++) {
        for (const TransitionRelation &tr : trs.at(0)) {
            BDD succ_states_image =
                fw ? tr.preimage(cur_states) : tr.image(cur_states);
            BDD succ_states_closed =
                succ_states_image * closed->get_zero_closed_at(cur_cost, newSteps0);
            BDD succ_states =
                succ_states_closed * !cur_cut.get_visited_states(); // avoid loops
            if (fw && (cur_cut.get_g() > 0)) {
                // will always form a loop
                succ_states *= !fw_search->getClosedShared()->get_start_states();
            }

            if (succ_states.IsZero()) {
                continue;
            }

            Plan new_plan = plan;
            SimpleSymSolutionCut new_simple_cut(
                cur_cut.get_g(), cur_cut.get_h(), succ_states,
                cur_cut.get_sol_cost(),
                cur_cut.get_visited_states() + succ_states);
            if (fw) {
                new_plan.insert(new_plan.begin(), *(tr.getOpsIds().begin()));
            } else {
                new_plan.push_back(*(tr.getOpsIds().begin()));
            }

            if (fw) {
                double nr_states = sym_vars->numStates(succ_states);
                if (nr_states <= 1) {
                    extract_all_plans(new_simple_cut, true, new_plan);
                } else {
                    BDD visited = cur_cut.get_visited_states();
                    extract_one_by_one(succ_states, visited, new_simple_cut, new_plan);
                }
            } else {
                extract_all_plans(new_simple_cut, false, new_plan);
            }

            if (!plan_data_base->reconstruct_solutions(new_simple_cut)) {
                return;
            }
        }
    }
}

/**
 * Naive way of extracting one state after the other from the bdd and
 * reconstructing from it. Faster than using the generator if the number of
 * states in the given bdd < 100 (threshold only guessed - depending on domain).
 */
void SimpleSymSolutionRegistry::extract_one_by_one(
    BDD states, BDD &visited, SimpleSymSolutionCut &simple_cut, Plan &plan) {
    BDD remaining = states;
    while (!remaining.IsZero() && plan_data_base->reconstruct_solutions(simple_cut)) {
        State next_state = sym_vars->getStateFrom(remaining);
        BDD next_state_bdd = sym_vars->getStateBDD(next_state.get_values());
        simple_cut.set_cut(next_state_bdd); // Only one single state!
        simple_cut.set_visited_states(visited + next_state_bdd);
        extract_all_plans(simple_cut, true, plan);
        remaining *= !next_state_bdd; // take the state out!
    }
}
} // namespace symbolic
