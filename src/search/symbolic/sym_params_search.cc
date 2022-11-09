#include "sym_params_search.h"

#include "../option_parser.h"
#include "../utils/logging.h"
#include "../utils/timer.h"

using options::Options;
using namespace std;

namespace symbolic {
SymParamsSearch::SymParamsSearch(const Options &opts)
    : maxAllotedTime(opts.get<int>("max_alloted_time")),
      maxAllotedNodes(opts.get<int>("max_alloted_nodes")),
      ratioAllotedTime(opts.get<double>("ratio_alloted_time")),
      ratioAllotedNodes(opts.get<double>("ratio_alloted_nodes")),
      non_stop(opts.get<bool>("non_stop")) {
    maxAllotedNodes = maxAllotedNodes < 0 ? 0 : maxAllotedNodes;
    maxAllotedTime = maxAllotedTime < 0 ? 0 : maxAllotedTime;
}

void SymParamsSearch::increase_bound() {
    maxAllotedNodes *= ratioAllotedNodes;
    if (maxAllotedNodes <= 0)
        maxAllotedNodes = 0;

    maxAllotedTime *= ratioAllotedTime;
    if (maxAllotedTime <= 0)
        maxAllotedTime = 0;
    utils::g_log << "Increase allot limits! "
                 << "Max alloted time: " << maxAllotedTime / 1000
                 << "s nodes: " << maxAllotedNodes << endl;
}

void SymParamsSearch::print_options() const {
    utils::g_log << "Max alloted time (for bd): "
                 << (maxAllotedTime == 0 ? "INF" : to_string(maxAllotedTime / 1000.0) + "s")
                 << " nodes: "
                 << (maxAllotedNodes == 0 ? "INF" : to_string(maxAllotedNodes)) << endl;
    utils::g_log << "Mult alloted time (for bd): " << ratioAllotedTime
                 << " nodes: " << ratioAllotedNodes << endl;
}

void SymParamsSearch::add_options_to_parser(OptionParser &parser) {
    parser.add_option<int>("max_alloted_time", "maximum alloted time for an step",
                           to_string(60000));
    parser.add_option<int>("max_alloted_nodes",
                           "maximum alloted nodes for an step",
                           to_string(10000000));

    parser.add_option<double>("ratio_alloted_time",
                              "multiplier to decide alloted time for a step",
                              "2.0");
    parser.add_option<double>("ratio_alloted_nodes",
                              "multiplier to decide alloted nodes for a step",
                              "2.0");

    parser.add_option<bool>(
        "non_stop",
        "Removes initial state from closed to avoid backward search to stop.",
        "false");
}
}
