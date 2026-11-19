#include "variable_ordering.h"

#include "../../plugins/plugin.h"

using namespace std;

namespace symbolic {
VariableOrdering::VariableOrdering(const plugins::Options &opts)
    : reversed(opts.get<bool>("reversed")) {
}

void VariableOrdering::add_options_to_feature(plugins::Feature &feature) {
    feature.add_option<bool>("reversed", "Reverse the ordering", "false");
}

static class VariableOrderingPlugin
    : public plugins::TypedCategoryPlugin<VariableOrdering> {
public:
    VariableOrderingPlugin() : TypedCategoryPlugin("VariableOrdering") {
        document_synopsis("A variable ordering interface for symbolic search.");
        allow_variable_binding();
    }
} _category_plugin;
}