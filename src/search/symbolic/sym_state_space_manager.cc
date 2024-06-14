#include "sym_state_space_manager.h"

#include "sym_enums.h"
#include "sym_utils.h"

#include "../abstract_task.h"
#include "../mutex_group.h"
#include "../task_proxy.h"

#include "../tasks/effect_aggregated_task.h"
#include "../tasks/sdac_task.h"
#include "../task_utils/task_properties.h"
#include "../utils/logging.h"
#include "../utils/timer.h"

#include <algorithm>
#include <limits>
#include <numeric>
#include <queue>


using namespace std;

namespace symbolic {
SymStateSpaceManager::SymStateSpaceManager(SymVariables *sym_vars, const SymParameters &sym_params, const shared_ptr<AbstractTask> &task)
    : sym_vars(sym_vars),
      sym_params(sym_params),
      task(task),
      initial_state(sym_vars->zeroBDD()),
      goal(sym_vars->zeroBDD()),
      sym_mutexes(sym_vars, sym_params),
      sym_transition_relations(sym_vars, sym_params) {
    // Transform initial state and goal states if axioms are present
    if (task_properties::has_axioms(TaskProxy(*task))) {
        initial_state = sym_vars->get_axiom_compiliation()->get_compilied_init_state();
        goal = sym_vars->get_axiom_compiliation()->get_compilied_goal_state();
    } else {
        initial_state = sym_vars->getStateBDD(task->get_initial_state_values());
        vector<pair<int, int>> goal_facts;
        for (int i = 0; i < task->get_num_goals(); i++) {
            auto fact = task->get_goal_fact(i);
            goal_facts.emplace_back(fact.var, fact.value);
        }
        goal = sym_vars->getPartialStateBDD(goal_facts);
    }

    sym_mutexes.init(task);
    sym_transition_relations.init(task, sym_mutexes);
}

void SymStateSpaceManager::zero_preimage(BDD bdd, vector<BDD> &res, int node_limit) const {
    for (const auto &tr : sym_transition_relations.get_transition_relations().at(0)) {
        res.push_back(tr->preimage(bdd, node_limit));
    }
}

void SymStateSpaceManager::zero_image(BDD bdd, vector<BDD> &res, int node_limit) const {
    for (const auto &tr : sym_transition_relations.get_transition_relations().at(0)) {
        res.push_back(tr->image(bdd, node_limit));
    }
}

void SymStateSpaceManager::cost_preimage(BDD bdd, map<int, vector<BDD>> &res, int node_limit) const {
    for (auto trs : sym_transition_relations.get_transition_relations()) {
        int cost = trs.first;
        if (cost == 0)
            continue;
        for (size_t i = 0; i < trs.second.size(); i++) {
            BDD result = trs.second[i]->preimage(bdd, node_limit);
            res[cost].push_back(result);
        }
    }
}

void SymStateSpaceManager::cost_image(BDD bdd, map<int, vector<BDD>> &res, int node_limit) const {
    for (auto trs : sym_transition_relations.get_transition_relations()) {
        int cost = trs.first;
        if (cost == 0)
            continue;
        for (size_t i = 0; i < trs.second.size(); i++) {
            BDD result = trs.second[i]->image(bdd, node_limit);
            res[cost].push_back(result);
        }
    }
}

BDD SymStateSpaceManager::filter_mutex(BDD bdd, bool fw, int node_limit, bool initialization) {
    BDD res = bdd;
    const vector<BDD> &notDeadEndBDDs = fw ? sym_mutexes.notDeadEndFw : sym_mutexes.notDeadEndBw;
    for (const BDD &notDeadEnd : notDeadEndBDDs) {
        assert(!(notDeadEnd.IsZero()));
        res = res.And(notDeadEnd, node_limit);
    }

    const vector<BDD> &notMutexBDDs = (fw ? sym_mutexes.notMutexBDDsFw : sym_mutexes.notMutexBDDsBw);

    switch (sym_params.mutex_type) {
    case MutexType::MUTEX_NOT:
        break;
    case MutexType::MUTEX_EDELETION:
        if (initialization) {
            for (const BDD &notMutexBDD : notMutexBDDs) {
                res = res.And(notMutexBDD, node_limit);
            }
        }
        break;
    case MutexType::MUTEX_AND:
        for (const BDD &notMutexBDD : notMutexBDDs) {
            res = res.And(notMutexBDD, node_limit);
        }
        break;
    }
    return res;
}

int SymStateSpaceManager::filterMutexBucket(vector<BDD> &bucket, bool fw, bool initialization, int max_time, int max_nodes) {
    int numFiltered = 0;
    set_time_limit(max_time);
    try {
        for (size_t i = 0; i < bucket.size(); ++i) {
            bucket[i] = filter_mutex(bucket[i], fw, max_nodes, initialization);
            numFiltered++;
        }
    } catch (BDDError e) {
    }
    unset_time_limit();

    return numFiltered;
}

void SymStateSpaceManager::filter_mutex(Bucket &bucket, bool fw, bool initialization) {
    filterMutexBucket(bucket, fw, initialization, sym_params.max_aux_time, sym_params.max_aux_nodes);
}

void SymStateSpaceManager::merge_bucket(Bucket &bucket) const {
    merge_bucket(bucket, sym_params.max_aux_time, sym_params.max_aux_nodes);
}

void SymStateSpaceManager::merge_bucket_and(Bucket &bucket) const {
    merge_bucket_and(bucket, sym_params.max_aux_time, sym_params.max_aux_nodes);
}
}
