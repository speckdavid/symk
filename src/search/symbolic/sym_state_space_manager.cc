#include "sym_state_space_manager.h"

#include "sym_enums.h"
#include "sym_utils.h"

#include "../abstract_task.h"
#include "../mutex_group.h"
#include "../options/option_parser.h"
#include "../options/options.h"
#include "../task_proxy.h"
#include "../task_utils/task_properties.h"
#include "../utils/logging.h"
#include "../utils/timer.h"

#include <algorithm>
#include <limits>
#include <queue>

using namespace std;

namespace symbolic {
SymStateSpaceManager::SymStateSpaceManager(SymVariables *sym_vars, const SymParameters &sym_params)
    : sym_vars(sym_vars),
      sym_params(sym_params),
      initial_state(sym_vars->zeroBDD()),
      goal(sym_vars->zeroBDD()),
      min_transition_cost(0),
      hasTR0(false) {}

void SymStateSpaceManager::zero_preimage(BDD bdd, vector<BDD> &res, int node_limit) const {
    for (const auto &tr : transitions.at(0)) {
        res.push_back(tr.preimage(bdd, node_limit));
    }
}

void SymStateSpaceManager::zero_image(BDD bdd, vector<BDD> &res, int node_limit) const {
    for (const auto &tr : transitions.at(0)) {
        res.push_back(tr.image(bdd, node_limit));
    }
}

void SymStateSpaceManager::cost_preimage(BDD bdd, map<int, vector<BDD>> &res, int node_limit) const {
    for (auto trs : transitions) {
        int cost = trs.first;
        if (cost == 0)
            continue;
        for (size_t i = 0; i < trs.second.size(); i++) {
            BDD result = trs.second[i].preimage(bdd, node_limit);
            res[cost].push_back(result);
        }
    }
}

void SymStateSpaceManager::cost_image(BDD bdd, map<int, vector<BDD>> &res, int node_limit) const {
    for (auto trs : transitions) {
        int cost = trs.first;
        if (cost == 0)
            continue;
        for (size_t i = 0; i < trs.second.size(); i++) {
            BDD result = trs.second[i].image(bdd, node_limit);
            res[cost].push_back(result);
        }
    }
}

BDD SymStateSpaceManager::filter_mutex(BDD bdd, bool fw, int node_limit, bool initialization) {
    BDD res = bdd;
    const vector<BDD> &notDeadEndBDDs = fw ? notDeadEndFw : notDeadEndBw;
    for (const BDD &notDeadEnd : notDeadEndBDDs) {
        assert(!(notDeadEnd.IsZero()));
        res = res.And(notDeadEnd, node_limit);
    }

    const vector<BDD> &notMutexBDDs = (fw ? notMutexBDDsFw : notMutexBDDsBw);

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

int SymStateSpaceManager::filterMutexBucket(vector<BDD> &bucket,
                                            bool fw,
                                            bool initialization,
                                            int max_time,
                                            int max_nodes) {
    int numFiltered = 0;
    setTimeLimit(max_time);
    try {
        for (size_t i = 0; i < bucket.size(); ++i) {
            bucket[i] = filter_mutex(bucket[i], fw, max_nodes, initialization);
            numFiltered++;
        }
    } catch (BDDError e) {
    }
    unsetTimeLimit();

    return numFiltered;
}

void SymStateSpaceManager::filterMutex(Bucket &bucket, bool fw, bool initialization) {
    filterMutexBucket(bucket, fw, initialization, sym_params.max_aux_time,
                      sym_params.max_aux_nodes);
}

void SymStateSpaceManager::mergeBucket(Bucket &bucket) const {
    mergeBucket(bucket, sym_params.max_aux_time, sym_params.max_aux_nodes);
}

void SymStateSpaceManager::mergeBucketAnd(Bucket &bucket) const {
    mergeBucketAnd(bucket, sym_params.max_aux_time, sym_params.max_aux_nodes);
}

void SymStateSpaceManager::init_transitions(const map<int, vector<TransitionRelation>> &(indTRs)) {
    transitions = indTRs; // Copy
    if (transitions.empty()) {
        hasTR0 = false;
        min_transition_cost = 1;
        return;
    }

    for (map<int, vector<TransitionRelation>>::iterator it = transitions.begin();
         it != transitions.end(); ++it) {
        merge(sym_vars, it->second, mergeTR, sym_params.max_tr_time, sym_params.max_tr_size);
    }

    min_transition_cost = transitions.begin()->first;
    if (min_transition_cost == 0) {
        hasTR0 = true;
        if (transitions.size() > 1) {
            min_transition_cost = (transitions.begin()++)->first;
        }
    }
}

}
