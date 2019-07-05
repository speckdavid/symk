#include "sym_plan_reconstruction.h"
#include "closed_list.h"
#include "transition_relation.h"

namespace symbolic
{

bool PlanReconstructor::states_on_path(const Plan &plan, BDD &states)
{
  GlobalState cur = state_registry->get_initial_state();
  states = sym_vars->getStateBDD(cur);
  BDD zero_reachable = states;
  bool zero_loop = false;
  for (auto &op : plan)
  {
    cur = state_registry->get_successor_state(
        cur, state_registry->get_task_proxy().get_operators()[op]);
    BDD new_state = sym_vars->getStateBDD(cur);
    states += new_state;

    // Check for a zero loop!
    if (task_has_zero_costs() && !zero_loop)
    {
      if (state_registry->get_task_proxy().get_operators()[op].get_cost() != 0)
      {
        zero_reachable = new_state;
      }
      else
      {
        BDD intersection = zero_reachable * new_state;
        if (!intersection.IsZero())
        {
          zero_loop = true;
        }
        zero_reachable += new_state;
      }
    }
  }
  return zero_loop;
}

size_t PlanReconstructor::different(const std::vector<Plan> &plans, const Plan &plan) const
{
  for (auto &cur : plans)
  {
    if (cur.size() == plan.size())
    {
      bool same = true;
      for (size_t i = 0; i < cur.size(); ++i)
      {
        if (cur.at(i) != plan.at(i))
        {
          same = false;
          break;
        }
      }
      if (same)
      {
        return false;
      }
    }
  }
  return true;
}

// Hashes a vector of ints (= a plan)
// According to the following link this is the has function used by boost
// for hashing vector<int>. Experience: really good function
// https://stackoverflow.com/questions/20511347/a-good-hash-function-for-a-vector
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
  BDD new_goal_path_states;
  bool zero_loop_plan = false;
  if (hashes_found_plans.count(plan_seed) == 0)
  {
    num_found_plans += 1;
    hashes_found_plans[plan_seed] = std::vector<Plan>();
    hashes_found_plans[plan_seed].push_back(plan);
    plan_mgr.save_plan(plan, state_registry->get_task_proxy(), false, true);
    zero_loop_plan = states_on_path(plan, new_goal_path_states);
    states_on_goal_path += new_goal_path_states;
    // std::cout << "ADD PLAN --------------" << std::endl;
  }
  else
  {
    if (different(hashes_found_plans[plan_seed], plan))
    {
      num_found_plans += 1;
      hashes_found_plans[plan_seed].push_back(plan);
      plan_mgr.save_plan(plan, state_registry->get_task_proxy(), false, true);
      zero_loop_plan = states_on_path(plan, new_goal_path_states);
      states_on_goal_path += new_goal_path_states;
    }
  }

  // Detected a zero loop => we can generate all remaining plans
  if (zero_loop_plan)
  {
    std::pair<int, int> zero_cost_op_seq(-1, -1);
    int last_zero_op_state = 0;
    std::vector<GlobalState> states;
    states.push_back(state_registry->get_initial_state());
    for (size_t op_i = 0; op_i < plan.size(); ++op_i)
    {
      GlobalState succ = state_registry->get_successor_state(
          states.back(), state_registry->get_task_proxy().get_operators()[plan[op_i]]);

      for (size_t state_i = last_zero_op_state; state_i < states.size(); ++state_i)
      {
        if (states[state_i].get_id() == succ.get_id())
        {
          zero_cost_op_seq.first = state_i;
          zero_cost_op_seq.second = op_i;
          break;
        }
      }
      if (state_registry->get_task_proxy().get_operators()[op_i].get_cost() != 0)
      {
        last_zero_op_state = states.size() - 1;
      }

      if (zero_cost_op_seq.first != -1)
      {
        break;
      }
      states.push_back(succ);
    }

    if (zero_cost_op_seq.first == -1)
    {
      std::cerr << "Zero loop goes wrong!" << std::endl;
      exit(0);
    }
    Plan cur_plan = plan;
    std::cout << "\nZero loop!" << std::endl;
    while (!found_enough_plans())
    {
      cur_plan.insert(cur_plan.begin() + zero_cost_op_seq.first, plan.begin() + zero_cost_op_seq.first, plan.begin() + zero_cost_op_seq.second + 1);
      plan_mgr.save_plan(cur_plan, state_registry->get_task_proxy(), false, true);
      num_found_plans += 1;
    }
  }
}

BDD PlanReconstructor::get_resulting_state(const Plan &plan) const
{
  GlobalState cur = state_registry->get_initial_state();
  for (auto &op : plan)
  {
    cur = state_registry->get_successor_state(
        cur, state_registry->get_task_proxy().get_operators()[op]);
  }
  return sym_vars->getStateBDD(cur);
}

BDD PlanReconstructor::bdd_for_zero_reconstruction(
    const BDD &cut, int cost, std::shared_ptr<ClosedList> closed) const
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

void PlanReconstructor::extract_all_plans(SymSolutionCut &sym_cut, bool fw, Plan plan)
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

void PlanReconstructor::extract_all_cost_plans(SymSolutionCut &sym_cut, bool fw, Plan &plan)
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
      SymSolutionCut new_cut(0, sym_cut.get_h(), get_resulting_state(plan));
      reconstruct_cost_action(new_cut, false, uni_search_bw->getClosedShared(), plan);
    }
  }
  else
  {
    reconstruct_cost_action(sym_cut, false, uni_search_bw->getClosedShared(), plan);
  }
}

void PlanReconstructor::extract_all_zero_plans(SymSolutionCut &sym_cut, bool fw, Plan &plan)
{

  //std::cout << sym_cut << std::endl;
  //std::cout << plan.size() << std::endl;
  //std::cout << "Found plans: " << found_plans.size() << std::endl;
  //for (auto &op : plan)
  //{
  //  std::cout << state_registry->get_task_proxy().get_operators()[op].get_name() << " " << std::endl;
  //}
  //std::cout << std::endl;

  BDD intersection;
  // Only zero costs left!
  if (sym_cut.get_g() == 0 && sym_cut.get_h() == 0)
  {
    // Check wether we are really in a initial or goal state
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
        SymSolutionCut new_cut(0, sym_cut.get_h(), get_resulting_state(plan));

        intersection = new_cut.get_cut() * uni_search_bw->getClosedShared()->get_start_states();
        if (!intersection.IsZero())
        {
          add_plan(plan);
          if (found_enough_plans())
          {
            return;
          }
        }
        reconstruct_zero_action(new_cut, false, uni_search_bw->getClosedShared(), plan);
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
        intersection = sym_cut.get_cut() * uni_search_fw->getClosedShared()->get_start_states();
        if (!intersection.IsZero())
        {
          SymSolutionCut new_cut(0, sym_cut.get_h(), get_resulting_state(plan));
          reconstruct_cost_action(new_cut, false, uni_search_bw->getClosedShared(), plan);
          reconstruct_zero_action(new_cut, false, uni_search_bw->getClosedShared(), plan);
        }
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
    SymSolutionCut &sym_cut, bool fw, std::shared_ptr<ClosedList> closed,
    const Plan &plan)
{
  int cur_cost = fw ? sym_cut.get_g() : sym_cut.get_h();
  BDD cut = sym_cut.get_cut();

  bool some_action_found = false;
  BDD succ;
  for (size_t newSteps0 = 0; newSteps0 < closed->get_num_zero_closed_layers(cur_cost); newSteps0++)
  {
    for (const TransitionRelation &tr : trs.at(0))
    {
      succ = fw ? tr.preimage(cut) : tr.image(cut);
      if (succ.IsZero())
      {
        continue;
      }

      BDD intersection =
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
        SymSolutionCut new_cut(sym_cut.get_g(), sym_cut.get_h(), intersection);
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
    SymSolutionCut &sym_cut, bool fw, std::shared_ptr<ClosedList> closed,
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
      BDD succ =
          fw ? tr.preimage(sym_cut.get_cut()) : tr.image(sym_cut.get_cut());
      BDD intersection = succ * closed->get_closed_at(new_cost);
      if (intersection.IsZero())
      {
        continue;
      }
      Plan new_plan = plan;
      some_action_found = true;
      SymSolutionCut new_cut(0, 0, intersection);
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
  plan_mgr.set_plan_filename("found_plans/sas_plan");
}

int PlanReconstructor::reconstruct_plans(const SymSolutionCut &cut,
                                         size_t num_desired_plans, BDD &goal_path_states)
{
  this->num_desired_plans = num_desired_plans;
  num_found_plans = 0;
  states_on_goal_path = sym_vars->zeroBDD();
  Plan plan;
  SymSolutionCut modifiable_cut = cut;

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
  goal_path_states = states_on_goal_path;
  return num_found_plans;
}

} // namespace symbolic
