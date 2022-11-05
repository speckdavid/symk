#include "sym_solution_registry.h"
#include "../searches/uniform_cost_search.h"

using namespace std;

namespace symbolic {
void SymSolutionRegistry::add_plan(const Plan &plan) const {
    assert(!(simple_solutions() && plan_data_base->has_zero_cost_loop(plan)));
    plan_data_base->add_plan(plan);
}

void SymSolutionRegistry::reconstruct_plans(
    const vector<SymSolutionCut> &sym_cuts) {
    assert(queue.empty());

    for (const SymSolutionCut &sym_cut : sym_cuts) {
        assert(fw_closed || sym_cut.get_g() == 0);
        assert(bw_closed || sym_cut.get_h() == 0);

        ReconstructionNode cur_node(sym_cut.get_g(),
                                    sym_cut.get_h(),
                                    numeric_limits<int>::max(),
                                    sym_cut.get_cut(),
                                    sym_vars->zeroBDD(),
                                    fw_closed != nullptr,
                                    0);
        queue.push(cur_node);

        // In the bidirectional case we might can directly swap the direction
        if (swap_to_bwd_phase(cur_node)) {
            ReconstructionNode bw_node = cur_node;
            bw_node.set_states(fw_closed->get_start_states());
            bw_node.set_visited_states(fw_closed->get_start_states());
            bw_node.set_fwd_phase(false);
            queue.push(bw_node);
        }
    }

    // While queue is not empty
    while (!queue.empty()) {
        ReconstructionNode cur_node = queue.top();
        queue.pop();

        // If we do simple planning, we extract a single state form the relevant states
        // and process it
        if (simple_solutions()) {
            if (sym_vars->numStates(cur_node.get_states()) > 1) {
                BDD state_bdd = sym_vars->getSinglePrimaryStateFrom(cur_node.get_states());
                ReconstructionNode remaining_node = cur_node;
                remaining_node.set_states(remaining_node.get_states() * !state_bdd);
                if (sym_vars->numStates(remaining_node.get_states()) > 0) {
                    queue.push(remaining_node);
                }
                assert(sym_vars->numStates(remaining_node.get_states()) > 0
                       || remaining_node.get_states() == sym_vars->zeroBDD());
                cur_node.set_states(state_bdd);
            }
            cur_node.add_visited_states(cur_node.get_states());
        }

        // utils::g_log << cur_node << endl;

        assert(sym_vars->numStates(cur_node.get_states()) > 0);
        assert(!simple_solutions() || sym_vars->numStates(cur_node.get_states()) == 1);
        assert(!simple_solutions() || cur_node.get_plan_length() + 1 == sym_vars->numStates(cur_node.get_visitied_states()));

        // Check if we have found a solution with this cut
        if (is_solution(cur_node)) {
            Plan cur_plan;
            cur_node.get_plan(cur_plan);
            add_plan(cur_plan);

            // Plan data base tells us if we need to continue
            // We can stop early if we, e.g., have found enough plans
            if (!plan_data_base->reconstruct_solutions(sym_cuts[0].get_f())) {
                queue = ReconstructionQueue(CompareReconstructionNodes(ReconstructionPriority::REMAINING_COST));
                return;
            }

            // Not necessary to with this plan since it can only lead to
            // unjustified plans
            if (justified_solutions()) {
                continue;
            }
        }
        expand_actions(cur_node);
    }
    assert(queue.empty());
}

void SymSolutionRegistry::expand_actions(const ReconstructionNode &node) {
    bool fwd = node.is_fwd_phase();
    int cur_cost;
    shared_ptr<ClosedList> cur_closed_list;

    if (fwd) {
        cur_cost = node.get_g();
        cur_closed_list = fw_closed;
    } else {
        cur_cost = node.get_h();
        cur_closed_list = bw_closed;
    }

    // Traverse in oposite direction to first consider actions with higher costs
    // Mostly relevant for single solution reconstruction
    for (auto it = trs.rbegin(); it != trs.rend(); it++) {
        int op_cost = it->first;
        int new_cost = cur_cost - op_cost;

        // new cost can not be negative
        if (new_cost < 0) {
            continue;
        }

        for (const TransitionRelation &tr : it->second) {
            BDD succ = fwd ? tr.preimage(node.get_states()) : tr.image(node.get_states());

            BDD closed_states = cur_closed_list->get_closed_at(new_cost);
            BDD intersection = succ * closed_states;
            int layer_id = 0;
            if (op_cost == 0)
                layer_id = cur_closed_list->get_zero_cut(new_cost, intersection);

            // Ignore states we have already visited
            if (simple_solutions()) {
                intersection *= !node.get_visitied_states();
            }

            if (intersection.IsZero()) {
                continue;
            }

            ReconstructionNode new_node(-1, -1, layer_id,
                                        intersection, node.get_visitied_states(),
                                        fwd, node.get_plan_length() + 1);
            if (fwd) {
                new_node.set_g(new_cost);
                new_node.set_h(node.get_h());
                new_node.set_predecessor(make_shared<ReconstructionNode>(node), make_shared<TransitionRelation>(tr));
            } else {
                new_node.set_g(node.get_g());
                new_node.set_h(new_cost);
                new_node.set_successor(make_shared<ReconstructionNode>(node), make_shared<TransitionRelation>(tr));
            }

            // We have sucessfully reconstructed to the initial state
            if (swap_to_bwd_phase(new_node)) {
                assert(fw_closed->get_start_states() == new_node.get_states());
                BDD middle_state = new_node.get_middle_state(fw_closed->get_start_states());
                ReconstructionNode bw_node(0, new_node.get_h(),
                                           numeric_limits<int>::max(), middle_state,
                                           new_node.get_visitied_states(),
                                           false, node.get_plan_length() + 1);
                bw_node.set_predecessor(make_shared<ReconstructionNode>(node), make_shared<TransitionRelation>(tr));

                // Add init state to visited states
                if (simple_solutions()) {
                    bw_node.add_visited_states(fw_closed->get_start_states());
                }

                queue.push(bw_node);

                if (task_has_zero_costs() && no_pruning()) {
                    queue.push(new_node);
                }
            } else {
                queue.push(new_node);
            }

            // A single solution and we made progress
            if (single_solution() &&
                (new_node.get_f() < node.get_f() ||
                 new_node.get_zero_layer() < node.get_zero_layer())) {
                return;
            }
        }
    }
}

bool SymSolutionRegistry::swap_to_bwd_phase(const ReconstructionNode &node) const {
    return bw_closed
           && node.is_fwd_phase()
           && node.get_g() == 0
           && !(node.get_states() * fw_closed->get_start_states()).IsZero();
}

bool SymSolutionRegistry::is_solution(const ReconstructionNode &node) const {
    if (node.get_f() > 0)
        return false;
    if (bw_closed && node.is_fwd_phase())
        return false;
    shared_ptr<ClosedList> closed = node.is_fwd_phase() ?
        fw_closed : bw_closed;
    return !(node.get_states() * closed->get_start_states()).IsZero();
}

SymSolutionRegistry::SymSolutionRegistry()
    : justified_solutions_pruning(false),
      single_solution_pruning(false),
      simple_solutions_pruning(false),
      fw_closed(nullptr),
      bw_closed(nullptr),
      plan_data_base(nullptr) {
    queue = ReconstructionQueue(CompareReconstructionNodes(ReconstructionPriority::REMAINING_COST));
}

void SymSolutionRegistry::init(shared_ptr<SymVariables> sym_vars,
                               shared_ptr<symbolic::ClosedList> fw_closed,
                               shared_ptr<symbolic::ClosedList> bw_closed,
                               map<int, vector<TransitionRelation>> &trs,
                               shared_ptr<PlanSelector> plan_data_base,
                               bool single_solution,
                               bool simple_solutions) {
    this->sym_vars = sym_vars;
    this->plan_data_base = plan_data_base;
    this->fw_closed = fw_closed;
    this->bw_closed = bw_closed;
    this->trs = trs;
    this->single_solution_pruning = single_solution;
    this->simple_solutions_pruning = simple_solutions;

    // If unit costs we simple use sort by remaining cost
    if (trs.size() == 1) {
        queue = ReconstructionQueue(CompareReconstructionNodes(ReconstructionPriority::REMAINING_COST));
    }
}

void SymSolutionRegistry::register_solution(const SymSolutionCut &solution) {
    if (single_solution()) {
        if (!sym_cuts.empty()) {
            sym_cuts = map<int, vector<SymSolutionCut>>();
        }
        sym_cuts[solution.get_f()].push_back(solution);
        return;
    }

    bool merged = false;
    size_t pos = 0;
    for (; pos < sym_cuts[solution.get_f()].size(); pos++) {
        // a cut with same g and h values exist
        // => we combine the cut to avoid multiple cuts with same solutions
        if (sym_cuts[solution.get_f()][pos] == solution) {
            sym_cuts[solution.get_f()][pos].merge(solution);
            merged = true;
            break;
        }
    }
    if (!merged) {
        sym_cuts[solution.get_f()].push_back(solution);
    }
}

void SymSolutionRegistry::construct_cheaper_solutions(int bound) {
    for (const auto &key : sym_cuts) {
        int plan_cost = key.first;
        const vector<SymSolutionCut> &cuts = key.second;
        if (plan_cost >= bound || found_all_plans())
            break;

        reconstruct_plans(cuts);
    }

    // Erase handled keys
    for (auto it = sym_cuts.begin(); it != sym_cuts.end();) {
        (it->first < bound) ? sym_cuts.erase(it++) : (++it);
    }
}
}
