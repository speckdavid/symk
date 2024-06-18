#include "symbolic_uniform_cost_search.h"

#include "../sym_state_space_manager.h"

#include "../searches/bidirectional_search.h"
#include "../searches/uniform_cost_search.h"


using namespace std;

namespace symbolic {
void SymbolicUniformCostSearch::initialize() {
    if (plan_data_base->get_num_desired_plans() > 1) {
        cerr << "*****************************************************************************"
             << "******************************************************************************"
             << endl;
        cerr << "*** Error: The symbolic search configuration for finding a single plan (e.g., sym_[fw|bw|bd]) is selected,"
             << " but multiple plans (" << plan_data_base->get_num_desired_plans() << ") have been requested. ***"
             << endl;
        cerr << "*** Please use symk_[fw|bw|bd] or symq_[fw|bw|bd] (note the additional 'k' or 'q') for multiple plans."
             << "                                                  ***"
             << endl;
        cerr << "*****************************************************************************"
             << "******************************************************************************"
             << endl << endl;
        utils::exit_with(utils::ExitCode::SEARCH_INPUT_ERROR);
    }

    SymbolicSearch::initialize();
    mgr = make_shared<SymStateSpaceManager>(vars.get(), sym_params, search_task);

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

    auto sym_trs = fw ? fw_search->getStateSpaceShared()->get_transition_relations()
            :  bw_search->getStateSpaceShared()->get_transition_relations();

    solution_registry->init(vars,
                            fw_search ? fw_search->getClosedShared() : nullptr,
                            bw_search ? bw_search->getClosedShared() : nullptr,
                            sym_trs,
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
    const plugins::Options &opts, bool fw, bool bw, bool alternating)
    : SymbolicSearch(opts), fw(fw), bw(bw), alternating(alternating) {}

void SymbolicUniformCostSearch::new_solution(const SymSolutionCut &sol) {
    if (!solution_registry->found_all_plans() && sol.get_f() < upper_bound) {
        solution_registry->register_solution(sol);
        upper_bound = sol.get_f();
    }
}

class SymbolicForwardUniformCostSearchFeature : public plugins::TypedFeature<SearchAlgorithm, SymbolicUniformCostSearch> {
public:
    SymbolicForwardUniformCostSearchFeature() : TypedFeature("sym_fw") {
        document_title("Symbolic Forward Uniform Cost Search");
        document_synopsis("");
        symbolic::SymbolicSearch::add_options_to_feature(*this);
        this->add_option<shared_ptr<symbolic::PlanSelector>>("plan_selection", "plan selection strategy", "top_k(num_plans=1)");
    }

    virtual shared_ptr<SymbolicUniformCostSearch> create_component(const plugins::Options &options, const utils::Context &) const override {
        utils::g_log << "Search Algorithm: Symbolic Forward Uniform Cost Search" << endl;
        return make_shared<SymbolicUniformCostSearch>(options, true, false);
    }
};

static plugins::FeaturePlugin<SymbolicForwardUniformCostSearchFeature> _fw_plugin;

class SymbolicBackwardUniformCostSearchFeature : public plugins::TypedFeature<SearchAlgorithm, SymbolicUniformCostSearch> {
public:
    SymbolicBackwardUniformCostSearchFeature() : TypedFeature("sym_bw") {
        document_title("Symbolic Backward Uniform Cost Search");
        document_synopsis("");
        symbolic::SymbolicSearch::add_options_to_feature(*this);
        this->add_option<shared_ptr<symbolic::PlanSelector>>("plan_selection", "plan selection strategy", "top_k(num_plans=1)");
    }

    virtual shared_ptr<SymbolicUniformCostSearch> create_component(const plugins::Options &options, const utils::Context &) const override {
        utils::g_log << "Search Algorithm: Symbolic Backward Uniform Cost Search" << endl;
        return make_shared<SymbolicUniformCostSearch>(options, false, true);
    }
};

static plugins::FeaturePlugin<SymbolicBackwardUniformCostSearchFeature> _bw_plugin;

class SymbolicBidirectionalUniformCostSearchFeature : public plugins::TypedFeature<SearchAlgorithm, SymbolicUniformCostSearch> {
public:
    SymbolicBidirectionalUniformCostSearchFeature() : TypedFeature("sym_bd") {
        document_title("Symbolic Bidirectional Uniform Cost Search");
        document_synopsis("");
        symbolic::SymbolicSearch::add_options_to_feature(*this);
        this->add_option<shared_ptr<symbolic::PlanSelector>>("plan_selection", "plan selection strategy", "top_k(num_plans=1)");
        this->add_option<bool>("alternating", "alternating", "false");
    }

    virtual shared_ptr<SymbolicUniformCostSearch> create_component(const plugins::Options &options, const utils::Context &) const override {
        utils::g_log << "Search Algorithm: Symbolic Bidirectional Uniform Cost Search" << endl;
        return make_shared<SymbolicUniformCostSearch>(options, true, true, options.get<bool>("alternating"));
    }
};

static plugins::FeaturePlugin<SymbolicBidirectionalUniformCostSearchFeature> _bd_plugin;
}
