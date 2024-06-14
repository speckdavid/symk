#include "sym_transition_relations.h"

#include "sym_utils.h"

#include "../tasks/effect_aggregated_task.h"
#include "../tasks/sdac_task.h"
#include "../task_utils/task_properties.h"
#include "../utils/logging.h"

#include <numeric>

using namespace std;

namespace symbolic {
SymTransitionRelations::SymTransitionRelations(SymVariables *sym_vars, const SymParameters &sym_params) :
    sym_vars(sym_vars),
    sym_params(sym_params) {}

void SymTransitionRelations::init(const shared_ptr<AbstractTask> &task, const SymMutexes &sym_mutexes) {
    init_individual_transitions(task, sym_mutexes);

    utils::g_log << "Individual transition relations: " << get_size(individual_disj_transitions) + get_size(individual_conj_transitions);
    utils::g_log << " (disj=" << get_size(individual_disj_transitions);
    utils::g_log << ", conj=" << get_size(individual_conj_transitions) << ")" << endl;

    utils::g_log << "Merging transition relations..." << flush;
    create_merged_transitions();
    utils::g_log << "done!" << endl;

    // Fill individual_transitions with individual_disj_transitions
    for (const auto & [cost, tr_vec] : individual_disj_transitions) {
        for (const auto &tr : tr_vec) {
            individual_transitions[cost].emplace_back(make_shared<DisjunctiveTransitionRelation>(tr));
        }
    }

    // Fill transitions with disj_transitions
    for (const auto & [cost, tr_vec] : disj_transitions) {
        for (const auto &tr : tr_vec) {
            transitions[cost].emplace_back(make_shared<DisjunctiveTransitionRelation>(tr));
        }
    }

    // Fill transitions with individual_conj_transitions
    for (const auto & [cost, tr_vec] : individual_conj_transitions) {
        for (const auto &tr : tr_vec) {
            individual_transitions[cost].emplace_back(make_shared<ConjunctiveTransitionRelation>(tr));
            transitions[cost].emplace_back(make_shared<ConjunctiveTransitionRelation>(tr));
        }
    }

    min_transition_cost = individual_transitions.empty() ? numeric_limits<int>::max() : individual_transitions.begin()->first;
    utils::g_log << "Merged transition relations: " << get_size(transitions);
    utils::g_log << " (disj=" << get_size(transitions) - get_size(individual_conj_transitions);
    utils::g_log << ", conj=" << get_size(individual_conj_transitions) << ")" << endl;
}

void SymTransitionRelations::init_individual_transitions(const shared_ptr<AbstractTask> &task, const SymMutexes &sym_mutexes) {
    utils::g_log << "Creating " << task->get_num_operators() << " transition relations..." << flush;
    shared_ptr<extra_tasks::SdacTask> sdac_task = dynamic_pointer_cast<extra_tasks::SdacTask>(task);
    if (sdac_task == nullptr) {
        create_single_trs(task, sym_mutexes);
    } else {
        create_single_sdac_trs(sdac_task, sym_params.fast_sdac_generation);
    }
    utils::g_log << "Done!" << endl;
}

void SymTransitionRelations::create_single_trs(const shared_ptr<AbstractTask> &task, const SymMutexes &sym_mutexes) {
    // For Conjunctive Transiton Relations in presence of CEs
    shared_ptr<extra_tasks::EffectAggregatedTask> effect_aggregated_task = nullptr;
    TaskProxy task_proxy(*task);
    if (task_properties::has_conditional_effects(task_proxy)) {
        effect_aggregated_task = make_shared<extra_tasks::EffectAggregatedTask>(task);
    }


    for (int i = 0; i < task->get_num_operators(); ++i) {
        int cost = task->get_operator_cost(i, false);

        if (is_ce_transition_type_conjunctive(sym_params.ce_transition_type)
            && task_properties::has_conditional_effects(task_proxy, OperatorID(i))) {
            bool early_quantification = sym_params.ce_transition_type == ConditionalEffectsTransitionType::CONJUNCTIVE_EARLY_QUANTIFICATION;
            individual_conj_transitions[cost].emplace_back(sym_vars, OperatorID(i), effect_aggregated_task,
                                                           early_quantification);
            individual_conj_transitions[cost].back().init();
            // if (sym_params.mutex_type == MutexType::MUTEX_EDELETION) {
            //     individual_conj_transitions[cost].back().edeletion(sym_mutexes.notMutexBDDsByFluentFw,
            //                                                        sym_mutexes.notMutexBDDsByFluentBw,
            //                                                        sym_mutexes.exactlyOneBDDsByFluent);
            // }
        } else {
            individual_disj_transitions[cost].emplace_back(sym_vars, OperatorID(i), task);
            individual_disj_transitions[cost].back().init();

            if (sym_params.mutex_type == MutexType::MUTEX_EDELETION) {
                individual_disj_transitions[cost].back().edeletion(sym_mutexes.notMutexBDDsByFluentFw,
                                                                   sym_mutexes.notMutexBDDsByFluentBw,
                                                                   sym_mutexes.exactlyOneBDDsByFluent);
            }
        }
    }
}

void SymTransitionRelations::create_single_sdac_trs(const shared_ptr<extra_tasks::SdacTask> &sdac_task, bool fast_creation) {
    const string generation_mode = fast_creation ? "Fast" : "Normal";
    utils::g_log << generation_mode << " SDAC TR generation." << endl;

    if (!fast_creation) {
        for (int i = 0; i < sdac_task->get_num_operators(); ++i) {
            const int cost = sdac_task->get_operator_cost(i, false);
            individual_disj_transitions[cost].emplace_back(sym_vars, OperatorID(i), sdac_task);
            auto &cur_tr = individual_disj_transitions[cost].back();
            cur_tr.init();
            cur_tr.add_condition(sdac_task->get_operator_cost_condition(i, false));
        }
    } else {
        vector<DisjunctiveTransitionRelation> look_up;
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
            individual_disj_transitions[cost].emplace_back(sym_vars, OperatorID(i), sdac_task);
            auto &cur_tr = individual_disj_transitions[cost].back();
            cur_tr.init_from_tr(look_up[parent_op_id]);
            cur_tr.set_cost(cost);
            cur_tr.setOpsIds({OperatorID(i)});
            cur_tr.add_condition(sdac_task->get_operator_cost_condition(i, false));
        }
    }
}

void SymTransitionRelations::create_merged_transitions() {
    for (auto & [cost, tr_vec] : individual_conj_transitions) {
        for (auto &tr : tr_vec) {
            tr.merge_transitions(sym_params.max_tr_time, sym_params.max_tr_size);
        }
    }

    // Monolithic Conjunctive transitions can be moved to individual_disj_transitions
    // By that we can further merge them
    move_monolithic_conj_transitions();

    disj_transitions = individual_disj_transitions; // Copy

    for (auto it = disj_transitions.begin(); it != disj_transitions.end(); ++it) {
        merge(sym_vars, it->second, disjunctive_tr_merge, sym_params.max_tr_time, sym_params.max_tr_size);
    }
}

void SymTransitionRelations::move_monolithic_conj_transitions() {
    // Temporary map to hold new individual_disj_transitions
    map<int, vector<ConjunctiveTransitionRelation>> new_individual_conj_transitions;

    for (auto & [cost, tr_vec] : individual_conj_transitions) {
        for (auto &conj_tr : tr_vec) {
            if (conj_tr.is_monolithic()) {
                individual_disj_transitions[cost].push_back(conj_tr.get_transitions().at(0));
            } else {
                new_individual_conj_transitions[cost].push_back(conj_tr);
            }
        }
    }

    individual_conj_transitions = move(new_individual_conj_transitions);
}

template<class T>
int SymTransitionRelations::get_size(map<int, vector<T>> transitions) const {
    return accumulate(transitions.begin(), transitions.end(), 0, [](int sum, const auto &pair) {return sum + pair.second.size();});
}

int SymTransitionRelations::get_min_transition_cost() const {
    return min_transition_cost;
}

bool SymTransitionRelations::has_zero_cost_transition() const {
    return min_transition_cost == 0;
}

bool SymTransitionRelations::has_unit_cost() const {
    return transitions.size();
}

const map<int, vector<TransitionRelationPtr>> &SymTransitionRelations::get_transition_relations() const {
    return transitions;
}

const map<int, vector<TransitionRelationPtr>> &SymTransitionRelations::get_individual_transition_relations() const {
    return individual_transitions;
}
}
