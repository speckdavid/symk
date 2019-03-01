#include "closed_list.h"

#include "sym_solution.h"
#include "sym_state_space_manager.h"
#include "sym_utils.h"

#include "../utils/timer.h"
#include "debug_macros.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <cassert>

using namespace std;

namespace symbolic {

ClosedList::ClosedList(int num_target_plans)
    : mgr(nullptr),
      num_found_plans(0),
      num_target_plans(num_target_plans),
      relevant_task(*tasks::g_root_task) {}

void ClosedList::init(SymStateSpaceManager *manager,
                      UnidirectionalSearch *search) {
  mgr = manager;
  my_search = search;
  set<int>().swap(h_values);
  map<int, Bdd>().swap(closedUpTo);
  map<int, vector<Bdd>>().swap(zeroCostClosed);
  map<int, Bdd>().swap(closed);
  closedTotal = mgr->zeroBDD();
  hNotClosed = 0;
  fNotClosed = 0;
}

void ClosedList::init(SymStateSpaceManager *manager,
                      UnidirectionalSearch *search, const ClosedList &other) {
  mgr = manager;
  my_search = search;
  set<int>().swap(h_values);
  map<int, Bdd>().swap(closedUpTo);
  map<int, vector<Bdd>>().swap(zeroCostClosed);
  map<int, Bdd>().swap(closed);
  closedTotal = mgr->zeroBDD();
  hNotClosed = 0;
  fNotClosed = 0;

  closedTotal = other.closedTotal;
  closed[0] = closedTotal;
  newHValue(0);
}

void ClosedList::newHValue(int h_value) { h_values.insert(h_value); }

void ClosedList::insert(int h, const Bdd &S) {
  DEBUG_MSG(cout << "Inserting on closed "
                 << "g=" << h << ": " << S.nodeCount() << " nodes and "
                 << endl;);

#ifdef DEBUG_GST
  gst_plan.checkClose(S, h, exploration);
#endif

  if (closed.count(h)) {
    assert(h_values.count(h));
    closed[h] += S;
  } else {
    closed[h] = S;
    newHValue(h);
  }

  if (mgr->hasTransitions0()) {
    zeroCostClosed[h].push_back(S);
  }
  closedTotal += S;

  // Introduce in closedUpTo
  auto c = closedUpTo.lower_bound(h);
  while (c != std::end(closedUpTo)) {
    c->second += S;
    c++;
  }
}

void ClosedList::setHNotClosed(int newHNotClosed) {
  if (newHNotClosed > hNotClosed) {
    hNotClosed = newHNotClosed;
    newHValue(newHNotClosed);  // Add newHNotClosed to list of h values (and
                               // those of parents)
  }
}

void ClosedList::setFNotClosed(int f) {
  if (f > fNotClosed) {
    fNotClosed = f;
  }
}

void ClosedList::extract_path(const Bdd &c, int h, bool fw,
                              vector<OperatorID> &path) const {
  extract_multiply_paths(c, h, fw, vector<OperatorID>());
  std::cout << "#Plans: " << paths.size() << std::endl;
  path = paths.back();
  /*const map<int, vector<TransitionRelation>> &trs = mgr->getIndividualTRs();
  Bdd cut = c;
  while (h > 0) {
    // Extracting cost-actions
    bool found = false;
    for (auto key : trs) {  // TODO: maybe is best to use the inverse order
      if (found) break;
      int newH = h - key.first;
      if (key.first == 0 || closed.count(newH) == 0) continue;
      for (TransitionRelation &tr : key.second) {
        Bdd succ;
        if (fw) {
          succ = tr.preimage(cut);
        } else {
          succ = tr.image(cut);
        }
        Bdd intersection = succ * closed.at(newH);
        if (!intersection.IsZero()) {
          h = newH;
          cut = succ;
          path.push_back(*(tr.getOpsIds().begin()));
          found = true;
          break;
        }
      }
    }
    if (!found) {
      cerr << "Error: Solution reconstruction failed: " << endl;
      utils::exit_with(utils::ExitCode::SEARCH_CRITICAL_ERROR);
    }
  }

  DEBUG_MSG(cout << "Sym closed extracted path" << endl;);*/
}

void ClosedList::extract_multiply_paths(const Bdd &c, int h, bool fw,
                                        vector<OperatorID> path) const {
  const map<int, vector<TransitionRelation>> &trs = mgr->getIndividualTRs();
  Bdd cut = c;
  for (auto key : trs) {
    int newH = h - key.first;
    if (newH < 0) {
      continue;
    }
    for (TransitionRelation &tr : key.second) {
      Bdd succ;
      if (fw) {
        succ = tr.preimage(cut);
      } else {
        succ = tr.image(cut);
      }
      Bdd intersection = succ * closed.at(newH);
      if (!intersection.IsZero()) {
        vector<OperatorID> new_path = path;
        new_path.push_back(*(tr.getOpsIds().begin()));
        if (newH == 0) {
          paths.push_back(new_path);
          num_found_plans++;
          if (num_found_plans >= num_target_plans) {
            for (auto &plan : paths) {
              plan_mgr.save_plan(plan, relevant_task, true);
            }
            exit(0);
          }
          // std::cout << paths.size() << std::endl;
        } else {
          extract_multiply_paths(intersection, newH, fw, new_path);
        }
      }
    }
  }
}

SymSolution ClosedList::checkCut(UnidirectionalSearch *search,
                                 const Bdd &states, int g, bool fw) const {
  Bdd cut_candidate = states * closedTotal;
  if (cut_candidate.IsZero()) {
    return SymSolution();  // No solution yet :(
  }

  for (const auto &closedH : closed) {
    int h = closedH.first;

    DEBUG_MSG(cout << "Check cut of g=" << g << " with h=" << h << endl;);
    Bdd cut = closedH.second * cut_candidate;
    if (!cut.IsZero()) {
      if (fw)  // Solution reconstruction will fail
        return SymSolution(search, my_search, g, h, cut);
      else
        return SymSolution(my_search, search, h, g, cut);
    }
  }

  cerr << "Error: Cut with closedTotal but not found on closed" << endl;
  utils::exit_with(utils::ExitCode::SEARCH_CRITICAL_ERROR);
}

void ClosedList::statistics() const {
  // cout << "h (eval " << num_calls_eval << ", not_closed" <<
  // time_eval_states
  // << "s, closed " << time_closed_states
  //   << "s, pruned " << time_pruned_states << "s, some " << time_prune_some
  //   << "s, all " << time_prune_all  << ", children " <<
  //   time_prune_some_children << "s)";
}

const std::set<int> &ClosedList::getHValues() {
  assert(h_values.count(hNotClosed));

  return h_values;
}
}  // namespace symbolic
