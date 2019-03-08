#include "sym_state_space_manager.h"

#include "debug_macros.h"
#include "sym_enums.h"
#include <algorithm>
#include <limits>
#include <queue>

#include "../abstract_task.h"
#include "../mutex_group.h"
#include "../options/option_parser.h"
#include "../options/options.h"
#include "../task_proxy.h"
#include "../task_utils/task_properties.h"
#include "../utils/timer.h"
#include "sym_utils.h"

using namespace std;

namespace symbolic {

SymStateSpaceManager::SymStateSpaceManager(SymVariables *v,
                                           const SymParamsMgr &params,
                                           const set<int> &relevant_vars_)
    : vars(v), p(params), relevant_vars(relevant_vars_),
      initialState(v->zeroBDD()), goal(v->zeroBDD()), min_transition_cost(0),
      hasTR0(false) {

  if (relevant_vars.empty()) {
    for (int i = 0; i < tasks::g_root_task->get_num_variables(); ++i) {
      relevant_vars.insert(i);
    }
  }
}

void SymStateSpaceManager::dumpMutexBDDs(bool fw) const {
  if (fw) {
    cout << "Mutex BDD FW Size(" << p.max_mutex_size << "):";
    for (const auto &bdd : notMutexBDDsFw) {
      cout << " " << bdd.nodeCount();
    }
    cout << endl;
  } else {
    cout << "Mutex BDD BW Size(" << p.max_mutex_size << "):";
    for (const auto &bdd : notMutexBDDsBw) {
      cout << " " << bdd.nodeCount();
    }
    cout << endl;
  }
}

void SymStateSpaceManager::zero_preimage(const Bdd &bdd, vector<Bdd> &res,
                                         int nodeLimit) const {
#ifdef DDLIBSYLVAN
  if (Bdd::is_parallel_search()) {
    // Reserve the space
    vector<Bdd> cur_res(transitions.at(0).size(), Bdd::BddOne());
    // Compute in parallel
    preimage_parallel(bdd, nodeLimit, transitions.at(0), cur_res, 0,
                      transitions.at(0).size());
    // Append to result
    res.insert(std::end(res), std::begin(cur_res), std::end(cur_res));
  } else {
    for (const auto &tr : transitions.at(0)) {
      res.push_back(tr.preimage(bdd, nodeLimit));
    }
  }
#else
  for (const auto &tr : transitions.at(0)) {
    res.push_back(tr.preimage(bdd, nodeLimit));
  }
#endif
}

void SymStateSpaceManager::zero_image(const Bdd &bdd, vector<Bdd> &res,
                                      int nodeLimit) const {
#ifdef DDLIBSYLVAN
  if (Bdd::is_parallel_search()) {
    // Reserve the space
    vector<Bdd> cur_res(transitions.at(0).size(), Bdd::BddOne());
    // Compute in parallel
    image_parallel(bdd, nodeLimit, transitions.at(0), cur_res, 0,
                   transitions.at(0).size());
    // Append to result
    res.insert(std::end(res), std::begin(cur_res), std::end(cur_res));
  } else {
    for (const auto &tr : transitions.at(0)) {
      res.push_back(tr.image(bdd, nodeLimit));
    }
  }
#else
  for (const auto &tr : transitions.at(0)) {
    res.push_back(tr.image(bdd, nodeLimit));
  }
#endif
}

void SymStateSpaceManager::cost_preimage(const Bdd &bdd,
                                         map<int, vector<Bdd>> &res,
                                         int nodeLimit) const {

#ifdef DDLIBSYLVAN
  if (Bdd::is_parallel_search()) {
    for (auto trs : transitions) {
      int cost = trs.first;
      // Reserve the space
      vector<Bdd> cur_res(transitions.at(cost).size(), Bdd::BddOne());
      // Compute in parallel
      preimage_parallel(bdd, nodeLimit, transitions.at(cost), cur_res, 0,
                        transitions.at(cost).size());
      // Append to result
      res[cost].insert(std::end(res[cost]), std::begin(cur_res),
                       std::end(cur_res));
    }
  } else {
    for (auto trs : transitions) {
      int cost = trs.first;
      if (cost == 0)
        continue;
      for (size_t i = 0; i < trs.second.size(); i++) {
        Bdd result = trs.second[i].preimage(bdd, nodeLimit);
        res[cost].push_back(result);
      }
    }
  }
#else
  for (auto trs : transitions) {
    int cost = trs.first;
    if (cost == 0)
      continue;
    for (size_t i = 0; i < trs.second.size(); i++) {
      Bdd result = trs.second[i].preimage(bdd, nodeLimit);
      res[cost].push_back(result);
    }
  }
#endif
}

void SymStateSpaceManager::cost_image(const Bdd &bdd,
                                      map<int, vector<Bdd>> &res,
                                      int nodeLimit) const {
#ifdef DDLIBSYLVAN
  if (Bdd::is_parallel_search()) {
    for (auto trs : transitions) {
      int cost = trs.first;
      // Reserve the space
      vector<Bdd> cur_res(transitions.at(cost).size(), Bdd::BddOne());
      // Compute in parallel
      image_parallel(bdd, nodeLimit, transitions.at(cost), cur_res, 0,
                     transitions.at(cost).size());
      // Append to result
      res[cost].insert(std::end(res[cost]), std::begin(cur_res),
                       std::end(cur_res));
    }
  } else {
    for (auto trs : transitions) {
      int cost = trs.first;
      if (cost == 0)
        continue;
      for (size_t i = 0; i < trs.second.size(); i++) {
        Bdd result = trs.second[i].image(bdd, nodeLimit);
        res[cost].push_back(result);
      }
    }
  }
#else
  for (auto trs : transitions) {
    int cost = trs.first;
    if (cost == 0)
      continue;
    for (size_t i = 0; i < trs.second.size(); i++) {
      Bdd result = trs.second[i].image(bdd, nodeLimit);
      res[cost].push_back(result);
    }
  }
#endif
}

void SymStateSpaceManager::preimage_parallel(
    const Bdd &bdd, int nodeLimit, const std::vector<TransitionRelation> &trs,
    std::vector<Bdd> &res, int i, int k) const {
#ifdef DDLIBSYLVAN
  LACE_ME
#endif
  if (k == 1) {
    res[i] = trs[i].preimage(bdd, nodeLimit);
  } else {
    preimage_parallel(bdd, nodeLimit, trs, res, i, k / 2);
    preimage_parallel(bdd, nodeLimit, trs, res, i + (k / 2), k - (k / 2));
  }
}

void SymStateSpaceManager::image_parallel(
    const Bdd &bdd, int nodeLimit, const std::vector<TransitionRelation> &trs,
    std::vector<Bdd> &res, int i, int k) const {
#ifdef DDLIBSYLVAN
  LACE_ME
#endif
  if (k == 1) {
    res[i] = trs[i].image(bdd, nodeLimit);
  } else {
    image_parallel(bdd, nodeLimit, trs, res, i, k / 2);
    image_parallel(bdd, nodeLimit, trs, res, i + (k / 2), k - (k / 2));
  }
}

Bdd SymStateSpaceManager::filter_mutex(const Bdd &bdd, bool fw, int nodeLimit,
                                       bool initialization) {
  Bdd res = bdd;
  const vector<Bdd> &notDeadEndBDDs =
      ((fw || isAbstracted()) ? notDeadEndFw : notDeadEndBw);
  for (const Bdd &notDeadEnd : notDeadEndBDDs) {
    DEBUG_MSG(cout << "Filter: " << res.nodeCount() << " and dead end "
                   << notDeadEnd.nodeCount() << flush;);
    assert(!(notDeadEnd.IsZero()));
    res = res.And(notDeadEnd, nodeLimit);
    DEBUG_MSG(cout << ": " << res.nodeCount() << endl;);
  }

  const vector<Bdd> &notMutexBDDs = (fw ? notMutexBDDsFw : notMutexBDDsBw);

  switch (p.mutex_type) {
  case MutexType::MUTEX_NOT:
    break;
  case MutexType::MUTEX_EDELETION:
    if (initialization) {
      for (const Bdd &notMutexBDD : notMutexBDDs) {
        DEBUG_MSG(cout << res.nodeCount() << " and " << notMutexBDD.nodeCount()
                       << flush;);
        res = res.And(notMutexBDD, nodeLimit);
        DEBUG_MSG(cout << ": " << res.nodeCount() << endl;);
      }
    }
    break;
  case MutexType::MUTEX_AND:
    for (const Bdd &notMutexBDD : notMutexBDDs) {
      DEBUG_MSG(cout << "Filter: " << res.nodeCount() << " and "
                     << notMutexBDD.nodeCount() << flush;);
      res = res.And(notMutexBDD, nodeLimit);
      DEBUG_MSG(cout << ": " << res.nodeCount() << endl;);
    }
    break;
    /*case MutexType::MUTEX_RESTRICT:
        for (const Bdd &notMutexBDD : notMutexBDDs)
            res = res.Restrict(notMutexBDD);
        break;
    case MutexType::MUTEX_NPAND:
        for (const Bdd &notMutexBDD : notMutexBDDs)
            res = res.NPAnd(notMutexBDD);
        break;
    case MutexType::MUTEX_CONSTRAIN:
        for (const Bdd &notMutexBDD : notMutexBDDs)
            res = res.Constrain(notMutexBDD);
        break;
    case MutexType::MUTEX_LICOMP:
        for (const Bdd &notMutexBDD : notMutexBDDs)
            res = res.LICompaction(notMutexBDD);
        break;*/
  }
  return res;
}

int SymStateSpaceManager::filterMutexBucket(vector<Bdd> &bucket, bool fw,
                                            bool initialization, int maxTime,
                                            int maxNodes) {
  int numFiltered = 0;
  setTimeLimit(maxTime);
  try {
    for (size_t i = 0; i < bucket.size(); ++i) {
      DEBUG_MSG(cout << "Filter spurious " << (fw ? "fw" : "bw") << ": "
                     << *this << " from: " << bucket[i].nodeCount()
                     << " maxTime: " << maxTime
                     << " and maxNodes: " << maxNodes;);

      bucket[i] = filter_mutex(bucket[i], fw, maxNodes, initialization);
      DEBUG_MSG(cout << " => " << bucket[i].nodeCount() << endl;);
      numFiltered++;
    }
  } catch (BDDError e) {
    DEBUG_MSG(cout << " truncated." << endl;);
  }
  unsetTimeLimit();

  return numFiltered;
}

void SymStateSpaceManager::filterMutex(Bucket &bucket, bool fw,
                                       bool initialization) {
  filterMutexBucket(bucket, fw, initialization, p.max_aux_time,
                    p.max_aux_nodes);
}

void SymStateSpaceManager::mergeBucket(Bucket &bucket) const {
  mergeBucket(bucket, p.max_aux_time, p.max_aux_nodes);
}

void SymStateSpaceManager::mergeBucketAnd(Bucket &bucket) const {
  mergeBucketAnd(bucket, p.max_aux_time, p.max_aux_nodes);
}

void SymStateSpaceManager::init_transitions(
    const map<int, vector<TransitionRelation>> &(indTRs)) {
  transitions = indTRs; // Copy
  if (transitions.empty()) {
    hasTR0 = false;
    min_transition_cost = 1;
    return;
  }

  for (map<int, vector<TransitionRelation>>::iterator it = transitions.begin();
       it != transitions.end(); ++it) {
    merge(vars, it->second, mergeTR, p.max_tr_time, p.max_tr_size);
  }

  min_transition_cost = transitions.begin()->first;
  if (min_transition_cost == 0) {
    hasTR0 = true;
    if (transitions.size() > 1) {
      min_transition_cost = (transitions.begin()++)->first;
    }
  }
}

SymParamsMgr::SymParamsMgr(const options::Options &opts)
    : num_plans(opts.get<int>("num_plans")),
      max_tr_size(opts.get<int>("max_tr_size")),
      max_tr_time(opts.get<int>("max_tr_time")),
      mutex_type(MutexType(opts.get_enum("mutex_type"))),
      max_mutex_size(opts.get<int>("max_mutex_size")),
      max_mutex_time(opts.get<int>("max_mutex_time")),
      max_aux_nodes(opts.get<int>("max_aux_nodes")),
      max_aux_time(opts.get<int>("max_aux_time")) {
  // Don't use edeletion with conditional effects
  TaskProxy task_proxy(*tasks::g_root_task);
  if (mutex_type == MutexType::MUTEX_EDELETION &&
      task_properties::has_conditional_effects(task_proxy)) {
    cout << "Mutex type changed to mutex_and because the domain has "
            "conditional effects"
         << endl;
    mutex_type = MutexType::MUTEX_AND;
  }
}

SymParamsMgr::SymParamsMgr()
    : num_plans(1), max_tr_size(100000), max_tr_time(60000),
      mutex_type(MutexType::MUTEX_EDELETION), max_mutex_size(100000),
      max_mutex_time(60000), max_aux_nodes(1000000), max_aux_time(2000) {
  // Don't use edeletion with conditional effects
  TaskProxy task_proxy(*tasks::g_root_task);
  if (mutex_type == MutexType::MUTEX_EDELETION &&
      task_properties::has_conditional_effects(task_proxy)) {
    cout << "Mutex type changed to mutex_and because the domain has "
            "conditional effects"
         << endl;
    mutex_type = MutexType::MUTEX_AND;
  }
}

void SymParamsMgr::print_options() const {
  cout << "TR(time=" << max_tr_time << ", nodes=" << max_tr_size << ")" << endl;
  cout << "Mutex(time=" << max_mutex_time << ", nodes=" << max_mutex_size
       << ", type=" << mutex_type << ")" << endl;
  cout << "Aux(time=" << max_aux_time << ", nodes=" << max_aux_nodes << ")"
       << endl;
}

void SymParamsMgr::add_options_to_parser(options::OptionParser &parser) {
  parser.add_option<int>("num_plans", "number of plans", "1");

  parser.add_option<int>("max_tr_size", "maximum size of TR BDDs", "100000");

  parser.add_option<int>("max_tr_time", "maximum time (ms) to generate TR BDDs",
                         "60000");

  parser.add_enum_option("mutex_type", MutexTypeValues, "mutex type",
                         "MUTEX_EDELETION");

  parser.add_option<int>("max_mutex_size", "maximum size of mutex BDDs",
                         "100000");

  parser.add_option<int>("max_mutex_time",
                         "maximum time (ms) to generate mutex BDDs", "60000");

  parser.add_option<int>("max_aux_nodes", "maximum size in pop operations",
                         "1000000");
  parser.add_option<int>("max_aux_time", "maximum time (ms) in pop operations",
                         "2000");
}

std::ostream &operator<<(std::ostream &os, const SymStateSpaceManager &abs) {
  abs.print(os, false);
  return os;
}
} // namespace symbolic