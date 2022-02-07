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
    mgr = make_shared<OriginalStateSpace>(vars.get(), mgrParams, search_task);

    unique_ptr<UniformCostSearch> fw_search = nullptr;
    unique_ptr<UniformCostSearch> bw_search = nullptr;

    if (fw) {
        fw_search = unique_ptr<UniformCostSearch>(
            new UniformCostSearch(this, searchParams));
    }

    if (bw) {
        bw_search = unique_ptr<UniformCostSearch>(
            new UniformCostSearch(this, searchParams));
    }

    if (fw) {
        fw_search->init(mgr, true, bw_search.get());
    }

    if (bw) {
        bw_search->init(mgr, false, fw_search.get());
    }

    solution_registry->init(vars, fw_search.get(), bw_search.get(), plan_data_base,
                            true);

    if (fw && bw) {
        search = unique_ptr<BidirectionalSearch>(new BidirectionalSearch(
                                                     this, searchParams, move(fw_search), move(bw_search)));
    } else {
        search.reset(fw ? fw_search.release() : bw_search.release());
    }
}

SymbolicUniformCostSearch::SymbolicUniformCostSearch(
    const options::Options &opts, bool fw, bool bw)
    : SymbolicSearch(opts), fw(fw), bw(bw) {}

void SymbolicUniformCostSearch::new_solution(const SymSolutionCut &sol) {
    if (!solution_registry->found_all_plans() && sol.get_f() < upper_bound) {
        solution_registry->register_solution(sol);
        upper_bound = sol.get_f();
    }
}
} // namespace symbolic

static shared_ptr<SearchEngine> _parse_forward_ucs(OptionParser &parser) {
    parser.document_synopsis("Symbolic Forward Uniform Cost Search", "");
    symbolic::SymbolicSearch::add_options_to_parser(parser);
    parser.add_option<shared_ptr<symbolic::PlanDataBase>>(
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
    parser.add_option<shared_ptr<symbolic::PlanDataBase>>(
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
    parser.add_option<shared_ptr<symbolic::PlanDataBase>>(
        "plan_selection", "plan selection strategy", "top_k(num_plans=1)");
    Options opts = parser.parse();

    shared_ptr<symbolic::SymbolicSearch> engine = nullptr;
    if (!parser.dry_run()) {
        engine =
            make_shared<symbolic::SymbolicUniformCostSearch>(opts, true, true);
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
