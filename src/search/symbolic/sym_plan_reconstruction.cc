#include "sym_plan_reconstruction.h"
#include "closed_list.h"
#include "transition_relation.h"

namespace symbolic
{

// Hashes a vecor of ints which is a plan (https://stackoverflow.com/questions/20511347/a-good-hash-function-for-a-vector)
size_t PlanReconstructor::get_hash_value(const Plan &plan) const
{
  std::size_t seed = plan.size();
  for (auto &op : plan)
  {
    seed ^= op.get_index() + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  }
  return seed;
}

void PlanReconstructor::add_plan(const Plan &plan)
{
  size_t plan_seed = get_hash_value(plan);
  if (hashes_found_plans.count(plan_seed) == 0)
  {
    found_plans.push_back(plan);
    hashes_found_plans.insert(plan_seed);
  }
}

Bdd PlanReconstructor::get_resulting_state(const Plan &plan) const
{
  GlobalState cur = state_registry->get_initial_state();
  for (auto &op : plan)
  {
    cur = state_registry->get_successor_state(
        cur, state_registry->get_task_proxy().get_operators()[op]);
  }
  return sym_vars->getStateBDD(cur);
}

Bdd PlanReconstructor::bdd_for_zero_reconstruction(
    const Bdd &cut, int cost, std::shared_ptr<ClosedList> closed) const
{
  // Contains 0 buckets
  if (closed->get_num_zero_closed_layers(cost))
  {
    size_t steps0 = closed->get_zero_cut(cost, cut);
    if (steps0 < closed->get_num_zero_closed_layers(cost))
    {
      return cut * closed->get_zero_closed_at(cost, steps0);
    }
  }

  // There exist no 0-cost buckets or we haven't found it => search it
  return cut;
}

void PlanReconstructor::extract_all_plans(SymCut &sym_cut, bool fw, Plan plan)
{
  if (found_enough_plans())
  {
    return;
  }

  if (!task_has_zero_costs())
  {
    extract_all_cost_plans(sym_cut, fw, plan);
  }
  else
  {
    extract_all_zero_plans(sym_cut, fw, plan);
  }
}

void PlanReconstructor::extract_all_cost_plans(SymCut &sym_cut, bool fw, Plan &plan)
{
  // std::cout << sym_cut << std::endl;
  if (sym_cut.get_g() == 0 && sym_cut.get_h() == 0)
  {
    add_plan(plan);
    return;
  }

  // Resolve cost action
  if (fw)
  {
    if (sym_cut.get_g() > 0)
    {
      reconstruct_cost_action(sym_cut, true, uni_search_fw->getClosedShared(), plan);
    }
    else
    {
      SymCut new_cut(0, sym_cut.get_h(), get_resulting_state(plan));
      reconstruct_cost_action(new_cut, false, uni_search_bw->getClosedShared(), plan);
    }
  }
  else
  {
    reconstruct_cost_action(sym_cut, false, uni_search_bw->getClosedShared(), plan);
  }
}

void PlanReconstructor::extract_all_zero_plans(SymCut &sym_cut, bool fw, Plan &plan)
{

  /*std::cout << sym_cut << std::endl;
  std::cout << "Found plans: " << found_plans.size() << std::endl;
  for (auto &op : plan)
  {
    std::cout << state_registry->get_task_proxy().get_operators()[op].get_name() << " " << std::endl;
  }
  std::cout << std::endl;*/

  // Only zero costs left!
  if (sym_cut.get_g() == 0 && sym_cut.get_h() == 0)
  {
    // Check wether we are really in a initial or goal state
    Bdd intersection;
    if (fw && !uni_search_bw)
    {
      intersection = sym_cut.get_cut() * uni_search_fw->getClosedShared()->get_start_states();
      if (!intersection.IsZero())
      {
        add_plan(plan);
        if (found_enough_plans())
        {
          return;
        }
      }
      reconstruct_zero_action(sym_cut, true, uni_search_fw->getClosedShared(), plan);
    }
    else if (fw && uni_search_bw)
    {
      intersection = sym_cut.get_cut() * uni_search_fw->getClosedShared()->get_start_states();
      if (!intersection.IsZero())
      {
        reconstruct_zero_action(sym_cut, false, uni_search_bw->getClosedShared(), plan);
      }
      reconstruct_zero_action(sym_cut, true, uni_search_fw->getClosedShared(), plan);
    }
    else
    { // bw
      intersection = sym_cut.get_cut() * uni_search_bw->getClosedShared()->get_start_states();
      if (!intersection.IsZero())
      {
        add_plan(plan);
        if (found_enough_plans())
        {
          return;
        }
      }
      reconstruct_zero_action(sym_cut, false, uni_search_bw->getClosedShared(), plan);
    }
  }
  else
  {
    // Some cost left!
    if (fw)
    {
      if (sym_cut.get_g() > 0)
      {
        reconstruct_cost_action(sym_cut, true, uni_search_fw->getClosedShared(), plan);
      }
      else
      {
        SymCut new_cut(0, sym_cut.get_h(), get_resulting_state(plan));
        reconstruct_cost_action(new_cut, false, uni_search_bw->getClosedShared(), plan);
      }
      reconstruct_zero_action(sym_cut, true, uni_search_fw->getClosedShared(), plan);
    }
    else
    {
      reconstruct_cost_action(sym_cut, false, uni_search_bw->getClosedShared(), plan);
      reconstruct_zero_action(sym_cut, false, uni_search_bw->getClosedShared(), plan);
    }
  }
}

bool PlanReconstructor::reconstruct_zero_action(
    SymCut &sym_cut, bool fw, std::shared_ptr<ClosedList> closed,
    const Plan &plan)
{
  int cur_cost = fw ? sym_cut.get_g() : sym_cut.get_h();
  Bdd cut = sym_cut.get_cut();

  bool some_action_found = false;
  Bdd succ;
  for (size_t newSteps0 = 0; newSteps0 < closed->get_num_zero_closed_layers(cur_cost); newSteps0++)
  {
    for (const TransitionRelation &tr : trs.at(0))
    {
      succ = fw ? tr.preimage(cut) : tr.image(cut);
      if (succ.IsZero())
      {
        continue;
      }

      Bdd intersection =
          succ * closed->get_zero_closed_at(cur_cost, newSteps0);
      if (!intersection.IsZero())
      {
        Plan new_plan = plan;
        some_action_found = true;
        if (fw)
        {
          new_plan.insert(new_plan.begin(), *(tr.getOpsIds().begin()));
        }
        else
        {
          new_plan.push_back(*(tr.getOpsIds().begin()));
        }
        SymCut new_cut(sym_cut.get_g(), sym_cut.get_h(), intersection);
        extract_all_plans(new_cut, fw, new_plan);

        if (found_enough_plans())
        {
          return true;
        }
      }
    }
  }
  return some_action_found;
}

bool PlanReconstructor::reconstruct_cost_action(
    SymCut &sym_cut, bool fw, std::shared_ptr<ClosedList> closed,
    const Plan &plan)
{
  int cur_cost = fw ? sym_cut.get_g() : sym_cut.get_h();
  bool some_action_found = false;

  for (auto key : trs)
  {
    int new_cost = cur_cost - key.first;
    if (key.first == 0 || new_cost < 0)
    {
      continue;
    }
    for (TransitionRelation &tr : key.second)
    {
      Bdd succ =
          fw ? tr.preimage(sym_cut.get_cut()) : tr.image(sym_cut.get_cut());
      Bdd intersection = succ * closed->get_closed_at(new_cost);
      if (intersection.IsZero())
      {
        continue;
      }
      Plan new_plan = plan;
      some_action_found = true;
      SymCut new_cut(0, 0, intersection);
      if (fw)
      {
        new_plan.insert(new_plan.begin(), *(tr.getOpsIds().begin()));
        new_cut.set_g(new_cost);
        new_cut.set_h(sym_cut.get_h());
      }
      else
      {
        new_plan.push_back(*(tr.getOpsIds().begin()));
        new_cut.set_g(sym_cut.get_g());
        new_cut.set_h(new_cost);
      }
      extract_all_plans(new_cut, fw, new_plan);

      if (found_enough_plans())
      {
        return true;
      }
    }
  }
  return some_action_found;
}

PlanReconstructor::PlanReconstructor(
    UnidirectionalSearch *uni_search_fw, UnidirectionalSearch *uni_search_bw,
    std::shared_ptr<SymVariables> sym_vars,
    std::shared_ptr<StateRegistry> state_registry)
    : uni_search_fw(uni_search_fw), uni_search_bw(uni_search_bw),
      sym_vars(sym_vars), state_registry(state_registry)
{
  auto cur_search = uni_search_fw ? uni_search_fw : uni_search_bw;
  auto cur_closed = cur_search->getClosedShared();
  trs = cur_search->getStateSpaceShared()->getIndividualTRs();
}

void PlanReconstructor::reconstruct_plans(const SymCut &cut,
                                          size_t desired_num_plans,
                                          std::vector<Plan> &plans)
{
  this->desired_num_plans = desired_num_plans;
  found_plans.clear();
  Plan plan;
  SymCut modifiable_cut = cut;

  if (uni_search_fw && !uni_search_bw)
  {
    modifiable_cut.set_h(0);
  }
  if (!uni_search_fw && uni_search_bw)
  {
    modifiable_cut.set_g(0);
  }

  if (uni_search_fw)
  {
    extract_all_plans(modifiable_cut, true, plan);
  }
  else
  {
    extract_all_plans(modifiable_cut, false, plan);
  }
  plans = found_plans;
  // std::cout << "DONE" << std::endl;
}

} // namespace symbolic
