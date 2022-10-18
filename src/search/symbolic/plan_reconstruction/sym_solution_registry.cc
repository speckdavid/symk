#include "sym_solution_registry.h"
#include "../searches/uniform_cost_search.h"

using namespace std;

namespace symbolic {
//////////////// Plan Reconstruction /////////////////////////

void SymSolutionRegistry::add_plan(const Plan &plan) const {
    plan_data_base->add_plan(plan);
    // assert(!plan_data_base->has_zero_cost_loop(plan));
}

void SymSolutionRegistry::reconstruct_plans(
    const vector<SymSolutionCut> &sym_cuts) {
    for (const SymSolutionCut &sym_cut : sym_cuts) {
        assert(fw_closed || sym_cut.get_g() == 0);
        assert(bw_closed || sym_cut.get_h() == 0);

        ReconstructionNode cur_node(sym_cut.get_g(), sym_cut.get_h(),
                                    sym_cut.get_cut(), fw_closed != nullptr, 0);
        queue.push(cur_node);

        // In the bidirectional case we might can directly swap the direction
        if (swap_to_bwd_phase(cur_node)) {
            ReconstructionNode bw_node = cur_node;
            bw_node.set_fwd_phase(false);
            queue.push(bw_node);
        }
    }

    // While queue is not empty
    while (!queue.empty()) {
        ReconstructionNode cur_node = queue.top();
        queue.pop();
        // utils::g_log << cur_node << endl;

        // Check if we have found a solution with this cut
        if (is_solution(cur_node)) {
            Plan cur_plan;
            cur_node.get_plan(cur_plan);
            add_plan(cur_plan);

            // Plan data base tells us if we need to continue
            // We can stop early if we, e.g., have found enough plans
            if (!plan_data_base->reconstruct_solutions(sym_cuts[0].get_f())) {
                return;
            }
            // continue;
        }
        expand_actions(cur_node);
    }
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

    for (auto const &key : trs) {
        int new_cost = cur_cost - key.first;

        // new cost can not be negative
        if (new_cost < 0) {
            continue;
        }

        for (const TransitionRelation &tr : key.second) {
            BDD succ = fwd ? tr.preimage(node.get_states()) : tr.image(node.get_states());
            BDD intersection = succ * cur_closed_list->get_closed_at(new_cost);
            if (intersection.IsZero()) {
                continue;
            }

            OperatorID op_id = *(tr.getOpsIds().begin());
            ReconstructionNode new_node(-1, -1, intersection, fwd, node.get_plan_length() + 1);
            if (fwd) {
                new_node.set_g(new_cost);
                new_node.set_h(node.get_h());
                new_node.set_predecessor(make_shared<ReconstructionNode>(node), op_id);
            } else {
                new_node.set_g(node.get_g());
                new_node.set_h(new_cost);
                new_node.set_successor(make_shared<ReconstructionNode>(node), op_id);
            }

            // We have sucessfully reconstructed to the initial state
            if (swap_to_bwd_phase(new_node)) {
                Plan partial_plan;
                new_node.get_plan(partial_plan);
                BDD middle_state = plan_data_base->get_final_state(partial_plan);
                ReconstructionNode bw_node(0, new_node.get_h(), middle_state,
                                           false, node.get_plan_length() + 1);
                bw_node.set_predecessor(make_shared<ReconstructionNode>(node), op_id);
                queue.push(bw_node);

                // We probably want to do the push below if we have zero cost
                // and have not simple path pruning
                if (task_has_zero_costs()) {
                    queue.push(new_node);
                }
            } else {
                queue.push(new_node);
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

////// Plan registry

SymSolutionRegistry::SymSolutionRegistry()
    : single_solution(true), fw_closed(nullptr),
      bw_closed(nullptr), plan_data_base(nullptr) {}

void SymSolutionRegistry::init(shared_ptr<symbolic::ClosedList> fw_closed,
                               shared_ptr<symbolic::ClosedList> bw_closed,
                               map<int, vector<TransitionRelation>> &trs,
                               shared_ptr<PlanDataBase> plan_data_base,
                               bool single_solution) {
    this->plan_data_base = plan_data_base;
    this->fw_closed = fw_closed;
    this->bw_closed = bw_closed;
    this->trs = trs;
    this->single_solution = single_solution;
}

void SymSolutionRegistry::register_solution(const SymSolutionCut &solution) {
    if (single_solution) {
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
