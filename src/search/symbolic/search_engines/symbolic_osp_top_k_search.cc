#include "symbolic_osp_top_k_search.h"

#include "../original_state_space.h"
#include "../plugin.h"
#include "../searches/osp_cost_search.h"
#include "../sym_function_creator.h"

#include "../../option_parser.h"


using namespace std;

namespace symbolic {
void SymbolicOspTopkSearch::initialize() {
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
                            false,
                            simple);

    search.reset(fw_search.release());
}

SymbolicOspTopkSearch::SymbolicOspTopkSearch(
    const options::Options &opts) :
    SymbolicOspSearch(opts) {}

vector<SymSolutionCut> SymbolicOspTopkSearch::get_all_util_solutions(const SymSolutionCut &sol) {
    vector<SymSolutionCut> result;
    for (auto iter = utility_function.rbegin(); iter != utility_function.rend(); ++iter) {
        BDD util_states = iter->second * sol.get_cut();
        if (!util_states.IsZero()) {
            double util_value = iter->first;
            result.emplace_back(sol.get_g(), sol.get_h(), round(util_value), util_states);
        }
    }
    return result;
}

void SymbolicOspTopkSearch::new_solution(const SymSolutionCut &sol) {
    if (solution_registry->found_all_plans() || sol.get_f() >= upper_bound)
        return;

    for (auto const &osp_sol : get_all_util_solutions(sol)) {
        // We found a solution with highest possible utility and reconstruct it directly
        // This is kind of a hack...
        if (max_utility == osp_sol.get_util()) {
            utils::g_log << "States with overall highest utility reached." << endl;
            solution_registry->reconstruct_solution(osp_sol);
            highest_seen_utility = osp_sol.get_util();
            utils::g_log << "Best util: " << highest_seen_utility << endl;
        } else {
            solution_registry->register_solution(osp_sol);
            if (osp_sol.get_util() > highest_seen_utility) {
                highest_seen_utility = osp_sol.get_util();
                utils::g_log << "Best util: " << highest_seen_utility << endl;
            }
        }
    }
}

SearchStatus SymbolicOspTopkSearch::step() {
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
    parser.document_synopsis("Symbolic Forward Oversubscription Top-k Search", "");
    symbolic::SymbolicSearch::add_options_to_parser(parser);
    parser.add_option<shared_ptr<symbolic::PlanSelector>>(
        "plan_selection", "plan selection strategy");
    Options opts = parser.parse();

    shared_ptr<symbolic::SymbolicSearch> engine = nullptr;
    if (!parser.dry_run()) {
        engine = make_shared<symbolic::SymbolicOspTopkSearch>(opts);
        utils::g_log << "Symbolic Forward Oversubscription Top-k Search" << endl;
    }

    return engine;
}

static Plugin<SearchEngine> _plugin_sym_fw_ordinary("symk-osp-fw",
                                                    _parse_forward_osp);
