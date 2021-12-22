#include "opt_order.h"

#include "../task_proxy.h"
#include "../task_utils/causal_graph.h"
#include "../tasks/root_task.h"
#include "../utils/logging.h"
#include "../utils/timer.h"

#include <ostream>

using namespace std;

namespace symbolic {
// Returns a optimized variable ordering that reorders the variables
// according to the standard causal graph criterion
void InfluenceGraph::compute_gamer_ordering(
    vector<int> &var_order, const shared_ptr<AbstractTask> &task) {
    TaskProxy task_proxy(*task);

    const causal_graph::CausalGraph &cg = task_proxy.get_causal_graph();

    if (var_order.empty()) {
        for (size_t v = 0; v < task_proxy.get_variables().size(); v++) {
            var_order.push_back(v);
        }
    }

    InfluenceGraph ig_partitions(task_proxy.get_variables().size());
    for (size_t v = 0; v < task_proxy.get_variables().size(); v++) {
        for (int v2 : cg.get_successors(v)) {
            if ((int)v != v2) {
                ig_partitions.set_influence(v, v2);
            }
        }
    }

    ig_partitions.get_ordering(var_order);

    // utils::g_log << "Var ordering: ";
    // for(int v : var_order) utils::g_log << v << " ";
    // utils::g_log  << endl;
}

void InfluenceGraph::get_ordering(vector<int> &ordering) const {
    utils::g_log << "Optimizing variable ordering..." << flush;
    utils::Timer timer;
    double value_optimization_function =
        optimize_variable_ordering_gamer(ordering, 50000);

    for (int counter = 0; counter < 20; counter++) {
        vector<int> new_order;
        randomize(ordering, new_order); // Copy the order randomly
        double new_value = optimize_variable_ordering_gamer(new_order, 50000);

        if (new_value < value_optimization_function) {
            value_optimization_function = new_value;
            ordering.swap(new_order);
        }
    }
    utils::g_log << "done!" << " [t=" << timer << "]" << endl;
}

void InfluenceGraph::randomize(vector<int> &ordering,
                               vector<int> &new_order) const {
    for (size_t i = 0; i < ordering.size(); i++) {
        int rnd_pos = (*rng)(ordering.size() - i);
        int pos = -1;
        do {
            pos++;
            bool found;
            do {
                found = false;
                for (size_t j = 0; j < new_order.size(); j++) {
                    if (new_order[j] == ordering[pos]) {
                        found = true;
                        break;
                    }
                }
                if (found)
                    pos++;
            } while (found);
        } while (rnd_pos-- > 0);
        new_order.push_back(ordering[pos]);
    }
}

double InfluenceGraph::optimize_variable_ordering_gamer(vector<int> &order,
                                                        int iterations) const {
    double totalDistance = compute_function(order);

    double oldTotalDistance = totalDistance;
    // Repeat iterations times
    for (int counter = 0; counter < iterations; counter++) {
        // Swap variable
        int swapIndex1 = (*rng)(order.size());
        int swapIndex2 = (*rng)(order.size());
        if (swapIndex1 == swapIndex2)
            continue;

        // Compute the new value of the optimization function
        for (int i = 0; i < int(order.size()); i++) {
            if ((int)i == swapIndex1 || (int)i == swapIndex2)
                continue;

            if (influence(order[i], order[swapIndex1]))
                totalDistance += (-(i - swapIndex1) * (i - swapIndex1) +
                                  (i - swapIndex2) * (i - swapIndex2));

            if (influence(order[i], order[swapIndex2]))
                totalDistance += (-(i - swapIndex2) * (i - swapIndex2) +
                                  (i - swapIndex1) * (i - swapIndex1));
        }

        // Apply the swap if it is worthy
        if (totalDistance < oldTotalDistance) {
            int tmp = order[swapIndex1];
            order[swapIndex1] = order[swapIndex2];
            order[swapIndex2] = tmp;
            oldTotalDistance = totalDistance;

            /*if(totalDistance != compute_function(order)){
              cerr << "Error computing total distance: " << totalDistance << " " <<
            compute_function(order) << endl; exit(-1); }else{ utils::g_log << "Bien: " <<
            totalDistance << endl;
            }*/
        } else {
            totalDistance = oldTotalDistance;
        }
    }
    //  utils::g_log << "Total distance: " << totalDistance << endl;
    return totalDistance;
}

double InfluenceGraph::compute_function(const vector<int> &order) const {
    double totalDistance = 0;
    for (size_t i = 0; i < order.size() - 1; i++) {
        for (size_t j = i + 1; j < order.size(); j++) {
            if (influence(order[i], order[j])) {
                totalDistance += (j - i) * (j - i);
            }
        }
    }
    return totalDistance;
}

InfluenceGraph::InfluenceGraph(int num) {
    // TODO(speckd): we need to randomize the seed here
    rng = make_shared<utils::RandomNumberGenerator>(0);
    influence_graph.resize(num);
    for (auto &i : influence_graph) {
        i.resize(num, 0);
    }
}

void InfluenceGraph::optimize_variable_ordering_gamer(
    vector<int> &order, vector<int> &partition_begin,
    vector<int> &partition_sizes, int iterations) const {
    double totalDistance = compute_function(order);

    double oldTotalDistance = totalDistance;
    // Repeat iterations times
    for (int counter = 0; counter < iterations; counter++) {
        // Swap variable
        int partition = (*rng)(partition_begin.size());
        if (partition_sizes[partition] <= 1)
            continue;
        int swapIndex1 =
            partition_begin[partition] + (*rng)(partition_sizes[partition]);
        int swapIndex2 =
            partition_begin[partition] + (*rng)(partition_sizes[partition]);
        if (swapIndex1 == swapIndex2)
            continue;

        // Compute the new value of the optimization function
        for (int i = 0; i < int(order.size()); i++) {
            if ((int)i == swapIndex1 || (int)i == swapIndex2)
                continue;

            if (influence(order[i], order[swapIndex1]))
                totalDistance += (-(i - swapIndex1) * (i - swapIndex1) +
                                  (i - swapIndex2) * (i - swapIndex2));

            if (influence(order[i], order[swapIndex2]))
                totalDistance += (-(i - swapIndex2) * (i - swapIndex2) +
                                  (i - swapIndex1) * (i - swapIndex1));
        }

        // Apply the swap if it is worthy
        if (totalDistance < oldTotalDistance) {
            int tmp = order[swapIndex1];
            order[swapIndex1] = order[swapIndex2];
            order[swapIndex2] = tmp;
            oldTotalDistance = totalDistance;

            /*if(totalDistance != compute_function(order)){
              cerr << "Error computing total distance: " << totalDistance << " " <<
            compute_function(order) << endl; exit(-1); }else{ utils::g_log << "Bien: " <<
            totalDistance << endl;
            }*/
        } else {
            totalDistance = oldTotalDistance;
        }
    }
    //  utils::g_log << "Total distance: " << totalDistance << endl;
}
} // namespace symbolic
