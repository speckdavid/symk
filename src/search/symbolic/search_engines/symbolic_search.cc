#include "symbolic_search.h"

#include "../sym_state_space_manager.h"
#include "../sym_variables.h"

#include "../plan_selection/plan_selector.h"
#include "../searches/bidirectional_search.h"
#include "../searches/top_k_uniform_cost_search.h"
#include "../searches/uniform_cost_search.h"

#include "../../task_utils/task_properties.h"
#include "../../tasks/sdac_task.h"

using namespace std;
using namespace options;

namespace symbolic {
SymbolicSearch::SymbolicSearch(const plugins::Options &opts)
    : SearchAlgorithm(opts),
      task(opts.get<shared_ptr<AbstractTask>>("transform")),
      search_task(task),
      task_proxy(*task),
      vars(make_shared<SymVariables>(opts, task)),
      sym_params(opts, task),
      step_num(-1),
      lower_bound_increased(true),
      lower_bound(0),
      upper_bound(bound),
      min_g(0),
      plan_data_base(opts.get<shared_ptr<PlanSelector>>("plan_selection")),
      solution_registry(make_shared<SymSolutionRegistry>()),
      simple(opts.get<bool>("simple")),
      single_solution(opts.get<bool>("single_solution")),
      silent(opts.get<bool>("silent")) {
    cout << endl;
    vars->print_options();
    cout << endl;
    sym_params.print_options();
    cout << endl;
    vars->init();
    cout << endl;
}

void SymbolicSearch::initialize() {
    plan_data_base->print_options();
    cout << endl;

    if (has_sdac_cost) {
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
        upper_bound = static_cast<int>(min((double)upper_bound, max_plan_cost + 1));
        utils::g_log << "Maximal plan cost: " << upper_bound << endl;
        cout << endl;
    }

    plan_data_base->init(vars, search_task, get_plan_manager());
}

SearchStatus SymbolicSearch::step() {
    step_num++;

    // Handling empty plan
    if (step_num == 0) {
        BDD cut = mgr->get_initial_state() * mgr->get_goal();
        if (!cut.IsZero()) {
            new_solution(SymSolutionCut(0, 0, cut));
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
        // Bound increased => construct plans
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

    if (lower_bound_increased && !silent) {
        utils::g_log << "BOUND: " << lower_bound << " < " << upper_bound << flush;

        utils::g_log << " [" << solution_registry->get_num_found_plans() << "/"
                     << plan_data_base->get_num_desired_plans() << " plans]"
                     << flush;
        if (step_num > 0) {
            utils::g_log << ", dir: " << search->get_last_dir() << flush;
        }
        utils::g_log << ", total time: " << utils::g_timer << ", " << flush;
        utils::g_log << endl;
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

void SymbolicSearch::add_options_to_feature(plugins::Feature &feature) {
    feature.add_option<shared_ptr<AbstractTask>>(
        "transform",
        "Optional task transformation for the search."
        " Currently, adapt_costs() and no_transform() are available.",
        "no_transform()");
    SearchAlgorithm::add_options_to_feature(feature);
    SymVariables::add_options_to_feature(feature);
    SymParameters::add_options_to_feature(feature);
    feature.add_option<bool>("silent", "silent mode that avoids writing the cost bounds", "false");
    feature.add_option<bool>("simple", "simple/loopless plan construction", "false");
    feature.add_option<bool>("single_solution", "search for a single solution", "true");
}
}
