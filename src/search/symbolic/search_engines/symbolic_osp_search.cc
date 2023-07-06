#include "symbolic_osp_search.h"

#include "../original_state_space.h"
#include "../plugin.h"
#include "../searches/osp_cost_search.h"
#include "../sym_function_creator.h"

#include "../../option_parser.h"


using namespace std;

namespace symbolic {
void SymbolicOspSearch::initialize() {
    SymbolicSearch::initialize();
    initialize_utlility();

    mgr = make_shared<OriginalStateSpace>(vars.get(), mgrParams, search_task);
    unique_ptr<OspCostSearch> fw_search = unique_ptr<OspCostSearch>(new OspCostSearch(this, searchParams));
    fw_search->init(mgr, true, nullptr);

    auto individual_trs = fw_search->getStateSpaceShared()->getIndividualTRs();

    solution_registry->init(vars,
                            fw_search->getClosedShared(),
                            nullptr,
                            individual_trs,
                            plan_data_base,
                            single_solution,
                            simple);

    search.reset(fw_search.release());
}

void SymbolicOspSearch::initialize_utlility() {
    upper_bound = search_task->get_plan_cost_bound();

    ADD add_utility_function = create_utility_function();
    partition_add_to_bdds(vars.get(), add_utility_function, utility_function);
    max_utility = utility_function.rbegin()->first;
    assert(max_utility == round(Cudd_V(add_utility_function.FindMax().getNode())));

    int min_utility = round(Cudd_V(add_utility_function.FindMin().getNode()));
    if (min_utility <= -numeric_limits<int>::max()) {
        cerr << "Utility values exceed integer limits." << endl;
        utils::exit_with(utils::ExitCode::SEARCH_INPUT_ERROR);
    }

    utils::g_log << "Plan cost bound: " << upper_bound - 1 << endl;
    utils::g_log << "Constant utility: " << search_task->get_constant_utility() << endl;
    utils::g_log << "Number of utility facts: " << search_task->get_num_utilties() << endl;
    utils::g_log << "Max utility value: " << max_utility << endl;
    cout << endl;
    // vars->to_dot(utility_function, "utility.dot");
}

ADD SymbolicOspSearch::create_utility_function() const {
    ADD res = vars->constant(search_task->get_constant_utility());
    for (const UtilityProxy util: task_proxy.get_utilities()) {
        BDD fact = vars->get_axiom_compiliation()->get_primary_representation(util.get_fact_pair().var, util.get_fact_pair().value);
        res += fact.Add() * vars->constant(util.get_utility());
    }
    return res;
}

SymSolutionCut SymbolicOspSearch::get_highest_util_solution(const SymSolutionCut &sol) const {
    double max_util_value = -1;
    BDD max_util_states = vars->zeroBDD();
    for (auto iter = utility_function.rbegin(); iter != utility_function.rend(); ++iter) {
        max_util_states = iter->second * sol.get_cut();
        if (!max_util_states.IsZero()) {
            max_util_value = iter->first;
            break;
        }
    }
    return SymSolutionCut(sol.get_g(), sol.get_h(), round(max_util_value), max_util_states);
}

SymbolicOspSearch::SymbolicOspSearch(
    const options::Options &opts) :
    SymbolicSearch(opts),
    max_utility(-numeric_limits<int>::max()),
    highest_seen_utility(-numeric_limits<int>::max()) {
    if (!is_oversubscribed) {
        cerr << "error: osp symbolic search does not support ordinary classical planning tasks. "
             << "Please use ordinary symbolic search, e.g., sym-bd()." << endl;
        utils::exit_with(utils::ExitCode::SEARCH_INPUT_ERROR);
    }
}

void SymbolicOspSearch::new_solution(const SymSolutionCut &sol) {
    auto osp_sol = get_highest_util_solution(sol);
    if (!solution_registry->found_all_plans() && sol.get_f() < upper_bound && highest_seen_utility < osp_sol.get_util()) {
        solution_registry->register_solution(osp_sol);
        highest_seen_utility = osp_sol.get_util();
        utils::g_log << "Best util: " << highest_seen_utility << endl;
    }
}

SearchStatus SymbolicOspSearch::step() {
    step_num++;
    // Handling empty plan
    if (step_num == 0) {
        BDD cut = mgr->getInitialState() * mgr->getGoal();
        if (!cut.IsZero()) {
            new_solution(SymSolutionCut(0, 0, cut));
        }
    }

    SearchStatus cur_status = IN_PROGRESS;

    // Search finished!
    if (lower_bound >= upper_bound) {
        solution_registry->construct_cheaper_solutions(numeric_limits<int>::max());
        solution_found = plan_data_base->get_num_reported_plan() > 0;
        cur_status = solution_found ? SOLVED : FAILED;
    } else if (max_utility == highest_seen_utility) {
        // Highest utility => Search finished!
        utils::g_log << "State with overall highest utility reached." << endl;
        solution_registry->construct_cheaper_solutions(numeric_limits<int>::max());
        if (solution_registry->found_all_plans()) {
            cur_status = SOLVED;
        }
    }
    if (lower_bound_increased && !silent) {
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
}

static shared_ptr<SearchEngine> _parse_forward_osp(OptionParser &parser) {
    parser.document_synopsis("Symbolic Forward Oversubscription Search", "");
    symbolic::SymbolicSearch::add_options_to_parser(parser);
    parser.add_option<shared_ptr<symbolic::PlanSelector>>(
        "plan_selection", "plan selection strategy", "top_k(num_plans=1)");
    Options opts = parser.parse();

    shared_ptr<symbolic::SymbolicSearch> engine = nullptr;
    if (!parser.dry_run()) {
        engine = make_shared<symbolic::SymbolicOspSearch>(opts);
        utils::g_log << "Symbolic Forward Oversubscription Search" << endl;
    }

    return engine;
}

static Plugin<SearchEngine> _plugin_sym_fw_ordinary("sym-osp-fw",
                                                    _parse_forward_osp);
