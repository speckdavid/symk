#include "simple_sym_solution_registry.h"

#include "reconstruction_node.h"

#include "../searches/uniform_cost_search.h"

using namespace std;


// TODOs: 1. We want to pass all sym_cuts with the same cost we have at once and add them to 
// the queue. And especially we want to do that fist when we have all to be able to sort them by
// lenght.
// However: this might detoriate our performance a bit
// 2. We might can ignore symetric sym_cuts: e.g, {g=11,h=0} and {g=0,h=11}...


namespace symbolic {
void SimpleSymSolutionRegistry::add_plan(const Plan &plan) const {
    plan_data_base->add_plan(plan);
    assert(!plan_data_base->has_zero_cost_loop(plan));
}

void SimpleSymSolutionRegistry::reconstruct_plans(
    const SymSolutionCut &sym_cut) {
    assert(fw_search || sym_cut.get_g() == 0);
    assert(bw_search || sym_cut.get_h() == 0);

    ReconstructionNode cur_node(sym_cut.get_g(), sym_cut.get_h(),
                                sym_cut.get_cut(), fw_search != nullptr, 0);
    queue.push(cur_node);

    // In the bidirectional case we might can directly swap the direction
    if (swap_to_bwd_phase(cur_node)) {
        ReconstructionNode bw_node = cur_node;
        bw_node.set_fwd_phase(false);
        queue.push(bw_node);
    }

    // While queue is not empty
    while (!queue.empty()) {
        cur_node = queue.top();
        queue.pop();
        // utils::g_log << cur_node << endl;

        // Check if we have found a solution with this cut
        if (cur_node.is_solution()) {
            Plan cur_plan;
            cur_node.get_plan(cur_plan);
            add_plan(cur_plan);

            // Plan data base tells us if we need to continue
            // We can stop early if we, e.g., have found enough plans
            if (!plan_data_base->reconstruct_solutions(sym_cut)) {
                return;
            }
            // continue;
        }
        expand_cost_actions(cur_node);
    }
}

void SimpleSymSolutionRegistry::expand_cost_actions(const ReconstructionNode &node) {
    bool fwd = node.is_fwd_phase();
    int cur_cost;
    shared_ptr<ClosedList> cur_closed_list;

    if (fwd) {
        cur_cost = node.get_g();
        cur_closed_list = fw_search->getClosedShared();
    } else {
        cur_cost = node.get_h();
        cur_closed_list = bw_search->getClosedShared();
    }

    for (auto key : trs) {
        int new_cost = cur_cost - key.first;

        // 1. zero cost action are handled differently
        // 2. new cost can not be negative
        if (key.first == 0 || new_cost < 0) {
            continue;
        }

        for (TransitionRelation &tr : key.second) {
            BDD succ = fwd ? tr.preimage(node.get_states()) : tr.image(node.get_states());
            BDD intersection = succ * cur_closed_list->get_closed_at(new_cost);
            if (intersection.IsZero()) {
                continue;
            }

            auto op_id = *(tr.getOpsIds().begin());
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
            } else {
                queue.push(new_node);
            }
        }
    }
}

bool SimpleSymSolutionRegistry::swap_to_bwd_phase(const ReconstructionNode &node) const {
    return bw_search
           && node.is_fwd_phase()
           && node.get_g() == 0
           && !(node.get_states() * fw_search->getClosedShared()->get_start_states()).IsZero();
}
}
