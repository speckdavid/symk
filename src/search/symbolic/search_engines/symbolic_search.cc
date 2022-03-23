#include "symbolic_search.h"

#include "../option_parser.h"
#include "../plugin.h"

#include "../original_state_space.h"
#include "../plan_reconstruction/simple_sym_solution_registry.h"
#include "../plan_selection/plan_database.h"
#include "../searches/bidirectional_search.h"
#include "../searches/top_k_uniform_cost_search.h"
#include "../searches/uniform_cost_search.h"

#include "../sym_params_search.h"
#include "../sym_state_space_manager.h"
#include "../sym_variables.h"

#include "../../task_utils/task_properties.h"
#include "../../tasks/sdac_task.h"

using namespace std;
using namespace options;

namespace symbolic {
SymbolicSearch::SymbolicSearch(const options::Options &opts)
    : SearchEngine(opts),
      task(opts.get<shared_ptr<AbstractTask>>("transform")),
      search_task(task),
      task_proxy(*task),
      vars(make_shared<SymVariables>(opts, task)),
      mgrParams(opts, task),
      searchParams(opts), step_num(-1),
      lower_bound_increased(true),
      lower_bound(0),
      upper_bound(numeric_limits<int>::max()),
      min_g(0),
      plan_data_base(opts.get<shared_ptr<PlanDataBase>>("plan_selection")),
      solution_registry(make_shared<SymSolutionRegistry>()),
      simple(opts.get<bool>("simple")) {
    cout << endl;
    mgrParams.print_options();
    cout << endl;
    searchParams.print_options();
    cout << endl;
    vars->init();
    cout << endl;
}

void SymbolicSearch::initialize() {
    plan_data_base->print_options();
    cout << endl;

    if (task_properties::has_sdac_cost_operator(task_proxy)) {
        utils::g_log << "Creating sdac task..." << endl;
        search_task = make_shared<extra_tasks::SdacTask>(task, vars.get());
        utils::g_log << "#Operators with sdac: " << task->get_num_operators() << endl;
        utils::g_log << "#Operators without sdac: " << search_task->get_num_operators() << endl;
        cout << endl;
    }

    if (simple) {
        // compute upper cost bound
        double num_states = 1;
        for (int var = 0; var < task->get_num_variables(); var++) {
            num_states *= task->get_variable_domain_size(var);
        }
        double max_plan_cost =
            (num_states - 1) *
            task_properties::get_max_operator_cost(task_proxy);

        utils::g_log << "Plan Reconstruction: Simple (without loops)" << endl;
        utils::g_log << "Maximal plan cost: " << max_plan_cost << endl;
        upper_bound = min((double)upper_bound, max_plan_cost + 1);
        solution_registry = make_shared<SimpleSymSolutionRegistry>();
        cout << endl;
    }

    plan_data_base->init(vars, search_task, get_plan_manager());
}

SearchStatus SymbolicSearch::step() {
    step_num++;

    // Handling empty plan
    if (step_num == 0) {
        BDD cut = mgr->getInitialState() * mgr->getGoal();
        if (!cut.IsZero()) {
            new_solution(SymSolutionCut(0, 0, cut, 0));
        }
    }

    SearchStatus cur_status;

    // Search finished!
    if (lower_bound >= upper_bound) {
        solution_registry->construct_cheaper_solutions(
            numeric_limits<int>::max());
        solution_found = plan_data_base->get_num_reported_plan() > 0;
        cur_status = solution_found ? SOLVED : FAILED;
    } else {
        // Bound increade => construct plans
        if (lower_bound_increased) {
            solution_registry->construct_cheaper_solutions(lower_bound);
        }

        // All plans found
        if (solution_registry->found_all_plans()) {
            solution_found = true;
            cur_status = SOLVED;
        } else {
            cur_status = IN_PROGRESS;
        }
    }

    if (lower_bound_increased) {
        utils::g_log << "BOUND: " << lower_bound << " < " << upper_bound << flush;

        utils::g_log << " [" << solution_registry->get_num_found_plans() << "/"
                     << plan_data_base->get_num_desired_plans() << " plans]"
                     << flush;
        utils::g_log << ", total time: " << utils::g_timer << endl;
    }
    lower_bound_increased = false;

    if (cur_status == SOLVED) {
        set_plan(plan_data_base->get_first_accepted_plan());
        cout << endl;
        return cur_status;
    }
    if (cur_status == FAILED) {
        return cur_status;
    }

    // Actuall step
    search->step();

    return cur_status;
}

void SymbolicSearch::setLowerBound(int lower) {
    if (lower > lower_bound) {
        lower_bound_increased = true;
    }
    lower_bound = max(lower_bound, lower);
}

void SymbolicSearch::new_solution(const SymSolutionCut &sol) {
    if (!solution_registry->found_all_plans()) {
        solution_registry->register_solution(sol);
        upper_bound = min(upper_bound, sol.get_f());
    }
}

void SymbolicSearch::save_plan_if_necessary() {
    if (found_solution()) {
        utils::g_log << "Best plan:" << endl;
        if (task_properties::has_sdac_cost_operator(task_proxy)) {
            plan_manager.dump_plan(get_plan(), TaskProxy(*search_task));
        } else {
            plan_manager.dump_plan(get_plan(), task_proxy);
        }
    }
}

void SymbolicSearch::add_options_to_parser(OptionParser &parser) {
    parser.add_option<shared_ptr<AbstractTask>>(
        "transform",
        "Optional task transformation for the search."
        " Currently, adapt_costs() and no_transform() are available.",
        "no_transform()");
    SearchEngine::add_options_to_parser(parser);
    SymVariables::add_options_to_parser(parser);
    SymParamsSearch::add_options_to_parser(parser, 30e3, 10e7);
    SymParamsMgr::add_options_to_parser(parser);
    PlanDataBase::add_options_to_parser(parser);
    parser.add_option<bool>("simple", "simple/loopless plan construction",
                            "false");
}
} // namespace symbolic
