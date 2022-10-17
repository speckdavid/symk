#include "simple_sym_solution_registry.h"

#include "reconstruction_node.h"

#include "../searches/uniform_cost_search.h"

using namespace std;

namespace symbolic {
void SimpleSymSolutionRegistry::add_plan(const Plan &plan) const {
    plan_data_base->add_plan(plan);
    assert(!plan_data_base->has_zero_cost_loop(plan));
}

void SimpleSymSolutionRegistry::reconstruct_plans(
    const SymSolutionCut &sym_cut) {
    assert(fw_search || sym_cut.get_g() == 0);
    assert(bw_search || sym_cut.get_h() == 0);

    int cur_f = sym_cut.get_f();
    ReconstructionNode cur_node(sym_cut.get_g(), sym_cut.get_h(), sym_cut.get_cut(),
                                fw_search != nullptr);
    queue.push(cur_node.get_f(), cur_node);

    // While queue is not empty
    while (!queue.empty()) {
        tie(cur_f, cur_node) = queue.pop();
        utils::g_log << cur_f << ": " << cur_node << endl;

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
    exit(0);
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
    cout << "here" << endl;

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
            ReconstructionNode new_node(-1, -1, intersection, fwd);
            if (fwd) {
                // TODO check for init state!
                new_node.set_g(new_cost);
                new_node.set_h(node.get_h());
                new_node.set_predecessor(make_shared<ReconstructionNode>(node), op_id);
            } else {
                new_node.set_g(node.get_g());
                new_node.set_h(new_cost);
                new_node.set_successor(make_shared<ReconstructionNode>(node), op_id);
            }
            queue.push(new_node.get_f(), new_node);
        }
    }
}
}
