#include "sym_state_space_manager.h"

#include "../abstract_task.h"
#include "../mutex_group.h"
#include "../options/option_parser.h"
#include "../options/options.h"
#include "../task_proxy.h"
#include "../task_utils/task_properties.h"
#include "../utils/logging.h"
#include "../utils/timer.h"
#include "sym_enums.h"
#include "sym_utils.h"

#include <algorithm>
#include <limits>
#include <queue>

using namespace std;

namespace symbolic {
SymStateSpaceManager::SymStateSpaceManager(SymVariables *v,
                                           const SymParamsMgr &params)
    : vars(v), p(params), initialState(v->zeroBDD()), goal(v->zeroBDD()),
      min_transition_cost(0), hasTR0(false) {}

void SymStateSpaceManager::dumpMutexBDDs(bool fw) const {
    if (fw) {
        utils::g_log << "Mutex BDD FW Size(" << p.max_mutex_size << "):";
        for (const auto &bdd : notMutexBDDsFw) {
            utils::g_log << " " << bdd.nodeCount();
        }
        utils::g_log << endl;
    } else {
        utils::g_log << "Mutex BDD BW Size(" << p.max_mutex_size << "):";
        for (const auto &bdd : notMutexBDDsBw) {
            utils::g_log << " " << bdd.nodeCount();
        }
        utils::g_log << endl;
    }
}

void SymStateSpaceManager::zero_preimage(const BDD &bdd, vector<BDD> &res,
                                         int nodeLimit) const {
    for (const auto &tr : transitions.at(0)) {
        res.push_back(tr.preimage(bdd, nodeLimit));
    }
}

void SymStateSpaceManager::zero_image(const BDD &bdd, vector<BDD> &res,
                                      int nodeLimit) const {
    for (const auto &tr : transitions.at(0)) {
        res.push_back(tr.image(bdd, nodeLimit));
    }
}

void SymStateSpaceManager::cost_preimage(const BDD &bdd,
                                         map<int, vector<BDD>> &res,
                                         int nodeLimit) const {
    for (auto trs : transitions) {
        int cost = trs.first;
        if (cost == 0)
            continue;
        for (size_t i = 0; i < trs.second.size(); i++) {
            BDD result = trs.second[i].preimage(bdd, nodeLimit);
            res[cost].push_back(result);
        }
    }
}

void SymStateSpaceManager::cost_image(const BDD &bdd,
                                      map<int, vector<BDD>> &res,
                                      int nodeLimit) const {
    for (auto trs : transitions) {
        int cost = trs.first;
        if (cost == 0)
            continue;
        for (size_t i = 0; i < trs.second.size(); i++) {
            BDD result = trs.second[i].image(bdd, nodeLimit);
            res[cost].push_back(result);
        }
    }
}

BDD SymStateSpaceManager::filter_mutex(const BDD &bdd, bool fw, int nodeLimit,
                                       bool initialization) {
    BDD res = bdd;
    const vector<BDD> &notDeadEndBDDs = fw ? notDeadEndFw : notDeadEndBw;
    for (const BDD &notDeadEnd : notDeadEndBDDs) {
        assert(!(notDeadEnd.IsZero()));
        res = res.And(notDeadEnd, nodeLimit);
    }

    const vector<BDD> &notMutexBDDs = (fw ? notMutexBDDsFw : notMutexBDDsBw);

    switch (p.mutex_type) {
    case MutexType::MUTEX_NOT:
        break;
    case MutexType::MUTEX_EDELETION:
        if (initialization) {
            for (const BDD &notMutexBDD : notMutexBDDs) {
                res = res.And(notMutexBDD, nodeLimit);
            }
        }
        break;
    case MutexType::MUTEX_AND:
        for (const BDD &notMutexBDD : notMutexBDDs) {
            res = res.And(notMutexBDD, nodeLimit);
        }
        break;
    }
    return res;
}

int SymStateSpaceManager::filterMutexBucket(vector<BDD> &bucket, bool fw,
                                            bool initialization, int maxTime,
                                            int maxNodes) {
    int numFiltered = 0;
    setTimeLimit(maxTime);
    try {
        for (size_t i = 0; i < bucket.size(); ++i) {
            bucket[i] = filter_mutex(bucket[i], fw, maxNodes, initialization);
            numFiltered++;
        }
    } catch (BDDError e) {
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

SymParamsMgr::SymParamsMgr(const options::Options &opts,
                           const shared_ptr<AbstractTask> &task)
    : max_tr_size(opts.get<int>("max_tr_size")),
      max_tr_time(opts.get<int>("max_tr_time")),
      mutex_type(opts.get<MutexType>("mutex_type")),
      max_mutex_size(opts.get<int>("max_mutex_size")),
      max_mutex_time(opts.get<int>("max_mutex_time")),
      max_aux_nodes(opts.get<int>("max_aux_nodes")),
      max_aux_time(opts.get<int>("max_aux_time")),
      fast_sdac_generation(opts.get<bool>("fast_sdac_generation")) {
    // Don't use edeletion with conditional effects
    if (mutex_type == MutexType::MUTEX_EDELETION &&
        (task_properties::has_conditional_effects(TaskProxy(*task))
         || task_properties::has_axioms(TaskProxy(*task))
         || task_properties::has_sdac_cost_operator(TaskProxy(*task)))) {
        utils::g_log << "Mutex type changed to mutex_and because the domain has "
            "conditional effects, axioms and/or sdac."
                     << endl;
        mutex_type = MutexType::MUTEX_AND;
    }
}

void SymParamsMgr::print_options() const {
    utils::g_log << "TR(time=" << max_tr_time << ", nodes=" << max_tr_size << ")" << endl;
    utils::g_log << "Mutex(time=" << max_mutex_time << ", nodes=" << max_mutex_size
                 << ", type=" << mutex_type << ")" << endl;
    utils::g_log << "Aux(time=" << max_aux_time << ", nodes=" << max_aux_nodes << ")"
                 << endl;
}

void SymParamsMgr::add_options_to_parser(options::OptionParser &parser) {
    parser.add_option<int>("max_tr_size", "maximum size of TR BDDs", "100000");

    parser.add_option<int>("max_tr_time", "maximum time (ms) to generate TR BDDs",
                           "60000");

    parser.add_enum_option<MutexType>("mutex_type", MutexTypeValues, "mutex type",
                                      "MUTEX_EDELETION");

    parser.add_option<int>("max_mutex_size", "maximum size of mutex BDDs",
                           "100000");

    parser.add_option<int>("max_mutex_time",
                           "maximum time (ms) to generate mutex BDDs", "60000");

    parser.add_option<int>("max_aux_nodes", "maximum size in pop operations",
                           "1000000");
    parser.add_option<int>("max_aux_time", "maximum time (ms) in pop operations",
                           "2000");
    parser.add_option<bool>(
        "fast_sdac_generation",
        "Generates one TR per original operators and reuses it.",
        "true");
}

ostream &operator<<(ostream &os, const SymStateSpaceManager &abs) {
    abs.print(os, false);
    return os;
}
}
