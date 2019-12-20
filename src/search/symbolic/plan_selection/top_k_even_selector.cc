#include "top_k_even_selector.h"

#include "../../option_parser.h"

namespace symbolic {

TopKEvenSelector::TopKEvenSelector(const options::Options &opts)
    : PlanDataBase(opts) {
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

static std::shared_ptr<PlanDataBase> _parse(OptionParser &parser) {
  PlanDataBase::add_options_to_parser(parser);

  Options opts = parser.parse();
  if (parser.dry_run())
    return nullptr;
  return std::make_shared<TopKEvenSelector>(opts);
}

static Plugin<PlanDataBase> _plugin("top_k_even", _parse);

} // namespace symbolic
