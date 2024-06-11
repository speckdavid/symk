#include "sym_parameters.h"

#include "../plugins/plugin.h"
#include "../task_utils/task_properties.h"
#include "../utils/logging.h"
#include "../utils/timer.h"

using namespace std;

namespace symbolic {
SymParameters::SymParameters(const plugins::Options &opts, const shared_ptr<AbstractTask> &task)
    : ce_transition_type(opts.get<ConditionalEffectsTransitionType>("ce_transition_type")),
      max_tr_size(opts.get<int>("max_tr_size")),
      max_tr_time(opts.get<int>("max_tr_time")),
      mutex_type(opts.get<MutexType>("mutex_type")),
      max_mutex_size(opts.get<int>("max_mutex_size")),
      max_mutex_time(opts.get<int>("max_mutex_time")),
      max_aux_nodes(opts.get<int>("max_aux_nodes")),
      max_aux_time(opts.get<int>("max_aux_time")),
      fast_sdac_generation(opts.get<bool>("fast_sdac_generation")),
      max_alloted_time(opts.get<int>("max_alloted_time")),
      max_alloted_nodes(opts.get<int>("max_alloted_nodes")),
      ratio_alloted_time(opts.get<double>("ratio_alloted_time")),
      ratio_alloted_nodes(opts.get<double>("ratio_alloted_nodes")),
      non_stop(opts.get<bool>("non_stop")) {
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
    if (is_ce_transition_type_conjunctive(ce_transition_type)
        && task_properties::has_conditional_effects(TaskProxy(*task))
        && task_properties::has_sdac_cost_operator(TaskProxy(*task))) {
        utils::g_log << "Conditional effect transition type changed to MONOLITHIC because the domain has "
            "state-dependent action cost. Support not implemented yet!" << endl;
        ce_transition_type = ConditionalEffectsTransitionType::MONOLITHIC;
    }

    max_alloted_nodes = max_alloted_nodes < 0 ? 0 : max_alloted_nodes;
    max_alloted_time = max_alloted_time < 0 ? 0 : max_alloted_time;
}

void SymParameters::increase_bound() {
    max_alloted_nodes = static_cast<int>(max_alloted_nodes * ratio_alloted_nodes);
    if (max_alloted_nodes <= 0)
        max_alloted_nodes = 0;

    max_alloted_time = static_cast<int>(max_alloted_time * ratio_alloted_time);
    if (max_alloted_time <= 0)
        max_alloted_time = 0;
    utils::g_log << "Increase allot limits! "
                 << "Max alloted time: " << max_alloted_time / 1000
                 << "s nodes: " << max_alloted_nodes << endl;
}

void SymParameters::print_options() const {
    utils::g_log << "TR(time=" << max_tr_time << ", nodes=" << max_tr_size << ", ce_type=" << ce_transition_type << ")" << endl;
    utils::g_log << "Mutex(time=" << max_mutex_time << ", nodes=" << max_mutex_size
                 << ", type=" << mutex_type << ")" << endl;
    utils::g_log << "Aux(time=" << max_aux_time << ", nodes=" << max_aux_nodes << ")" << endl;
    utils::g_log << "Max alloted time (for bd): " << (max_alloted_time == 0 ? "INF" : to_string(max_alloted_time / 1000.0) + "s")
                 << " nodes: " << (max_alloted_nodes == 0 ? "INF" : to_string(max_alloted_nodes)) << endl;
    utils::g_log << "Mult alloted time (for bd): " << ratio_alloted_time
                 << " nodes: " << ratio_alloted_nodes << endl;
}

void SymParameters::add_options_to_feature(plugins::Feature &feature) {
    feature.add_option<ConditionalEffectsTransitionType>("ce_transition_type", "ce transition type", "CONJUNCTIVE_EARLY_QUANTIFICATION");
    feature.add_option<int>("max_tr_size", "maximum size of TR BDDs", "100000");
    feature.add_option<int>("max_tr_time", "maximum time (ms) to generate TR BDDs", "60000");
    feature.add_option<MutexType>("mutex_type", "mutex type", "MUTEX_EDELETION");
    feature.add_option<int>("max_mutex_size", "maximum size of mutex BDDs", "100000");
    feature.add_option<int>("max_mutex_time", "maximum time (ms) to generate mutex BDDs", "60000");
    feature.add_option<int>("max_aux_nodes", "maximum size in pop operations", "1000000");
    feature.add_option<int>("max_aux_time", "maximum time (ms) in pop operations", "2000");
    feature.add_option<bool>("fast_sdac_generation", "Generates one TR per original operators and reuses it.", "true");
    feature.add_option<int>("max_alloted_time", "maximum alloted time for an step", to_string(60000));
    feature.add_option<int>("max_alloted_nodes", "maximum alloted nodes for an step", to_string(10000000));
    feature.add_option<double>("ratio_alloted_time", "multiplier to decide alloted time for a step", "2.0");
    feature.add_option<double>("ratio_alloted_nodes", "multiplier to decide alloted nodes for a step", "2.0");
    feature.add_option<bool>("non_stop", "Removes initial state from closed to avoid backward search to stop.", "false");
}
}
