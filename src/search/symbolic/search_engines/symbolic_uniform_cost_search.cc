#include "symbolic_uniform_cost_search.h"

#include "../original_state_space.h"
#include "../plugin.h"
#include "../searches/bidirectional_search.h"
#include "../searches/uniform_cost_search.h"
#include "../../option_parser.h"

using namespace std;

namespace symbolic {
void SymbolicUniformCostSearch::initialize() {
    SymbolicSearch::initialize();
    mgr = make_shared<OriginalStateSpace>(vars.get(), sym_params, search_task);

    unique_ptr<UniformCostSearch> fw_search = nullptr;
    unique_ptr<UniformCostSearch> bw_search = nullptr;

    if (fw) {
        fw_search = unique_ptr<UniformCostSearch>(
            new UniformCostSearch(this, sym_params));
    }

    if (bw) {
        bw_search = unique_ptr<UniformCostSearch>(
            new UniformCostSearch(this, sym_params));
    }

    if (fw) {
        fw_search->init(mgr, true, bw_search.get());
    }

    if (bw) {
        bw_search->init(mgr, false, fw_search.get());
    }

    auto individual_trs = fw ? fw_search->getStateSpaceShared()->getIndividualTRs() :  bw_search->getStateSpaceShared()->getIndividualTRs();

    solution_registry->init(vars,
                            fw_search ? fw_search->getClosedShared() : nullptr,
                            bw_search ? bw_search->getClosedShared() : nullptr,
                            individual_trs,
                            plan_data_base,
                            single_solution,
                            simple);

    if (fw && bw) {
        search = unique_ptr<BidirectionalSearch>(new BidirectionalSearch(
                                                     this, sym_params, move(fw_search), move(bw_search), alternating));
    } else {
        search.reset(fw ? fw_search.release() : bw_search.release());
    }
}

SymbolicUniformCostSearch::SymbolicUniformCostSearch(
    const options::Options &opts, bool fw, bool bw, bool alternating)
    : SymbolicSearch(opts), fw(fw), bw(bw), alternating(alternating) {}

void SymbolicUniformCostSearch::new_solution(const SymSolutionCut &sol) {
    if (!solution_registry->found_all_plans() && sol.get_f() < upper_bound) {
        solution_registry->register_solution(sol);
        upper_bound = sol.get_f();
    }
}
}

static shared_ptr<SearchEngine> _parse_forward_ucs(OptionParser &parser) {
    parser.document_synopsis("Symbolic Forward Uniform Cost Search", "");
    symbolic::SymbolicSearch::add_options_to_parser(parser);
    parser.add_option<shared_ptr<symbolic::PlanSelector>>(
        "plan_selection", "plan selection strategy", "top_k(num_plans=1)");
    Options opts = parser.parse();

    shared_ptr<symbolic::SymbolicSearch> engine = nullptr;
    if (!parser.dry_run()) {
        engine = make_shared<symbolic::SymbolicUniformCostSearch>(opts, true,
                                                                  false);
        utils::g_log << "Symbolic Forward Uniform Cost Search" << endl;
    }

    return engine;
}

static shared_ptr<SearchEngine> _parse_backward_ucs(OptionParser &parser) {
    parser.document_synopsis("Symbolic Backward Uniform Cost Search", "");
    symbolic::SymbolicSearch::add_options_to_parser(parser);
    parser.add_option<shared_ptr<symbolic::PlanSelector>>(
        "plan_selection", "plan selection strategy", "top_k(num_plans=1)");
    Options opts = parser.parse();

    shared_ptr<symbolic::SymbolicSearch> engine = nullptr;
    if (!parser.dry_run()) {
        engine = make_shared<symbolic::SymbolicUniformCostSearch>(opts, false,
                                                                  true);
        utils::g_log << "Symbolic Backward Uniform Cost Search" << endl;
    }

    return engine;
}

static shared_ptr<SearchEngine>
_parse_bidirectional_ucs(OptionParser &parser) {
    parser.document_synopsis("Symbolic Bidirectional Uniform Cost Search", "");
    symbolic::SymbolicSearch::add_options_to_parser(parser);
    parser.add_option<shared_ptr<symbolic::PlanSelector>>(
        "plan_selection", "plan selection strategy", "top_k(num_plans=1)");
    parser.add_option<bool>("alternating", "alternating", "false");
    Options opts = parser.parse();

    shared_ptr<symbolic::SymbolicSearch> engine = nullptr;
    if (!parser.dry_run()) {
        bool alternating = opts.get<bool>("alternating");
        engine =
            make_shared<symbolic::SymbolicUniformCostSearch>(opts, true, true, alternating);
        utils::g_log << "Symbolic Bidirectional Uniform Cost Search" << endl;
    }

    return engine;
}

static Plugin<SearchEngine> _plugin_sym_fw_ordinary("sym-fw",
                                                    _parse_forward_ucs);
static Plugin<SearchEngine> _plugin_sym_bw_ordinary("sym-bw",
                                                    _parse_backward_ucs);
static Plugin<SearchEngine> _plugin_sym_bd_ordinary("sym-bd",
                                                    _parse_bidirectional_ucs);
