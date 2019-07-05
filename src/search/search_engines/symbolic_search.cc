#include "symbolic_search.h"

#include "../option_parser.h"
#include "../plugin.h"

#include "../symbolic/bidirectional_search.h"
#include "../symbolic/original_state_space.h"
#include "../symbolic/sym_params_search.h"
#include "../symbolic/sym_state_space_manager.h"
#include "../symbolic/sym_variables.h"
#include "../symbolic/uniform_cost_search.h"

#include "../task_utils/task_properties.h"

using namespace std;
using namespace symbolic;
using namespace options;

namespace symbolic_search
{

bool only_zero_cost_actions() {
    return task_properties::get_max_operator_cost(TaskProxy(*tasks::g_root_task)) == 0;
}

SymbolicSearch::SymbolicSearch(const options::Options &opts)
    : SearchEngine(opts), SymController(opts) {}

SymbolicBidirectionalUniformCostSearch::SymbolicBidirectionalUniformCostSearch(
    const options::Options &opts)
    : SymbolicSearch(opts) {}

void SymbolicBidirectionalUniformCostSearch::initialize()
{
    mgr = shared_ptr<OriginalStateSpace>(
        new OriginalStateSpace(vars.get(), mgrParams, only_zero_cost_actions()));
    auto fw_search =
        unique_ptr<UniformCostSearch>(new UniformCostSearch(this, searchParams));
    auto bw_search =
        unique_ptr<UniformCostSearch>(new UniformCostSearch(this, searchParams));
    fw_search->init(mgr, true, bw_search->getClosedShared());
    bw_search->init(mgr, false, fw_search->getClosedShared());

    SymController::init(fw_search.get(), bw_search.get());
    
    search = unique_ptr<BidirectionalSearch>(new BidirectionalSearch(
        this, searchParams, move(fw_search), move(bw_search)));
    
}

SymbolicUniformCostSearch::SymbolicUniformCostSearch(
    const options::Options &opts, bool _fw)
    : SymbolicSearch(opts), fw(_fw) {}

void SymbolicUniformCostSearch::initialize()
{
    mgr = make_shared<OriginalStateSpace>(vars.get(), mgrParams, only_zero_cost_actions());
    auto uni_search =
        unique_ptr<UniformCostSearch>(new UniformCostSearch(this, searchParams));
    if (fw)
    {
        uni_search->init(mgr, true, nullptr);
        SymController::init(uni_search.get(),  nullptr);
    }
    else
    {
        uni_search->init(mgr, false, nullptr);
        SymController::init(nullptr,  uni_search.get());
    }

    search.reset(uni_search.release());
}

SearchStatus SymbolicSearch::step()
{
    search->step();

    if (getLowerBound() < getUpperBound())
    {
        return IN_PROGRESS;
    }
    else if (!SymController::get_states_on_goal_paths().IsZero())
    {
        solution_found = true;
        return SOLVED;
    }
    else
    {
        return FAILED;
    }
}

void SymbolicSearch::new_solution(const SymSolutionCut &sol)
{
    SymController::new_solution(sol);
}
} // namespace symbolic_search

static shared_ptr<SearchEngine> _parse_bidirectional_ucs(OptionParser &parser)
{
    parser.document_synopsis("Symbolic Bidirectional Uniform Cost Search", "");

    SearchEngine::add_options_to_parser(parser);
    SymVariables::add_options_to_parser(parser);
    SymParamsSearch::add_options_to_parser(parser, 30e3, 10e7);
    SymParamsMgr::add_options_to_parser(parser);

    Options opts = parser.parse();

    shared_ptr<symbolic_search::SymbolicSearch> engine = nullptr;
    if (!parser.dry_run())
    {
        engine =
            make_shared<symbolic_search::SymbolicBidirectionalUniformCostSearch>(
                opts);
    }

    return engine;
}

static shared_ptr<SearchEngine> _parse_forward_ucs(OptionParser &parser)
{
    parser.document_synopsis("Symbolic Bidirectional Uniform Cost Search", "");

    SearchEngine::add_options_to_parser(parser);
    SymVariables::add_options_to_parser(parser);
    SymParamsSearch::add_options_to_parser(parser, 30e3, 10e7);
    SymParamsMgr::add_options_to_parser(parser);

    Options opts = parser.parse();

    shared_ptr<symbolic_search::SymbolicSearch> engine = nullptr;
    if (!parser.dry_run())
    {
        engine =
            make_shared<symbolic_search::SymbolicUniformCostSearch>(opts, true);
    }

    return engine;
}

static shared_ptr<SearchEngine> _parse_backward_ucs(OptionParser &parser)
{
    parser.document_synopsis("Symbolic Bidirectional Uniform Cost Search", "");

    SearchEngine::add_options_to_parser(parser);
    SymVariables::add_options_to_parser(parser);
    SymParamsSearch::add_options_to_parser(parser, 30e3, 10e7);
    SymParamsMgr::add_options_to_parser(parser);

    Options opts = parser.parse();

    shared_ptr<symbolic_search::SymbolicSearch> engine = nullptr;
    if (!parser.dry_run())
    {
        engine =
            make_shared<symbolic_search::SymbolicUniformCostSearch>(opts, false);
    }

    return engine;
}

static Plugin<SearchEngine> _plugin_bd("symk-bd", _parse_bidirectional_ucs);
static Plugin<SearchEngine> _plugin_fw("symk-fw", _parse_forward_ucs);
static Plugin<SearchEngine> _plugin_bw("symk-bw", _parse_backward_ucs);
