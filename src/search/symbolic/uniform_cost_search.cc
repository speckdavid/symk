#include "uniform_cost_search.h"

#include "../utils/timer.h"
#include "closed_list.h"
#include "frontier.h"
#include "sym_controller.h"
#include "sym_solution_cut.h"
#include "sym_utils.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;
using utils::g_timer;
using utils::Timer;

namespace symbolic {

UniformCostSearch::UniformCostSearch(SymController *eng,
                                     const SymParamsSearch &params)
    : UnidirectionalSearch(eng, params), parent(nullptr),
      estimationCost(params), estimationZero(params), lastStepCost(true) {}

bool UniformCostSearch::init(std::shared_ptr<SymStateSpaceManager> manager,
                             bool forward,
                             std::shared_ptr<ClosedList> closed_opposite) {
  mgr = manager;
  fw = forward;
  lastStepCost = true;
  last_g_cost = 0;
  assert(mgr);

  BDD init_bdd = fw ? mgr->getInitialState() : mgr->getGoal();
  frontier.init(manager.get(), init_bdd);

  closed->init(mgr.get(), this);
  closed->insert(0, init_bdd);

  if (closed_opposite) {
    perfectHeuristic = closed_opposite;
  } else {
    if (fw) {
      perfectHeuristic =
          make_shared<OppositeFrontierFixed>(mgr->getGoal(), *mgr);
    } else {
      perfectHeuristic =
          make_shared<OppositeFrontierFixed>(mgr->getInitialState(), *mgr);
    }
  }

  prepareBucket();

  engine->setLowerBound(getF());
  engine->setMinG(getG());

  return true;
}

void UniformCostSearch::checkCutOriginal(Bucket &bucket, int g_val) {
  if (p.get_non_stop()) {
    return;
  }

  for (BDD &bucketBDD : bucket) {
    // Get one or all solutions
    if(!p.top_k) {
        auto sol = perfectHeuristic->getCheapestCut(bucketBDD, g_val, fw);
        if (sol.get_f() >= 0) {
            engine->new_solution(sol);
        }
        // Prune everything closed in opposite direction
	bucketBDD *= perfectHeuristic->notClosed();
    } else {
        auto all_sols = perfectHeuristic->getAllCuts(bucketBDD, g_val, fw, engine->getMinG());
        for (auto &sol : all_sols) {
            engine->new_solution(sol);
        }
    }
  }
}

bool UniformCostSearch::provable_no_more_plans() {
    // If we will expand states with new costs
    // We check weather all states in the open list have already
    // been expanded and not part of a goal path
    if (p.top_k) {
        // Here last_g_cost corresponds to the current g-value of the
        // search dir. Thus we consider all smaller
        if (getG() > last_g_cost) {
            BDD no_goal_path_states = !engine->get_states_on_goal_paths();
            no_goal_path_states *= closed->getPartialClosed(last_g_cost - 1);
            if (!open_list.contains_any_state(!no_goal_path_states)) {
               return true; // Search finished
           }
        }
    }
    
    // Important special case: open is empty => terminate
    if (open_list.empty()) {
      return true; // Search finished
    }

    return false;
}

void UniformCostSearch::prepareBucket() {
  if (!frontier.bucketReady()) {

    if (provable_no_more_plans()) {
      engine->setLowerBound(std::numeric_limits<int>::max());
      return;
    }

    open_list.pop(frontier);
    last_g_cost = frontier.g();
    assert(!frontier.empty() || frontier.g() == numeric_limits<int>::max());
    checkCutOriginal(frontier.bucket(), frontier.g());
    
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
    if (!p.top_k) {
        frontier.filter(!closed->notClosed());
    } else{
        frontier.filter(closed->get_closed_at(frontier.g()));
    }
    mgr->filterMutex(frontier.bucket(), fw, initialization());
    removeZero(frontier.bucket());
}

bool UniformCostSearch::stepImage(int maxTime, int maxNodes) {
  Timer sTime;
  Result prepare_res =
      frontier.prepare(maxTime, maxNodes, fw, initialization());
  if (!prepare_res.ok) {
    violated(prepare_res.truncated_reason, prepare_res.time_spent, maxTime,
             maxNodes);

    if (sTime() * 1000.0 > p.maxStepTime) {
      double ratio = (double)p.maxStepTime / ((double)sTime() * 1000.0);
      p.maxStepNodes *= ratio;
    }
    stats.step_time += sTime();
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
        checkCutOriginal(pairCostBDDs.second, cost);

        for (auto &bdd : pairCostBDDs.second) {
          if (!bdd.IsZero()) {
            stepNodes = max(stepNodes, bdd.nodeCount());
            open_list.insert(bdd, cost);
          }
        }
      }
    }
    stats.add_image_time(res_expansion.time_spent);
  } else {
    stats.add_image_time_failed(res_expansion.time_spent);
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

  stats.step_time += sTime();
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

/////////////////////////////////////////////////
////   Auxiliar methods to load/save/print   ////
/////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &os, const UniformCostSearch &exp) {
  os << "exp " << dirname(exp.isFW());
  if (exp.mgr) {
    os << " in " << *(exp.mgr) << " f=" << exp.getF() << flush
       << " g=" << exp.frontier.g() << flush << exp.open_list << flush
       << " est_time: " << exp.nextStepTime() << flush
       << " est_nodes: " << exp.nextStepNodes() << flush;
  }
  return os;
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

BDD UniformCostSearch::getClosedTotal() { return closed->getClosed(); }

BDD UniformCostSearch::notClosed() { return closed->notClosed(); }

std::ostream &operator<<(std::ostream &os, const TruncatedReason &reason) {
  switch (reason) {
  case TruncatedReason::FILTER_MUTEX:
    return os << "filter_mutex";
  case TruncatedReason::MERGE_BUCKET:
    return os << "merge";
  case TruncatedReason::MERGE_BUCKET_COST:
    return os << "merge_cost";
  case TruncatedReason::IMAGE_ZERO:
    return os << "0-image";
  case TruncatedReason::IMAGE_COST:
    return os << "cost-image";
  default:
    cerr << "UniformCostSearch truncated by unkown reason" << endl;
    utils::exit_with(utils::ExitCode::SEARCH_UNSUPPORTED);
  }
}
} // namespace symbolic
