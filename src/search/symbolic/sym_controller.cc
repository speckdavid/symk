#include "sym_controller.h"

#include "sym_state_space_manager.h"
#include "opt_order.h"
#include "../option_parser.h"
#include "../task_utils/task_properties.h"


using namespace std;

namespace symbolic {

    SymController::SymController(const Options &opts)
    : vars(make_shared<SymVariables>(opts)), mgrParams(opts),
    searchParams(opts), lower_bound(0),
    solution_registry(mgrParams.num_plans) {

        task_properties::verify_no_axioms(TaskProxy(*tasks::g_root_task));

        mgrParams.print_options();
        searchParams.print_options();
        vars->init();
    }
    
    void SymController::init(UnidirectionalSearch* fwd_search, UnidirectionalSearch* bwd_search) {
        solution_registry.init(vars, fwd_search, bwd_search);
    }

    void SymController::add_options_to_parser(OptionParser &parser, int maxStepTime,
            int maxStepNodes) {
        SymVariables::add_options_to_parser(parser);
        SymParamsMgr::add_options_to_parser(parser);
        SymParamsSearch::add_options_to_parser(parser, maxStepTime, maxStepNodes);
    }

    void SymController::new_solution(const SymSolutionCut &sol) {
        if (!solution_registry.found_all_plans()) {
            solution_registry.register_solution(sol);
        } else {
            lower_bound = std::numeric_limits<int>::max();
        }
    }

    void SymController::setLowerBound(int lower) {
        if (lower > lower_bound) {
            lower_bound = lower;
            std::cout << "BOUND: " << lower_bound << " < " << getUpperBound()
                    << std::flush;
            solution_registry.construct_cheaper_solutions(lower);
            std::cout << " [" << solution_registry.get_num_found_plans() << "/"
                    << mgrParams.num_plans << " plans]" << std::flush;
            std::cout << ", total time: " << utils::g_timer << std::endl;
        }
    }

    BDD SymController::get_states_on_goal_paths() const {
        return solution_registry.get_states_on_goal_paths();
    }
} // namespace symbolic
