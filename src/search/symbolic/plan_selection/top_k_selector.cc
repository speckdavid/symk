#include "top_k_selector.h"

#include "../../option_parser.h"

using namespace std;

namespace symbolic {
TopKSelector::TopKSelector(const options::Options &opts) : PlanDataBase(opts) {
    PlanDataBase::anytime_completness = true;
}

void TopKSelector::add_plan(const Plan &plan) {
    if (!has_accepted_plan(plan)) {
        save_accepted_plan(plan);
    }
}

static shared_ptr<PlanDataBase> _parse(OptionParser &parser) {
    PlanDataBase::add_options_to_parser(parser);

    Options opts = parser.parse();
    if (parser.dry_run())
        return nullptr;
    return make_shared<TopKSelector>(opts);
}

static Plugin<PlanDataBase> _plugin("top_k", _parse);
}
