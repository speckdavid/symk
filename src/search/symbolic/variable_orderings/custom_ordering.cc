#include "custom_ordering.h"

#include "../../plugins/plugin.h"
#include "../../utils/logging.h"

#include <algorithm>
#include <iostream>

using namespace std;

namespace symbolic {
CustomOrdering::CustomOrdering(const plugins::Options &opts)
    : VariableOrdering(opts), ordering(opts.get_list<int>("ordering")) {
}

void CustomOrdering::compute_ordering(
    const TaskProxy &task_proxy, vector<int> &var_order) {
    var_order = ordering;
    if (reversed) {
        reverse(var_order.begin(), var_order.end());
    }

    if (task_proxy.get_variables().size() != var_order.size()) {
        cerr << "Task has " << task_proxy.get_variables().size()
             << " variables and custom variable ordering has "
             << var_order.size() << " variables." << endl;
        utils::exit_with(utils::ExitCode::SEARCH_INPUT_ERROR);
    }
}

class CustomOrderingFeature : public plugins::TypedFeature<VariableOrdering> {
public:
    CustomOrderingFeature() : TypedFeature("custom_ordering") {
        document_title("Custom variable ordering specified by the user.");
        add_list_option<int>(
            "ordering",
            "Variable ordering as a list of variable indices, e.g. [0,2,1].");
        VariableOrdering::add_options_to_feature(*this);
    }

    shared_ptr<VariableOrdering> create_component(
        const plugins::Options &opts) const override {
        return make_shared<CustomOrdering>(opts);
    }
};

static plugins::FeaturePlugin<CustomOrderingFeature> _plugin;
}