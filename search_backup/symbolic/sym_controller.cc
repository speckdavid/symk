#include "sym_controller.h"

#include "opt_order.h"

#include "sym_state_space_manager.h"
#include "debug_macros.h"
#include "../option_parser.h"

using namespace std;

namespace symbolic
{
SymController::SymController(const Options &opts)
    : vars(make_shared<SymVariables>(opts)),
      mgrParams(opts), searchParams(opts), lower_bound(0)
{
    mgrParams.print_options();
    searchParams.print_options();
    vars->init();
}

void SymController::add_options_to_parser(OptionParser &parser, int maxStepTime, int maxStepNodes)
{
    SymVariables::add_options_to_parser(parser);
    SymParamsMgr::add_options_to_parser(parser);
    SymParamsSearch::add_options_to_parser(parser, maxStepTime, maxStepNodes);
}
void SymController::new_solution(const SymSolution &sol)
{
    if (!solution.solved() ||
        sol.getCost() < solution.getCost())
    {
        solution = sol;
        std::cout << "BOUND: " << lower_bound << " < " << getUpperBound()
                  << ", total time: " << utils::g_timer << std::endl;
    }
}

void SymController::setLowerBound(int lower)
{
    //Never set a lower bound greater than the current upper bound
    if (solution.solved())
    {
        lower = min(lower, solution.getCost());
    }

    if (lower > lower_bound)
    {
        lower_bound = lower;

        std::cout << "BOUND: " << lower_bound << " < " << getUpperBound()
                  << ", total time: " << utils::g_timer << std::endl;
    }
}
} // namespace symbolic