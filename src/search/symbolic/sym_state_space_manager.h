#ifndef SYMBOLIC_SYM_STATE_SPACE_MANAGER_H
#define SYMBOLIC_SYM_STATE_SPACE_MANAGER_H

#include "../plan_manager.h"
#include "../tasks/root_task.h"
#include "../utils/system.h"
#include "sym_bucket.h"
#include "sym_enums.h"
#include "sym_utils.h"
#include "sym_variables.h"

#include <algorithm>
#include <cassert>
#include <map>
#include <memory>
#include <set>
#include <vector>

namespace options {
class OptionParser;
class Options;
}  // namespace options

namespace symbolic {
class SymVariables;
class TransitionRelation;

/*
 * All the methods may throw exceptions in case the time or nodes are exceeded.
 *
 */
class SymParamsMgr {
 public:
  int num_plans;
  // Parameters to generate the TRs
  int max_tr_size, max_tr_time;

  // Parameters to generate the mutex BDDs
  MutexType mutex_type;
  int max_mutex_size, max_mutex_time;

  // Time and memory bounds for auxiliary operations
  int max_aux_nodes, max_aux_time;

  SymParamsMgr();
  SymParamsMgr(const options::Options &opts);
  static void add_options_to_parser(options::OptionParser &parser);
  void print_options() const;
};

class SymStateSpaceManager {
  void zero_preimage(const Bdd &bdd, std::vector<Bdd> &res, int maxNodes) const;
  void cost_preimage(const Bdd &bdd, std::map<int, std::vector<Bdd>> &res,
                     int maxNodes) const;
  void zero_image(const Bdd &bdd, std::vector<Bdd> &res, int maxNodes) const;
  void cost_image(const Bdd &bdd, std::map<int, std::vector<Bdd>> &res,
                  int maxNodes) const;
  void preimage_parallel(const Bdd &bdd, int nodeLimit,
                         const std::vector<TransitionRelation> &trs,
                         std::vector<Bdd> &res, int i, int k) const;
  void image_parallel(const Bdd &bdd, int nodeLimit,
                      const std::vector<TransitionRelation> &trs,
                      std::vector<Bdd> &res, int i, int k) const;

 protected:
  mutable std::shared_ptr<PlanManager> plan_mgr;
  TaskProxy relevant_task;

  SymVariables *vars;
  const SymParamsMgr p;

  // If the variable is fully/partially/not considered in the abstraction
  std::set<int> relevant_vars;

  Bdd initialState;  // initial state
  Bdd goal;  // bdd representing the true (i.e. not simplified) goal-state

  std::map<int, std::vector<TransitionRelation>> transitions;  // TRs
  int min_transition_cost;  // minimum cost of non-zero cost transitions
  bool hasTR0;              // If there is transitions with cost 0

  // BDD representation of valid states (wrt mutex) for fw and bw search
  std::vector<Bdd> notMutexBDDsFw, notMutexBDDsBw;

  // Dead ends for fw and bw searches. They are always removed in
  // filter_mutex (it does not matter which mutex_type we are using).
  std::vector<Bdd> notDeadEndFw, notDeadEndBw;

  Bdd getRelVarsCubePre() const { return vars->getCubePre(relevant_vars); }

  Bdd getRelVarsCubeEff() const { return vars->getCubeEff(relevant_vars); }

  virtual std::string tag() const = 0;

  void init_transitions(
      const std::map<int, std::vector<TransitionRelation>> &(indTRs));

 public:
  SymStateSpaceManager(SymVariables *v, const SymParamsMgr &params,
                       std::shared_ptr<PlanManager> plan_mgr,
                       const std::set<int> &relevant_vars_ = std::set<int>());

  virtual ~SymStateSpaceManager() {}

  void save_plan(std::vector<OperatorID> &plan, bool reverse) const {
    if (reverse) {
      std::reverse(plan.begin(), plan.end());
    }
    plan_mgr->save_plan(plan, relevant_task, true);
    if (get_num_plans() % 1000 == 0) {
      std::cout << " => " << get_num_plans() << " found plan!" << std::endl;
    }
  }
  int get_target_num_plans() const { return p.num_plans; }
  int get_num_plans() const { return plan_mgr->get_num_of_genertated_plans(); }
  bool found_enough_plans() const {
    return get_target_num_plans() <= get_num_plans();
  }

  bool isAbstracted() const { return !isOriginal(); }

  bool isOriginal() const {
    return static_cast<int>(relevant_vars.size()) ==
           tasks::g_root_task->get_num_variables();
  }

  void filterMutex(Bucket &bucket, bool fw, bool initialization);
  void mergeBucket(Bucket &bucket) const;
  void mergeBucketAnd(Bucket &bucket) const;

  void shrinkBucket(Bucket &bucket, int maxNodes);

  SymVariables *getVars() const { return vars; }

  const SymParamsMgr getParams() const { return p; }

  const Bdd &getGoal() { return goal; }

  const Bdd &getInitialState() { return initialState; }

  Bdd getBDD(int variable, int value) const {
    return vars->preBDD(variable, value);
  }

  Bdd zeroBDD() const { return vars->zeroBDD(); }

  Bdd oneBDD() const { return vars->oneBDD(); }

  const std::vector<Bdd> &getNotMutexBDDs(bool fw) const {
    return fw ? notMutexBDDsFw : notMutexBDDsBw;
  }

  bool mergeBucket(Bucket &bucket, int maxTime, int maxNodes) const {
    auto mergeBDDs = [](Bdd bdd, Bdd bdd2, int maxNodes) {
      return bdd.Or(bdd2, maxNodes);
    };
    merge(vars, bucket, mergeBDDs, maxTime, maxNodes);
    removeZero(bucket);  // Be sure that we do not contain only the zero BDD

    return bucket.size() <= 1;
  }

  bool mergeBucketAnd(Bucket &bucket, int maxTime, int maxNodes) const {
    auto mergeBDDs = [](Bdd bdd, Bdd bdd2, int maxNodes) {
      return bdd.And(bdd2, maxNodes);
    };
    merge(vars, bucket, mergeBDDs, maxTime, maxNodes);
    removeZero(bucket);  // Be sure that we do not contain only the zero BDD

    return bucket.size() <= 1;
  }

  void dumpMutexBDDs(bool fw) const;

  // Methods that require of TRs initialized
  int getMinTransitionCost() const {
    assert(!transitions.empty());
    return min_transition_cost;
  }

  int getAbsoluteMinTransitionCost() const {
    assert(!transitions.empty());
    if (hasTR0) return 0;
    return min_transition_cost;
  }

  bool hasTransitions0() const {
    assert(!transitions.empty());
    return hasTR0;
  }

  void zero_image(bool fw, const Bdd &bdd, std::vector<Bdd> &res,
                  int maxNodes) {
    if (fw)
      zero_image(bdd, res, maxNodes);
    else
      zero_preimage(bdd, res, maxNodes);
  }

  void cost_image(bool fw, const Bdd &bdd, std::map<int, std::vector<Bdd>> &res,
                  int maxNodes) {
    if (fw) {
      cost_image(bdd, res, maxNodes);
    } else {
      cost_preimage(bdd, res, maxNodes);
    }
  }

  Bdd filter_mutex(const Bdd &bdd, bool fw, int maxNodes, bool initialization);

  int filterMutexBucket(std::vector<Bdd> &bucket, bool fw, bool initialization,
                        int maxTime, int maxNodes);

  void setTimeLimit(int maxTime) { vars->setTimeLimit(maxTime); }

  void unsetTimeLimit() { vars->unsetTimeLimit(); }

  friend std::ostream &operator<<(std::ostream &os,
                                  const SymStateSpaceManager &state_space);

  virtual void print(std::ostream &os, bool /*fullInfo*/) const {
    os << tag() << " (" << relevant_vars.size() << ")";
  }

  // For plan solution reconstruction. Only avaialble in original state space
  virtual const std::map<int, std::vector<TransitionRelation>>
      &getIndividualTRs() const {
    std::cerr << "Error: trying to get individual TRs from an invalid state "
                 "space type"
              << std::endl;
    utils::exit_with(utils::ExitCode::SEARCH_CRITICAL_ERROR);
  }
};

}  // namespace symbolic
#endif
