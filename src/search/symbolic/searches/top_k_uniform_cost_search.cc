#include "top_k_uniform_cost_search.h"
#include "../closed_list.h"
#include "../search_engines/symbolic_search.h"

namespace symbolic {
bool TopkUniformCostSearch::provable_no_more_plans() {
    // If we will expand states with new costs
    // We check weather all states in the open list have already
    // been expanded and not part of a goal path
    // Here last_g_cost corresponds to the current g-value of the
    // search dir. Thus we consider all smaller
    if (getG() > last_g_cost) {
        BDD no_goal_path_states = !engine->get_states_on_goal_paths();
        no_goal_path_states *= closed->getPartialClosed(last_g_cost - 1);
        if (!open_list.contains_any_state(!no_goal_path_states)) {
            return true; // Search finished
        }
    }

    // Important 'special' case: open is empty => terminate
    return open_list.empty();
}

void TopkUniformCostSearch::checkFrontierCut(Bucket &bucket, int g) {
    for (BDD &bucketBDD : bucket) {
        auto all_sols =
            perfectHeuristic->getAllCuts(bucketBDD, g, fw, engine->getMinG());
        for (auto &sol : all_sols) {
            engine->new_solution(sol);
        }
    }
}

void TopkUniformCostSearch::filterFrontier() {
    frontier.filter(closed->get_closed_at(frontier.g()));
    mgr->filterMutex(frontier.bucket(), fw, initialization());
    removeZero(frontier.bucket());
}
} // namespace symbolic
