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

void SymTransitionRelations::init(const std::shared_ptr<AbstractTask> &task, const SymMutexes &sym_mutexes) {
    utils::g_log << "Creating " << task->get_num_operators() << " transition relations..." << flush;
    init_individual_transitions(task, sym_mutexes);
    utils::g_log << "Done!" << endl;

    min_transition_cost = individual_transitions.begin()->first;

    utils::g_log << "Merging transition relations: " << task->get_num_operators() << " => " << flush;
    create_merged_transitions();
    utils::g_log << accumulate(transitions.begin(), transitions.end(), 0, [](int sum, const auto &pair)
                               {return sum + pair.second.size();}) << endl << endl;
}

void SymTransitionRelations::init_individual_transitions(const std::shared_ptr<AbstractTask> &task, const SymMutexes &sym_mutexes) {
    shared_ptr<extra_tasks::SdacTask> sdac_task = dynamic_pointer_cast<extra_tasks::SdacTask>(task);
    if (sdac_task == nullptr) {
        create_single_trs(task, sym_mutexes);
    } else {
        utils::g_log << "Creating " << task->get_num_operators() << " transition relations..." << flush;
        create_single_sdac_trs(sdac_task, sym_params.fast_sdac_generation);
    }
}

void SymTransitionRelations::create_single_trs(const std::shared_ptr<AbstractTask> &task, const SymMutexes &sym_mutexes) {
    TaskProxy task_proxy(*task);
    for (int i = 0; i < task->get_num_operators(); ++i) {
        int cost = task->get_operator_cost(i, false);

        if (false && task_properties::has_conditional_effects(task_proxy, OperatorID(i))) {
            auto effect_aggregated_task = make_shared<extra_tasks::EffectAggregatedTask>(task);
            auto conj_tr = ConjunctiveTransitionRelation(sym_vars, OperatorID(i), effect_aggregated_task);
            conj_tr.init();
            utils::g_log << "TR nodes/size: " << conj_tr.nodeCount() << " " << conj_tr.size() << endl;
        } else {
            individual_transitions[cost].emplace_back(sym_vars, OperatorID(i), task);

            individual_transitions[cost].back().init();

            if (sym_params.mutex_type == MutexType::MUTEX_EDELETION) {
                individual_transitions[cost].back().edeletion(sym_mutexes.notMutexBDDsByFluentFw,
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
            individual_transitions[cost].emplace_back(sym_vars, OperatorID(i), sdac_task);
            DisjunctiveTransitionRelation &cur_tr = individual_transitions[cost].back();
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
            individual_transitions[cost].emplace_back(sym_vars, OperatorID(i), sdac_task);
            DisjunctiveTransitionRelation &cur_tr = individual_transitions[cost].back();
            cur_tr.init_from_tr(look_up[parent_op_id]);
            cur_tr.set_cost(cost);
            cur_tr.setOpsIds({OperatorID(i)});
            cur_tr.add_condition(sdac_task->get_operator_cost_condition(i, false));
        }
    }
}

void SymTransitionRelations::create_merged_transitions() {
    transitions = individual_transitions; // Copy
    assert(!transitions.empty());

    for (auto it = transitions.begin(); it != transitions.end(); ++it) {
        merge(sym_vars, it->second, mergeTR, sym_params.max_tr_time, sym_params.max_tr_size);
    }
}

int SymTransitionRelations::get_min_transition_cost() const {
    return min_transition_cost;
}

bool SymTransitionRelations::has_zero_cost_transition() const {
    return min_transition_cost == 0;
}

const std::map<int, std::vector<DisjunctiveTransitionRelation>> &SymTransitionRelations::get_transition_relations() const {
    return transitions;
}

const std::map<int, std::vector<DisjunctiveTransitionRelation>> &SymTransitionRelations::get_individual_transition_relations() const {
    return individual_transitions;
}
}
