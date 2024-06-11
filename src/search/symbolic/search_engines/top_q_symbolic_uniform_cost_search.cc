#include "top_q_symbolic_uniform_cost_search.h"

#include "../searches/bidirectional_search.h"
#include "../searches/top_k_uniform_cost_search.h"


using namespace std;

namespace symbolic {
TopqSymbolicUniformCostSearch::TopqSymbolicUniformCostSearch(const plugins::Options &opts, bool fw, bool bw, bool alternating)
    : TopkSymbolicUniformCostSearch(opts, fw, bw, alternating),
      quality_multiplier(opts.get<double>("quality")) {
    utils::g_log << "Quality: " << quality_multiplier << endl;
}

void TopqSymbolicUniformCostSearch::new_solution(const SymSolutionCut &sol) {
    if (!(solution_registry->found_all_plans() || lower_bound > get_quality_bound())) {
        solution_registry->register_solution(sol);
        if (get_quality_bound() < numeric_limits<double>::infinity()) {
            // utils::g_log << "Quality bound: " << get_quality_bound() << endl;
            upper_bound = static_cast<int>(min((double)upper_bound, get_quality_bound() + 1));
        }
    } else {
        lower_bound = numeric_limits<int>::max();
    }
}

SearchStatus TopqSymbolicUniformCostSearch::step() {
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
        solution_registry->construct_cheaper_solutions(upper_bound);
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

class TopqSymbolicForwardUniformCostSearchFeature : public plugins::TypedFeature<SearchAlgorithm, TopqSymbolicUniformCostSearch> {
public:
    TopqSymbolicForwardUniformCostSearchFeature() : TypedFeature("symq_fw") {
        document_title("Topq Symbolic Forward Uniform Cost Search");
        document_synopsis("");
        symbolic::SymbolicSearch::add_options_to_feature(*this);
        this->add_option<shared_ptr<symbolic::PlanSelector>>("plan_selection", "plan selection strategy");
        this->add_option<double>("quality", "relative quality multiplier", "1.0", plugins::Bounds("1.0", "infinity"));
    }

    virtual shared_ptr<TopqSymbolicUniformCostSearch> create_component(const plugins::Options &options, const utils::Context &) const override {
        utils::g_log << "Search Algorithm: Topq Symbolic Forward Uniform Cost Search" << endl;
        return make_shared<TopqSymbolicUniformCostSearch>(options, true, false);
    }
};

static plugins::FeaturePlugin<TopqSymbolicForwardUniformCostSearchFeature> _fw_plugin;

class TopqSymbolicBackwardUniformCostSearchFeature : public plugins::TypedFeature<SearchAlgorithm, TopqSymbolicUniformCostSearch> {
public:
    TopqSymbolicBackwardUniformCostSearchFeature() : TypedFeature("symq_bw") {
        document_title("Topq Symbolic Backward Uniform Cost Search");
        document_synopsis("");
        symbolic::SymbolicSearch::add_options_to_feature(*this);
        this->add_option<shared_ptr<symbolic::PlanSelector>>("plan_selection", "plan selection strategy");
        this->add_option<double>("quality", "relative quality multiplier", "1.0", plugins::Bounds("1.0", "infinity"));
    }

    virtual shared_ptr<TopqSymbolicUniformCostSearch> create_component(const plugins::Options &options, const utils::Context &) const override {
        utils::g_log << "Search Algorithm: Topq Symbolic Backward Uniform Cost Search" << endl;
        return make_shared<TopqSymbolicUniformCostSearch>(options, false, true);
    }
};

static plugins::FeaturePlugin<TopqSymbolicBackwardUniformCostSearchFeature> _bw_plugin;

class TopqSymbolicBidirectionalUniformCostSearchFeature : public plugins::TypedFeature<SearchAlgorithm, TopqSymbolicUniformCostSearch> {
public:
    TopqSymbolicBidirectionalUniformCostSearchFeature() : TypedFeature("symq_bd") {
        document_title("Topq Symbolic Bidirectional Uniform Cost Search");
        document_synopsis("");
        symbolic::SymbolicSearch::add_options_to_feature(*this);
        this->add_option<shared_ptr<symbolic::PlanSelector>>("plan_selection", "plan selection strategy");
        this->add_option<double>("quality", "relative quality multiplier", "1.0", plugins::Bounds("1.0", "infinity"));
        this->add_option<bool>("alternating", "alternating", "false");
    }

    virtual shared_ptr<TopqSymbolicUniformCostSearch> create_component(const plugins::Options &options, const utils::Context &) const override {
        utils::g_log << "Search Algorithm: Topq Symbolic Bidirectional Uniform Cost Search" << endl;
        return make_shared<TopqSymbolicUniformCostSearch>(options, true, true, options.get<bool>("alternating"));
    }
};

static plugins::FeaturePlugin<TopqSymbolicBidirectionalUniformCostSearchFeature> _bd_plugin;
}
