#include "top_k_even_selector.h"

#include "../../option_parser.h"

using namespace std;

namespace symbolic {
TopKEvenSelector::TopKEvenSelector(const options::Options &opts)
    : PlanSelector(opts) {
    anytime_completness = true;
}

void TopKEvenSelector::add_plan(const Plan &plan) {
    if (!has_rejected_plan(plan) && !has_accepted_plan(plan)) {
        if (plan.size() % 2 == 0) {
            save_accepted_plan(plan);
        } else {
            save_rejected_plan(plan);
        }
    }
}

static shared_ptr<PlanSelector> _parse(OptionParser &parser) {
    PlanSelector::add_options_to_parser(parser);

    Options opts = parser.parse();
    if (parser.dry_run())
        return nullptr;
    return make_shared<TopKEvenSelector>(opts);
}

static Plugin<PlanSelector> _plugin("top_k_even", _parse);
}
