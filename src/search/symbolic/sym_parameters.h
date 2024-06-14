#ifndef SYMBOLIC_SYM_PARAMETERS_H
#define SYMBOLIC_SYM_PARAMETERS_H

#include <algorithm>

#include "sym_enums.h"

#include "../abstract_task.h"

namespace plugins {
class Options;
class Feature;
} // namespace options

namespace symbolic {
struct SymParameters {
    ConditionalEffectsTransitionType ce_transition_type;
    int max_tr_size, max_tr_time;

    MutexType mutex_type;
    int max_mutex_size, max_mutex_time;

    int max_aux_nodes, max_aux_time; // Time and memory bounds for auxiliary operations

    bool fast_sdac_generation;

    int max_alloted_time, max_alloted_nodes; // max alloted time and nodes to a step
    double ratio_alloted_time, ratio_alloted_nodes; // factor to multiply the estimation

    bool non_stop;

    SymParameters(const plugins::Options &opts, const std::shared_ptr<AbstractTask> &task);

    void increase_bound();

    static void add_options_to_feature(plugins::Feature &feature);

    void print_options() const;
};
}

#endif
