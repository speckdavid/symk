#include "top_k_symbolic_uniform_cost_search.h"

#include "../sym_state_space_manager.h"

#include "../searches/bidirectional_search.h"
#include "../searches/top_k_uniform_cost_search.h"


#include <memory>

using namespace std;

namespace symbolic {
void TopkSymbolicUniformCostSearch::initialize() {
    SymbolicSearch::initialize();

    mgr = make_shared<SymStateSpaceManager>(vars.get(), sym_params, search_task);

    unique_ptr<TopkUniformCostSearch> fw_search = nullptr;
    unique_ptr<TopkUniformCostSearch> bw_search = nullptr;

    if (fw) {
        fw_search = unique_ptr<TopkUniformCostSearch>(
            new TopkUniformCostSearch(this, sym_params));
    }

    if (bw) {
        bw_search = unique_ptr<TopkUniformCostSearch>(
            new TopkUniformCostSearch(this, sym_params));
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
                            false,
                            simple);

    if (fw && bw) {
        search = unique_ptr<BidirectionalSearch>(new BidirectionalSearch(
                                                     this, sym_params, move(fw_search), move(bw_search)));
    } else {
        search.reset(fw ? fw_search.release() : bw_search.release());
    }
}

TopkSymbolicUniformCostSearch::TopkSymbolicUniformCostSearch(
    const plugins::Options &opts, bool fw, bool bw, bool alternating)
    : SymbolicUniformCostSearch(opts, fw, bw, alternating) {}

void TopkSymbolicUniformCostSearch::new_solution(const SymSolutionCut &sol) {
    if (!solution_registry->found_all_plans()) {
        solution_registry->register_solution(sol);
    } else {
        lower_bound = numeric_limits<int>::max();
    }
}
class TopkSymbolicForwardUniformCostSearchFeature : public plugins::TypedFeature<SearchAlgorithm, TopkSymbolicUniformCostSearch> {
public:
    TopkSymbolicForwardUniformCostSearchFeature() : TypedFeature("symk_fw") {
        document_title("Topk Symbolic Forward Uniform Cost Search");
        document_synopsis("");
        symbolic::SymbolicSearch::add_options_to_feature(*this);
        this->add_option<shared_ptr<symbolic::PlanSelector>>("plan_selection", "plan selection strategy");
    }

    virtual shared_ptr<TopkSymbolicUniformCostSearch> create_component(const plugins::Options &options, const utils::Context &) const override {
        utils::g_log << "Search Algorithm: Topk Symbolic Forward Uniform Cost Search" << endl;
        return make_shared<TopkSymbolicUniformCostSearch>(options, true, false);
    }
};

static plugins::FeaturePlugin<TopkSymbolicForwardUniformCostSearchFeature> _fw_plugin;

class TopkSymbolicBackwardUniformCostSearchFeature : public plugins::TypedFeature<SearchAlgorithm, TopkSymbolicUniformCostSearch> {
public:
    TopkSymbolicBackwardUniformCostSearchFeature() : TypedFeature("symk_bw") {
        document_title("Topk Symbolic Backward Uniform Cost Search");
        document_synopsis("");
        symbolic::SymbolicSearch::add_options_to_feature(*this);
        this->add_option<shared_ptr<symbolic::PlanSelector>>("plan_selection", "plan selection strategy");
    }

    virtual shared_ptr<TopkSymbolicUniformCostSearch> create_component(const plugins::Options &options, const utils::Context &) const override {
        utils::g_log << "Search Algorithm: Topk Symbolic Backward Uniform Cost Search" << endl;
        return make_shared<TopkSymbolicUniformCostSearch>(options, false, true);
    }
};

static plugins::FeaturePlugin<TopkSymbolicBackwardUniformCostSearchFeature> _bw_plugin;

class TopkSymbolicBidirectionalUniformCostSearchFeature : public plugins::TypedFeature<SearchAlgorithm, TopkSymbolicUniformCostSearch> {
public:
    TopkSymbolicBidirectionalUniformCostSearchFeature() : TypedFeature("symk_bd") {
        document_title("Topk Symbolic Bidirectional Uniform Cost Search");
        document_synopsis("");
        symbolic::SymbolicSearch::add_options_to_feature(*this);
        this->add_option<shared_ptr<symbolic::PlanSelector>>("plan_selection", "plan selection strategy");
        this->add_option<bool>("alternating", "alternating", "false");
    }

    virtual shared_ptr<TopkSymbolicUniformCostSearch> create_component(const plugins::Options &options, const utils::Context &) const override {
        utils::g_log << "Search Algorithm: Topk Symbolic Bidirectional Uniform Cost Search" << endl;
        return make_shared<TopkSymbolicUniformCostSearch>(options, true, true, options.get<bool>("alternating"));
    }
};

static plugins::FeaturePlugin<TopkSymbolicBidirectionalUniformCostSearchFeature> _bd_plugin;
}
