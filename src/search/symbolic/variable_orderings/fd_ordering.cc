#include "fd_ordering.h"

#include "../../plugins/plugin.h"

#include <algorithm>
#include <numeric>

using namespace std;

namespace symbolic {
FDOrdering::FDOrdering(const plugins::Options &opts) : VariableOrdering(opts) {
}

void FDOrdering::compute_ordering(
    const TaskProxy &task_proxy, vector<int> &var_order) {
    var_order.resize(task_proxy.get_variables().size());
    iota(var_order.begin(), var_order.end(), 0);
    if (reversed) {
        reverse(var_order.begin(), var_order.end());
    }
}

class FDOrderingFeature : public plugins::TypedFeature<VariableOrdering> {
public:
    FDOrderingFeature() : TypedFeature("fd_ordering") {
        document_title("Fast Downward variable ordering.");
        VariableOrdering::add_options_to_feature(*this);
    }

    shared_ptr<VariableOrdering> create_component(
        const plugins::Options &opts) const override {
        return make_shared<FDOrdering>(opts);
    }
};

static plugins::FeaturePlugin<FDOrderingFeature> _plugin;
}