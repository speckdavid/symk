#include "moral_permissibility_selector.h"

#include "../../option_parser.h"

namespace symbolic {

MoralPermissibilitySelector::MoralPermissibilitySelector(
    const options::Options &opts)
    : PlanDataBase(opts) {
  PlanDataBase::anytime_completness = false;
}

void MoralPermissibilitySelector::add_plan(const Plan &plan) {
  if (!has_rejected_plan(plan) && !has_accepted_plan(plan)) {

    // Only accept plans with even number of plans
    // Here you can add any condition
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
  return std::make_shared<MoralPermissibilitySelector>(opts);
}

static Plugin<PlanDataBase> _plugin("moral_permissibility", _parse);

} // namespace symbolic
