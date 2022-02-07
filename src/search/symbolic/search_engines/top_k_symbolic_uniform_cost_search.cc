#include "top_k_symbolic_uniform_cost_search.h"

#include "../original_state_space.h"
#include "../plugin.h"
#include "../searches/bidirectional_search.h"
#include "../searches/top_k_uniform_cost_search.h"
#include "../../option_parser.h"

#include <memory>

using namespace std;

namespace symbolic {
void TopkSymbolicUniformCostSearch::initialize() {
    SymbolicSearch::initialize();

    mgr = make_shared<OriginalStateSpace>(vars.get(), mgrParams, search_task);

    unique_ptr<TopkUniformCostSearch> fw_search = nullptr;
    unique_ptr<TopkUniformCostSearch> bw_search = nullptr;

    if (fw) {
        fw_search = unique_ptr<TopkUniformCostSearch>(
            new TopkUniformCostSearch(this, searchParams));
    }

    if (bw) {
        bw_search = unique_ptr<TopkUniformCostSearch>(
            new TopkUniformCostSearch(this, searchParams));
    }

    if (fw) {
        fw_search->init(mgr, true, bw_search.get());
    }

    if (bw) {
        bw_search->init(mgr, false, fw_search.get());
    }

    solution_registry->init(vars, fw_search.get(), bw_search.get(), plan_data_base,
                            false);

    if (fw && bw) {
        search = unique_ptr<BidirectionalSearch>(new BidirectionalSearch(
                                                     this, searchParams, move(fw_search), move(bw_search)));
    } else {
        search.reset(fw ? fw_search.release() : bw_search.release());
    }
}

TopkSymbolicUniformCostSearch::TopkSymbolicUniformCostSearch(
    const options::Options &opts, bool fw, bool bw)
    : SymbolicUniformCostSearch(opts, fw, bw) {}

void TopkSymbolicUniformCostSearch::new_solution(const SymSolutionCut &sol) {
    if (!solution_registry->found_all_plans()) {
        solution_registry->register_solution(sol);
    } else {
        lower_bound = numeric_limits<int>::max();
    }
}
}

static shared_ptr<SearchEngine> _parse_forward_ucs(OptionParser &parser) {
    parser.document_synopsis("Top-k Symbolic Forward Uniform Cost Search", "");
    symbolic::SymbolicSearch::add_options_to_parser(parser);
    parser.add_option<shared_ptr<symbolic::PlanDataBase>>(
        "plan_selection", "plan selection strategy");
    Options opts = parser.parse();

    shared_ptr<symbolic::SymbolicSearch> engine = nullptr;
    if (!parser.dry_run()) {
        engine = make_shared<symbolic::TopkSymbolicUniformCostSearch>(
            opts, true, false);
        utils::g_log << "Top-k Symbolic Forward Uniform Cost Search" << endl;
    }

    return engine;
}

static shared_ptr<SearchEngine> _parse_backward_ucs(OptionParser &parser) {
    parser.document_synopsis("Top-k Symbolic Backward Uniform Cost Search", "");
    symbolic::SymbolicSearch::add_options_to_parser(parser);
    parser.add_option<shared_ptr<symbolic::PlanDataBase>>(
        "plan_selection", "plan selection strategy");
    Options opts = parser.parse();

    shared_ptr<symbolic::SymbolicSearch> engine = nullptr;
    if (!parser.dry_run()) {
        engine = make_shared<symbolic::TopkSymbolicUniformCostSearch>(
            opts, false, true);
        utils::g_log << "Top-k Symbolic Backward Uniform Cost Search" << endl;
    }

    return engine;
}

static shared_ptr<SearchEngine>
_parse_bidirectional_ucs(OptionParser &parser) {
    parser.document_synopsis("Top-k Symbolic Bidirectional Uniform Cost Search",
                             "");
    symbolic::SymbolicSearch::add_options_to_parser(parser);
    parser.add_option<shared_ptr<symbolic::PlanDataBase>>(
        "plan_selection", "plan selection strategy");
    Options opts = parser.parse();

    shared_ptr<symbolic::SymbolicSearch> engine = nullptr;
    if (!parser.dry_run()) {
        engine = make_shared<symbolic::TopkSymbolicUniformCostSearch>(
            opts, true, true);
        utils::g_log << "Top-k Symbolic Bidirectional Uniform Cost Search"
                     << endl;
    }

    return engine;
}

static Plugin<SearchEngine> _plugin_sym_fw_top_k("symk-fw", _parse_forward_ucs);
static Plugin<SearchEngine> _plugin_sym_bw_top_k("symk-bw",
                                                 _parse_backward_ucs);
static Plugin<SearchEngine> _plugin_sym_bd_top_k("symk-bd",
                                                 _parse_bidirectional_ucs);
