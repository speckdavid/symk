#ifndef SYMBOLIC_SYM_PARAMS_SEARCH_H
#define SYMBOLIC_SYM_PARAMS_SEARCH_H

#include <algorithm>

namespace options {
class Options;
class OptionParser;
} // namespace options

namespace symbolic {
class SymParamsSearch {
public:
    // By default max<int>. If lower, it allows for skip disjunction if the size
    // of S is greater.
    int max_disj_nodes;

    // Parameters for sym_step_cost_estimation
    double min_estimation_time;       // Dont estimate if time is lower
    double penalty_time_estimation_sum; // violated_time = sum + time*mult
    double penalty_time_estimation_mult;
    double penalty_nodes_estimation_sum; // violated_nodes = sum + nodes*mult
    double penalty_nodes_estimation_mult;

    // Parameters to control isUseful() and isSearchable()
    int maxStepTime, maxStepNodes;

    // Allows to scale maxStepNodes with planning time (starts at 100*x
    // during 100s and then grows at a rate of 1x)
    int maxStepNodesPerPlanningSecond, maxStepNodesMin,
        maxStepNodesTimeStartIncrement;
    double ratioUseful;

    // Parameters to decide the alloted time for a step
    // alloted = max(minAlloted, estimated*multAlloted)
    int minAllotedTime, minAllotedNodes; // min alloted time and nodes to a step
    int maxAllotedTime, maxAllotedNodes; // min alloted time and nodes to a step
    double ratioAllotedTime,
           ratioAllotedNodes; // factor to multiply the estimation

    double ratioAfterRelax;

    bool non_stop;

    bool debug;

    SymParamsSearch(const options::Options &opts);

    static void add_options_to_parser(options::OptionParser &parser,
                                      int maxStepTime, int maxStepNodes);
    // Parameters with default values for hierarchy policies
    static void add_options_to_parser_abstractions(options::OptionParser &parser,
                                                   int maxStepTime,
                                                   int maxStepNodes);
    void print_options() const;

    inline double getAllotedTime(double estimatedTime) const {
        return std::min(
            maxAllotedTime,
            std::max<int>(estimatedTime * ratioAllotedTime, minAllotedTime));
    }

    inline double getAllotedNodes(double estimatedNodes) const {
        return std::min(
            maxAllotedNodes,
            std::max<int>(estimatedNodes * ratioAllotedNodes, minAllotedNodes));
    }

    void inheritParentParams(const SymParamsSearch &other) {
        maxStepTime = std::min(maxStepTime, other.maxStepTime);
        maxStepNodes = std::min(maxStepNodes, other.maxStepNodes);
    }

    int getMaxStepNodes() const;

    inline bool get_non_stop() const {return non_stop;}
};
}

#endif
