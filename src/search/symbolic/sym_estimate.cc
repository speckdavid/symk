#include "sym_estimate.h"

#include <algorithm>
#include <iostream>

#include "sym_params_search.h"
#include "sym_utils.h"
#include "../utils/logging.h"

using namespace std;

namespace symbolic {
SymStepCostEstimation::SymStepCostEstimation(const SymParamsSearch &p)
    : param_min_estimation_time(p.min_estimation_time),
      param_penalty_time_estimation_sum(p.penalty_time_estimation_sum),
      param_penalty_time_estimation_mult(p.penalty_time_estimation_mult),
      param_penalty_nodes_estimation_sum(p.penalty_nodes_estimation_sum),
      param_penalty_nodes_estimation_mult(p.penalty_nodes_estimation_mult),
      nextStepNodes(1) {
    // Initialize the first data points (useful for interpolation)
    data[0] = Estimation(1, 1);
    data[1] = Estimation(1, 1);
}

void SymStepCostEstimation::update_data(long key, Estimation est) {
    if (!data.count(key)) {
        // Ensure consistency with lower estimations, we do not store the data in
        // case is smaller
        auto it = data.lower_bound(key);
        it--; // Guaranteed to exist because we initialize data[0] and data[1]
        est.time = max(est.time, it->second.time);
        est.nodes = max(est.nodes, it->second.nodes);
    } else if (est.time < data[key].time && est.nodes < data[key].nodes) {
        return; // Also skip in case that the new value is smaller than the previous
    } else {
        est.time = max(est.time, data[key].time);
        est.nodes = max(est.nodes, data[key].nodes);
    }

    data[key] = est;

    // Ensure consistency with greater estimations
    for (auto it = data.upper_bound(nextStepNodes); it != data.end(); ++it) {
        if (it->second.time < est.time) {
            it->second.time = est.time;
        }
        if (it->second.nodes < est.nodes) {
            it->second.nodes = est.nodes;
        }
    }
}

void SymStepCostEstimation::stepTaken(double time, double nodes) {
#ifdef DEBUG_ESTIMATES
    utils::g_log << "== STEP TAKEN: " << time << ", " << nodes << endl;
#endif
    update_data(
        nextStepNodes,
        Estimation(time + 10,
                   nodes)); // consider 10ms more to avoid values close to 0
}

// Sets the nodes of next iteration and recalculate estimations
void SymStepCostEstimation::nextStep(double nodes) {
#ifdef DEBUG_ESTIMATES
    utils::g_log << "== NEXT STEP: " << nodes << " " << *this << " to ";
#endif
    nextStepNodes = nodes;

    if (data.count(nodes)) {
        return; // We already have an estimation in our data :D
    }

    double estimatedTime, estimatedNodes;
    // Get next data point
    auto nextIt = data.upper_bound(nextStepNodes);
    if (nextIt == data.end()) {
        // This is greater than any est we have, just get the greatest
        --nextIt;
        long prevNodes = nextIt->first;
        Estimation prevEst = nextIt->second;

        if (prevNodes <= param_min_estimation_time) {
            estimatedTime = prevEst.time;
            estimatedNodes = prevEst.nodes;
        } else {
            double incrementNodes = max(0.0, nextStepNodes - prevEst.nodes);
            estimatedNodes = nextStepNodes + incrementNodes;

            double proportionNodes =
                ((double)estimatedNodes) / ((double)nextStepNodes);
            estimatedTime = prevEst.time * proportionNodes;
        }
    } else {
        long nextNodes = nextIt->first;
        Estimation nextEst = nextIt->second;
        --nextIt; // Guaranteed to exist (because we have initialized data[0] and
                  // data[1])
        long prevNodes = nextIt->first;
        Estimation prevEst = nextIt->second;

        // Interpolate
        double percentage = ((double)(nextStepNodes - prevNodes)) /
            ((double)(nextNodes - prevNodes));
        estimatedTime = prevEst.time + percentage * (nextEst.time - prevEst.time);
        estimatedNodes =
            prevEst.nodes + percentage * (nextEst.nodes - prevEst.nodes);
    }

    estimation = Estimation(estimatedTime, estimatedNodes);
#ifdef DEBUG_ESTIMATES
    utils::g_log << *this << endl;
    if (this->nodes() <= 0) {
        utils::g_log << "ERROR: estimated nodes is lower than 0 after nextStep" << endl;
        utils::exit_with(utils::ExitCode::CRITICAL_ERROR);
    }
#endif
}

void SymStepCostEstimation::violated(double time_ellapsed, double time_limit,
                                     double node_limit) {
#ifdef DEBUG_ESTIMATES
    utils::g_log << "== VIOLATED " << *this << ": " << time_ellapsed << " " << time_limit
                 << " " << node_limit << ", ";
#endif
    estimation.time = param_penalty_time_estimation_sum +
        max<double>(estimation.time, time_ellapsed) *
        param_penalty_time_estimation_mult;

    estimation.nodes = nodes();
    if (time_ellapsed < time_limit) {
        estimation.nodes =
            param_penalty_nodes_estimation_sum +
            max(estimation.nodes, node_limit) * param_penalty_nodes_estimation_mult;
    }

    update_data(nextStepNodes, estimation);
#ifdef DEBUG_ESTIMATES
    utils::g_log << *this << endl;
#endif
}

void SymStepCostEstimation::recalculate(const SymStepCostEstimation &o,
                                        long nodes) {
    nextStepNodes = nodes;
    double proportion = (double)nextStepNodes / (double)(o.nextStepNodes);
    estimation = Estimation(o.time() * proportion, o.nodes() * proportion);
    update_data(nodes, estimation);
}

ostream &operator<<(ostream &os, const SymStepCostEstimation &est) {
    return os << est.nextStepNodes << " nodes => (" << est.time() << " ms, "
              << est.nodes() << " nodes)";
}

void SymStepCostEstimation::write(ofstream &file) const {
    file << nextStepNodes << " => " << estimation << endl;
    for (const auto &d : data) {
        file << d.first << " => " << d.second << endl;
    }
    file << endl;
}

void SymStepCostEstimation::read(ifstream &file) {
    string line;
    getline(file, line);
    utils::g_log << "ESTIMATE PARSE: " << line << endl;
    nextStepNodes = getData<long>(line, "", "=");
    estimation = Estimation(getData<double>(line, ">", ","),
                            getData<double>(line, ",", ""));
    while (getline(file, line) && !line.empty()) {
        utils::g_log << "ESTIMATE PARSE: " << line << endl;
        data[getData<long>(line, "", "=")] = Estimation(
            getData<double>(line, ">", ","), getData<double>(line, ",", ""));
    }
}

ostream &operator<<(ostream &os, const Estimation &est) {
    return os << est.time << ", " << est.nodes;
}
}
