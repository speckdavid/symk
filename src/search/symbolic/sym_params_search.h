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

    int maxAllotedTime, maxAllotedNodes; // max alloted time and nodes to a step
    double ratioAllotedTime, ratioAllotedNodes; // factor to multiply the estimation

    bool non_stop;

    SymParamsSearch(const options::Options &opts);

    void increase_bound();

    static void add_options_to_parser(options::OptionParser &parser);

    void print_options() const;

    inline bool get_non_stop() const {return non_stop;}
};
}

#endif
