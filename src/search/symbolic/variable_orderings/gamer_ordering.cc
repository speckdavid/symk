#include "gamer_ordering.h"

#include "../../plugins/plugin.h"
#include "../../task_utils/causal_graph.h"
#include "../../utils/logging.h"
#include "../../utils/timer.h"

#include <numeric>

using namespace std;

namespace symbolic {
GamerOrdering::GamerOrdering(const plugins::Options &opts)
    : VariableOrdering(opts),
      rng(make_shared<utils::RandomNumberGenerator>(0)),
      timeout(opts.get<double>("timeout")) {
}

double GamerOrdering::influence(int var1, int var2) const {
    return influence_graph[var1][var2];
}

void GamerOrdering::set_influence(int var1, int var2, double value) {
    influence_graph[var1][var2] = value;
    influence_graph[var2][var1] = value;
}

void GamerOrdering::compute_ordering(
    const TaskProxy &task_proxy, vector<int> &var_order) {
    var_order.resize(task_proxy.get_variables().size());
    iota(var_order.begin(), var_order.end(), 0);
    influence_graph.assign(
        var_order.size(), vector<double>(var_order.size(), 0));

    const causal_graph::CausalGraph &causal_graph =
        task_proxy.get_causal_graph();
    for (size_t var = 0; var < task_proxy.get_variables().size(); ++var) {
        for (int successor : causal_graph.get_successors(var)) {
            if (static_cast<int>(var) != successor) {
                set_influence(var, successor);
            }
        }
    }

    utils::g_log << "Optimizing variable ordering..." << flush;
    utils::Timer timer;
    double best_value = optimize_ordering(var_order, 50000);
    for (int restart = 0; restart < 20; ++restart) {
        if (timer() > timeout) {
            utils::g_log << " Timeout: exceeded " << timeout << " seconds! "
                         << flush;
            break;
        }
        vector<int> candidate = randomize(var_order);
        double candidate_value = optimize_ordering(candidate, 50000);
        if (candidate_value < best_value) {
            best_value = candidate_value;
            var_order.swap(candidate);
        }
    }
    utils::g_log << "done! [t=" << timer << "]" << endl;

    if (reversed) {
        reverse(var_order.begin(), var_order.end());
    }
}

vector<int> GamerOrdering::randomize(const vector<int> &ordering) const {
    vector<int> result = ordering;
    rng->shuffle(result);
    return result;
}

double GamerOrdering::optimize_ordering(
    vector<int> &ordering, int iterations) const {
    double total_distance = compute_function(ordering);
    double best_distance = total_distance;
    for (int iteration = 0; iteration < iterations; ++iteration) {
        int index1 = rng->random(ordering.size());
        int index2 = rng->random(ordering.size());
        if (index1 == index2) {
            continue;
        }

        for (int index = 0; index < static_cast<int>(ordering.size());
             ++index) {
            if (index == index1 || index == index2) {
                continue;
            }
            if (influence(ordering[index], ordering[index1]) != 0.0) {
                total_distance += -(index - index1) * (index - index1) +
                                  (index - index2) * (index - index2);
            }
            if (influence(ordering[index], ordering[index2]) != 0.0) {
                total_distance += -(index - index2) * (index - index2) +
                                  (index - index1) * (index - index1);
            }
        }

        if (total_distance < best_distance) {
            swap(ordering[index1], ordering[index2]);
            best_distance = total_distance;
        } else {
            total_distance = best_distance;
        }
    }
    return best_distance;
}

double GamerOrdering::compute_function(const vector<int> &ordering) const {
    double total_distance = 0;
    for (size_t i = 0; i + 1 < ordering.size(); ++i) {
        for (size_t j = i + 1; j < ordering.size(); ++j) {
            if (influence(ordering[i], ordering[j]) != 0.0) {
                total_distance += (j - i) * (j - i);
            }
        }
    }
    return total_distance;
}

class GamerOrderingFeature : public plugins::TypedFeature<VariableOrdering> {
public:
    GamerOrderingFeature() : TypedFeature("gamer_ordering") {
        document_title("Gamer variable ordering based on the causal graph.");
        VariableOrdering::add_options_to_feature(*this);
        add_option<double>(
            "timeout", "time limit in seconds (checked between iterations)",
            "30.0", plugins::Bounds("0.0", "infinity"));
    }

    shared_ptr<VariableOrdering> create_component(
        const plugins::Options &opts) const override {
        return make_shared<GamerOrdering>(opts);
    }
};

static plugins::FeaturePlugin<GamerOrderingFeature> _plugin;
}