#include "uniform_cost_search.h"

#include "../closed_list.h"
#include "../frontier.h"
#include "../plan_reconstruction/sym_solution_cut.h"
#include "../search_engines/symbolic_search.h"
#include "../sym_utils.h"
#include "../utils/timer.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

namespace symbolic {
UniformCostSearch::UniformCostSearch(SymbolicSearch *eng,
                                     const SymParamsSearch &params)
    : SymSearch(eng, params), fw(true), closed(make_shared<ClosedList>()),
      estimationCost(params), estimationZero(params), lastStepCost(true) {}

bool UniformCostSearch::init(shared_ptr<SymStateSpaceManager> manager,
                             bool forward, UniformCostSearch *opposite_search) {
    mgr = manager;
    fw = forward;
    lastStepCost = true;
    last_g_cost = 0;
    assert(mgr);

    BDD init_bdd = fw ? mgr->getInitialState() : mgr->getGoal();
    frontier.init(manager.get(), init_bdd);

    closed->init(mgr.get());
    closed->insert(0, init_bdd);

    if (opposite_search) {
        perfectHeuristic = opposite_search->getClosedShared();
    } else {
        perfectHeuristic = make_shared<ClosedList>();
        perfectHeuristic->init(mgr.get());
        if (fw) {
            perfectHeuristic->insert(0, mgr->getGoal());
        } else {
            perfectHeuristic->insert(0, mgr->getInitialState());
        }
    }

    prepareBucket();

    engine->setLowerBound(getF());
    engine->setMinG(getG());

    return true;
}

void UniformCostSearch::checkFrontierCut(Bucket &bucket, int g) {
    if (p.get_non_stop()) {
        return;
    }

    for (BDD &bucketBDD : bucket) {
        auto sol = perfectHeuristic->getCheapestCut(bucketBDD, g, fw);
        if (sol.get_f() >= 0) {
            engine->new_solution(sol);
        }
        // Prune everything closed in opposite direction
        bucketBDD *= perfectHeuristic->notClosed();
    }
}

bool UniformCostSearch::provable_no_more_plans() {return open_list.empty();}

void UniformCostSearch::prepareBucket() {
    if (!frontier.bucketReady()) {
        if (provable_no_more_plans()) {
            engine->setLowerBound(numeric_limits<int>::max());
            return;
        }

        open_list.pop(frontier);
        last_g_cost = frontier.g();
        assert(!frontier.empty() || frontier.g() == numeric_limits<int>::max());
        checkFrontierCut(frontier.bucket(), frontier.g());

        filterFrontier();

        // Close and move to reopen

        if (!lastStepCost || frontier.g() != 0) {
            // Avoid closing init twice
            for (const BDD &states : frontier.bucket()) {
                closed->insert(frontier.g(), states);
            }
        }
        engine->setLowerBound(getF());
        engine->setMinG(getG());

        computeEstimation(true);
    }

    if (engine->solved()) {
        return; // If it has been solved, return
    }

    initialization();

    int maxTime = p.getAllotedTime(nextStepTime());
    int maxNodes = p.getAllotedNodes(nextStepNodesResult());

    Result res = frontier.prepare(maxTime, maxNodes, fw, initialization());
    if (!res.ok) {
        violated(res.truncated_reason, res.time_spent, maxTime, maxNodes);
    }
}

// Here we filter states: remove closed states and mutex states
// This procedure is delayed in comparision to explicit search
// Idea: no need to "change" BDDs until we actually process them
void UniformCostSearch::filterFrontier() {
    frontier.filter(!closed->notClosed());
    mgr->filterMutex(frontier.bucket(), fw, initialization());
    removeZero(frontier.bucket());
}

bool UniformCostSearch::stepImage(int maxTime, int maxNodes) {
    utils::Timer sTime;
    Result prepare_res =
        frontier.prepare(maxTime, maxNodes, fw, initialization());
    if (!prepare_res.ok) {
        violated(prepare_res.truncated_reason, prepare_res.time_spent, maxTime,
                 maxNodes);

        if (sTime() * 1000.0 > p.maxStepTime) {
            double ratio = (double)p.maxStepTime / ((double)sTime() * 1000.0);
            p.maxStepNodes *= ratio;
        }
        return false;
    }

    if (engine->solved()) {
        return true; // Skip image if we are done
    }

    int stepNodes = frontier.nodes();
    ResultExpansion res_expansion = frontier.expand(maxTime, maxNodes, fw);

    if (res_expansion.ok) {
        lastStepCost = false; // Must be set to false before calling checkCut
        // Process Simg, removing duplicates and computing h. Store in Sfilter and
        // reopen. Include new states in the open list
        for (auto &resImage : res_expansion.buckets) {
            for (auto &pairCostBDDs : resImage) {
                int cost = frontier.g() +
                    pairCostBDDs.first; // Include states of the given cost
                mgr->mergeBucket(pairCostBDDs.second);

                // Check for cut (remove those states)
                checkFrontierCut(pairCostBDDs.second, cost);

                for (auto &bdd : pairCostBDDs.second) {
                    if (!bdd.IsZero()) {
                        stepNodes = max(stepNodes, bdd.nodeCount());
                        open_list.insert(bdd, cost);
                    }
                }
            }
        }
    }

    if (!res_expansion.step_zero) {
        estimationCost.stepTaken(1000 * res_expansion.time_spent, stepNodes);
    } else {
        estimationZero.stepTaken(1000 * res_expansion.time_spent, stepNodes);
    }

    // Try to prepare next Bucket
    computeEstimation(true);

    // We prepare the next bucket before checking time in doing
    // the step because we consider preparing the bucket as a
    // part of the step.
    prepareBucket();

    if (sTime() * 1000.0 > p.maxStepTime) {
        double ratio = (double)p.maxStepTime / ((double)sTime() * 1000.0);
        p.maxStepNodes = stepNodes * ratio;
    } else if (!res_expansion.ok) {
        // In case maxAllotedNodes were exceeded we reduce the maximum
        // frontier size by 3/4.  TODO: make this a parameter
        p.maxStepNodes = stepNodes * 0.75;
    }

    return res_expansion.ok;
}

bool UniformCostSearch::isSearchableWithNodes(int maxNodes) const {
    return frontier.expansionReady() && nextStepNodes() <= maxNodes;
}

void UniformCostSearch::computeEstimation(bool prepare) {
    if (prepare) {
        prepareBucket(/*p.max_pop_time, p.max_pop_nodes, true*/);
    }

    if (frontier.expansionReady()) {
        // Succeded, the estimation will be only in image
        if (frontier.nextStepZero()) {
            estimationZero.nextStep(frontier.nodes());
        } else {
            estimationCost.nextStep(frontier.nodes());
        }
    } else {
        if (mgr->hasTransitions0()) {
            estimationZero.nextStep(frontier.nodes());
        } else {
            estimationCost.nextStep(frontier.nodes());
        }
    }
}

long UniformCostSearch::nextStepTime() const {
    long estimation = 0;

    if (mgr->hasTransitions0() &&
        (!frontier.expansionReady() || frontier.nextStepZero())) {
        estimation += estimationZero.time();
    } else {
        estimation += estimationCost.time();
    }
    return estimation;
}

long UniformCostSearch::nextStepNodes() const {
    if (mgr->hasTransitions0() &&
        (!frontier.expansionReady() || frontier.nextStepZero())) {
        return estimationZero.nextNodes();
    } else {
        return estimationCost.nextNodes();
    }
}

long UniformCostSearch::nextStepNodesResult() const {
    long estimation = 0;

    if (mgr->hasTransitions0() &&
        (!frontier.expansionReady() || frontier.nextStepZero())) {
        estimation = max(estimation, estimationZero.nodes());
    } else {
        estimation = max(estimation, estimationCost.nodes());
    }
    return estimation;
}

void UniformCostSearch::violated(TruncatedReason /*reason*/,
                                 double ellapsed_seconds, int maxTime,
                                 int maxNodes) {
    int time = 1 + ellapsed_seconds * 1000;

    if (mgr->hasTransitions0() &&
        (!frontier.expansionReady() || frontier.nextStepZero())) {
        estimationZero.violated(time, maxTime, maxNodes);
    } else {
        estimationCost.violated(time, maxTime, maxNodes);
    }
}
} // namespace symbolic
