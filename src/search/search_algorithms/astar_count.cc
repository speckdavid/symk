#include "astar_count.h"

#include "search_common.h"

#include "../plugins/plugin.h"


using namespace std;

namespace astar_count {
AStarCount::AStarCount(const shared_ptr<OpenListFactory> &open,
                       bool reopen_closed, const shared_ptr<Evaluator> &f_eval,
                       const vector<shared_ptr<Evaluator>> &preferred,
                       const shared_ptr<PruningMethod> &pruning,
                       const shared_ptr<Evaluator> &lazy_evaluator,
                       OperatorCost cost_type, int bound, double max_time,
                       const string &description, utils::Verbosity verbosity) :
    eager_search::EagerSearch(open, reopen_closed, f_eval, preferred,
                              pruning, lazy_evaluator, cost_type, bound, max_time, description,
                              verbosity) {}

class AStarCountFeature
    : public plugins::TypedFeature<SearchAlgorithm, AStarCount> {
public:
    AStarCountFeature() : TypedFeature("astar_count") {
        document_title("A*-Count search (eager)");
        document_synopsis(
            "A*-Count is a special case of A* search which count the number of solutions. "
            "We break ties using the evaluator.");

        add_option<shared_ptr<Evaluator>>("eval", "evaluator for h-value");
        add_option<shared_ptr<Evaluator>>(
            "lazy_evaluator",
            "An evaluator that re-evaluates a state before it is expanded.",
            plugins::ArgumentInfo::NO_DEFAULT);
        eager_search::add_eager_search_options_to_feature(
            *this, "astar_count");

        document_note(
            "lazy_evaluator",
            "When a state s is taken out of the open list, the lazy evaluator h "
            "re-evaluates s. If h(s) changes (for example because h is path-dependent), "
            "s is not expanded, but instead reinserted into the open list. "
            "This option is currently only present for the A* algorithm.");
    }

    virtual shared_ptr<AStarCount>
    create_component(const plugins::Options &opts) const override {
        plugins::Options options_copy(opts);
        auto temp =
            search_common::create_astar_open_list_factory_and_f_eval(
                opts.get<shared_ptr<Evaluator>>("eval"),
                opts.get<utils::Verbosity>("verbosity"));
        options_copy.set("open", temp.first);
        options_copy.set("f_eval", temp.second);
        options_copy.set("reopen_closed", true);
        vector<shared_ptr<Evaluator>> preferred_list;
        options_copy.set("preferred", preferred_list);
        return plugins::make_shared_from_arg_tuples<astar_count::AStarCount>(
            options_copy.get<shared_ptr<OpenListFactory>>("open"),
            options_copy.get<bool>("reopen_closed"),
            options_copy.get<shared_ptr<Evaluator>>("f_eval", nullptr),
            options_copy.get_list<shared_ptr<Evaluator>>("preferred"),
            eager_search::get_eager_search_arguments_from_options(
                options_copy)
            );
    }
};

static plugins::FeaturePlugin<AStarCountFeature> _plugin;
}
