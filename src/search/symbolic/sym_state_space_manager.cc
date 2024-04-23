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
#include <queue>

using namespace std;

namespace symbolic {
SymStateSpaceManager::SymStateSpaceManager(SymVariables *sym_vars, const SymParameters &sym_params, const std::shared_ptr<AbstractTask> &task)
    : sym_vars(sym_vars),
      sym_params(sym_params),
      task(task),
      initial_state(sym_vars->zeroBDD()),
      goal(sym_vars->zeroBDD()),
      min_transition_cost(0),
      has_zero_cost_transition(false) {
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

    init_mutex(task->get_mutex_groups());

    init_individual_transitions();
    utils::g_log << "Merging transition relations: " << task->get_num_operators() << " => " << flush;
    init_transitions(indTRs);
    int num_trs = 0;
    for (auto const &pair : transitions) {
        num_trs += pair.second.size();
    }
    utils::g_log << num_trs << endl;
    cout << endl;
}

void SymStateSpaceManager::init_individual_transitions() {
    utils::g_log << "Creating " << task->get_num_operators() << " transition relations..." << flush;
    shared_ptr<extra_tasks::SdacTask> sdac_task = dynamic_pointer_cast<extra_tasks::SdacTask>(task);
    if (sdac_task == nullptr) {
        create_single_trs();
    } else {
        utils::g_log << "Creating " << task->get_num_operators() << " transition relations..." << flush;
        create_single_sdac_trs(sdac_task, sym_params.fast_sdac_generation);
    }
    utils::g_log << "Done!" << endl;
}

void SymStateSpaceManager::create_single_trs() {
    for (int i = 0; i < task->get_num_operators(); ++i) {
        int cost = task->get_operator_cost(i, false);
        indTRs[cost].emplace_back(sym_vars, OperatorID(i), task);

        indTRs[cost].back().init();

        if (sym_params.mutex_type == MutexType::MUTEX_EDELETION) {
            indTRs[cost].back().edeletion(notMutexBDDsByFluentFw,
                                          notMutexBDDsByFluentBw,
                                          exactlyOneBDDsByFluent);
        }
    }
}

void SymStateSpaceManager::create_single_sdac_trs(shared_ptr<extra_tasks::SdacTask> sdac_task, bool fast_creation) {
    const string generation_mode = fast_creation ? "Fast" : "Normal";
    utils::g_log << generation_mode << " SDAC TR generation." << endl;

    if (!fast_creation) {
        for (int i = 0; i < sdac_task->get_num_operators(); ++i) {
            const int cost = sdac_task->get_operator_cost(i, false);
            indTRs[cost].emplace_back(sym_vars, OperatorID(i), sdac_task);
            TransitionRelation &indTR = indTRs[cost].back();
            indTR.init();
            indTR.add_condition(sdac_task->get_operator_cost_condition(i, false));
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
            indTRs[cost].emplace_back(sym_vars, OperatorID(i), sdac_task);
            TransitionRelation &indTR = indTRs[cost].back();
            indTR.init_from_tr(look_up[parent_op_id]);
            indTR.set_cost(cost);
            indTR.setOpsIds({OperatorID(i)});
            indTR.add_condition(sdac_task->get_operator_cost_condition(i, false));
        }
    }
}

void SymStateSpaceManager::init_mutex(const vector<MutexGroup> &mutex_groups) {
    // If (a) is initialized OR not using mutex OR edeletion does not need mutex
    if (sym_params.mutex_type == MutexType::MUTEX_NOT)
        return; // Skip mutex initialization

    bool genMutexBDD = true;
    bool genMutexBDDByFluent = (sym_params.mutex_type == MutexType::MUTEX_EDELETION);

    if (genMutexBDDByFluent) {
        // Initialize structure for exactlyOneBDDsByFluent (common to both
        // init_mutex calls)
        exactlyOneBDDsByFluent.resize(task->get_num_variables());
        for (int i = 0; i < task->get_num_variables(); ++i) {
            exactlyOneBDDsByFluent[i].resize(task->get_variable_domain_size(i));
            for (int j = 0; j < task->get_variable_domain_size(i); ++j) {
                exactlyOneBDDsByFluent[i][j] = oneBDD();
            }
        }
    }
    init_mutex(mutex_groups, genMutexBDD, genMutexBDDByFluent, false);
    init_mutex(mutex_groups, genMutexBDD, genMutexBDDByFluent, true);
}

void SymStateSpaceManager::init_mutex(const vector<MutexGroup> &mutex_groups, bool genMutexBDD, bool genMutexBDDByFluent, bool fw) {
    vector<vector<BDD>> &notMutexBDDsByFluent = (fw ? notMutexBDDsByFluentFw : notMutexBDDsByFluentBw);

    vector<BDD> &notMutexBDDs = (fw ? notMutexBDDsFw : notMutexBDDsBw);

    // BDD validStates = vars->oneBDD();
    int num_mutex = 0;
    int num_invariants = 0;

    if (genMutexBDDByFluent) {
        // Initialize structure for notMutexBDDsByFluent
        notMutexBDDsByFluent.resize(task->get_num_variables());
        for (int i = 0; i < task->get_num_variables(); ++i) {
            notMutexBDDsByFluent[i].resize(task->get_variable_domain_size(i));
            for (int j = 0; j < task->get_variable_domain_size(i); ++j) {
                notMutexBDDsByFluent[i][j] = oneBDD();
            }
        }
    }

    // Initialize mBDDByVar and invariant_bdds_by_fluent
    vector<BDD> mBDDByVar;
    mBDDByVar.reserve(task->get_num_variables());
    vector<vector<BDD>> invariant_bdds_by_fluent(task->get_num_variables());
    for (size_t i = 0; i < invariant_bdds_by_fluent.size(); i++) {
        mBDDByVar.push_back(oneBDD());
        invariant_bdds_by_fluent[i].resize(task->get_variable_domain_size(i));
        for (size_t j = 0; j < invariant_bdds_by_fluent[i].size(); j++) {
            invariant_bdds_by_fluent[i][j] = oneBDD();
        }
    }

    for (auto &mg : mutex_groups) {
        if (mg.pruneFW() != fw)
            continue;
        const vector<FactPair> &invariant_group = mg.getFacts();
        if (mg.isExactlyOne()) {
            BDD bddInvariant = zeroBDD();
            int var = numeric_limits<int>::max();
            int val = 0;
            bool exactlyOneRelevant = true;

            for (auto &fluent : invariant_group) {
                bddInvariant += sym_vars->preBDD(fluent.var, fluent.value);
                if (fluent.var < var) {
                    var = fluent.var;
                    val = fluent.value;
                }
            }

            if (exactlyOneRelevant) {
                num_invariants++;
                if (genMutexBDD) {
                    invariant_bdds_by_fluent[var][val] *= bddInvariant;
                }
                if (genMutexBDDByFluent) {
                    for (auto &fluent : invariant_group) {
                        exactlyOneBDDsByFluent[fluent.var][fluent.value] *= bddInvariant;
                    }
                }
            }
        }

        for (size_t i = 0; i < invariant_group.size(); ++i) {
            int var1 = invariant_group[i].var;
            int val1 = invariant_group[i].value;
            BDD f1 = sym_vars->preBDD(var1, val1);

            for (size_t j = i + 1; j < invariant_group.size(); ++j) {
                int var2 = invariant_group[j].var;
                int val2 = invariant_group[j].value;
                BDD f2 = sym_vars->preBDD(var2, val2);
                BDD mBDD = !(f1 * f2);
                if (genMutexBDD) {
                    num_mutex++;
                    mBDDByVar[min(var1, var2)] *= mBDD;
                    if (mBDDByVar[min(var1, var2)].nodeCount() > sym_params.max_mutex_size) {
                        notMutexBDDs.push_back(mBDDByVar[min(var1, var2)]);
                        mBDDByVar[min(var1, var2)] = sym_vars->oneBDD();
                    }
                }
                if (genMutexBDDByFluent) {
                    notMutexBDDsByFluent[var1][val1] *= mBDD;
                    notMutexBDDsByFluent[var2][val2] *= mBDD;
                }
            }
        }
    }

    if (genMutexBDD) {
        for (int var = 0; var < task->get_num_variables(); ++var) {
            if (!mBDDByVar[var].IsOne()) {
                notMutexBDDs.push_back(mBDDByVar[var]);
            }
            for (const BDD &bdd_inv : invariant_bdds_by_fluent[var]) {
                if (!bdd_inv.IsOne()) {
                    notMutexBDDs.push_back(bdd_inv);
                }
            }
        }

        merge(sym_vars, notMutexBDDs, mergeAndBDD, sym_params.max_mutex_time, sym_params.max_mutex_size);
        reverse(notMutexBDDs.begin(), notMutexBDDs.end());
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
