#include "sym_state_space_manager.h"

#include "sym_enums.h"
#include "sym_utils.h"

#include "../abstract_task.h"
#include "../mutex_group.h"
#include "../task_proxy.h"

#include "../options/option_parser.h"
#include "../options/options.h"
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
      min_transition_cost(0),
      has_zero_cost_transition(false),
      sym_mutexes(sym_vars, sym_params) {
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
    init_transitions();
}

void SymStateSpaceManager::init_transitions() {
    utils::g_log << "Creating " << task->get_num_operators() << " transition relations..." << flush;
    init_individual_transitions();
    utils::g_log << "Done!" << endl;

    utils::g_log << "Merging transition relations: " << task->get_num_operators() << " => " << flush;
    init_merged_transitions();
    utils::g_log << accumulate(transitions.begin(), transitions.end(), 0, [](int sum, const auto &pair)
                               {return sum + pair.second.size();}) << endl << endl;
}

void SymStateSpaceManager::init_individual_transitions() {
    shared_ptr<extra_tasks::SdacTask> sdac_task = dynamic_pointer_cast<extra_tasks::SdacTask>(task);
    if (sdac_task == nullptr) {
        create_single_trs();
    } else {
        utils::g_log << "Creating " << task->get_num_operators() << " transition relations..." << flush;
        create_single_sdac_trs(sdac_task, sym_params.fast_sdac_generation);
    }
}

void SymStateSpaceManager::create_single_trs() {
    for (int i = 0; i < task->get_num_operators(); ++i) {
        int cost = task->get_operator_cost(i, false);
        individual_transitions[cost].emplace_back(sym_vars, OperatorID(i), task);

        individual_transitions[cost].back().init();

        if (sym_params.mutex_type == MutexType::MUTEX_EDELETION) {
            individual_transitions[cost].back().edeletion(sym_mutexes.notMutexBDDsByFluentFw,
                                                          sym_mutexes.notMutexBDDsByFluentBw,
                                                          sym_mutexes.exactlyOneBDDsByFluent);
        }
    }
}

void SymStateSpaceManager::create_single_sdac_trs(shared_ptr<extra_tasks::SdacTask> sdac_task, bool fast_creation) {
    const string generation_mode = fast_creation ? "Fast" : "Normal";
    utils::g_log << generation_mode << " SDAC TR generation." << endl;

    if (!fast_creation) {
        for (int i = 0; i < sdac_task->get_num_operators(); ++i) {
            const int cost = sdac_task->get_operator_cost(i, false);
            individual_transitions[cost].emplace_back(sym_vars, OperatorID(i), sdac_task);
            TransitionRelation &cur_tr = individual_transitions[cost].back();
            cur_tr.init();
            cur_tr.add_condition(sdac_task->get_operator_cost_condition(i, false));
        }
    } else {
        vector<TransitionRelation> look_up;
        int last_parent_id = -1;

        for (int i = 0; i < sdac_task->get_num_operators(); ++i) {
            const int parent_op_id = sdac_task->convert_operator_index_to_parent(i);
            if (last_parent_id != parent_op_id) {
                last_parent_id = parent_op_id;
                look_up.emplace_back(sym_vars, OperatorID(i), sdac_task);
                look_up.back().init();
            }
        }

        for (int i = 0; i < sdac_task->get_num_operators(); ++i) {
            const int parent_op_id = sdac_task->convert_operator_index_to_parent(i);
            const int cost = sdac_task->get_operator_cost(i, false);
            individual_transitions[cost].emplace_back(sym_vars, OperatorID(i), sdac_task);
            TransitionRelation &cur_tr = individual_transitions[cost].back();
            cur_tr.init_from_tr(look_up[parent_op_id]);
            cur_tr.set_cost(cost);
            cur_tr.setOpsIds({OperatorID(i)});
            cur_tr.add_condition(sdac_task->get_operator_cost_condition(i, false));
        }
    }
}

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
    filterMutexBucket(bucket, fw, initialization, sym_params.max_aux_time, sym_params.max_aux_nodes);
}

void SymStateSpaceManager::mergeBucket(Bucket &bucket) const {
    mergeBucket(bucket, sym_params.max_aux_time, sym_params.max_aux_nodes);
}

void SymStateSpaceManager::mergeBucketAnd(Bucket &bucket) const {
    mergeBucketAnd(bucket, sym_params.max_aux_time, sym_params.max_aux_nodes);
}

void SymStateSpaceManager::init_merged_transitions() {
    transitions = individual_transitions; // Copy
    if (transitions.empty()) {
        has_zero_cost_transition = false;
        min_transition_cost = 1;
        return;
    }

    for (map<int, vector<TransitionRelation>>::iterator it = transitions.begin();
         it != transitions.end(); ++it) {
        merge(sym_vars, it->second, mergeTR, sym_params.max_tr_time, sym_params.max_tr_size);
    }

    min_transition_cost = transitions.begin()->first;
    if (min_transition_cost == 0) {
        has_zero_cost_transition = true;
        if (transitions.size() > 1) {
            min_transition_cost = (transitions.begin()++)->first;
        }
    }
}
}
