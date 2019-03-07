#include "sym_plan_reconstruction.h"
#include "closed_list.h"
#include "transition_relation.h"

namespace symbolic {


    bool PlanReconstructor::different(const Plan& plan1, const Plan& plan2) const
    {
        if (plan1.size() != plan2.size())
        {
            return true;
        }

        for (size_t op_id = 0; op_id < plan1.size(); op_id++)
        {
            if (plan1.at(op_id) != plan2.at(op_id))
            {
                return true;
            }
        }
        return false;
    }

    void PlanReconstructor::add_plan(const Plan& plan) 
    {
        for(size_t i = 0; i < closed_plans.size(); i++)
        {
            if (!different(plan, closed_plans.at(i)))
            {
                return;
            }
        }
        found_plans.push_back(plan);
        closed_plans.push_back(plan);
    }

    Bdd PlanReconstructor::get_resulting_state(const Plan& plan) const
    {
        GlobalState cur = state_registry->get_initial_state();
        for (auto &op : plan)
        {
            cur = state_registry->get_successor_state(
            cur, state_registry->get_task_proxy().get_operators()[op]);
        }
        return sym_vars->getStateBDD(cur);
    }

    Bdd PlanReconstructor::bdd_for_zero_reconstruction(const Bdd &cut, int cost, std::shared_ptr<ClosedList> closed) const 
    {
        // Contains 0 buckets
        if (closed->get_num_zero_closed_layers(cost)) {
            size_t steps0 = closed->get_zero_cut(cost, cut);
            if (steps0 < closed->get_num_zero_closed_layers(cost)) {
                return cut * closed->get_zero_closed_at(cost, steps0);
            }
        }

        // There exist no 0-cost buckets or we haven't found it => search it
        return cut;
    }

    void PlanReconstructor::extract_all_plans(SymCut& sym_cut, bool fw, std::shared_ptr<ClosedList> closed, Plan plan) 
    {
        // Resolve zero actions
        bool zero_action_found = reconstruct_zero_action(sym_cut, fw, closed, plan);
            
        // Resolve cost actions
        if (!zero_action_found)
        {
            bool cost_action_found = reconstruct_cost_action(sym_cut, fw, closed, plan);

            // No action found => empty plan (unless their is a bug)
            if (!cost_action_found)
            {
                found_plans.push_back(plan);
            }
        }

    }

    bool PlanReconstructor::reconstruct_zero_action(SymCut &sym_cut, bool fw, std::shared_ptr<ClosedList> closed, const Plan& plan) 
    {
        int cur_cost = fw ? sym_cut.get_g() : sym_cut.get_h();
        Bdd cut = sym_cut.get_cut();
        cut = bdd_for_zero_reconstruction(cut, cur_cost, closed);
        size_t steps0 = closed->get_zero_cut(cur_cost, cut);

        bool some_action_found = false;
        if (steps0 > 0)
        {
            Bdd succ;
            for (const TransitionRelation &tr : trs.at(0)) 
            {
                succ = fw ? tr.preimage(cut) : tr.image(cut);
                if (succ.IsZero())
                {
                    continue;
                }

                for (size_t newSteps0 = 0; newSteps0 < steps0; newSteps0++) {
                    Bdd intersection = succ * closed->get_zero_closed_at(cur_cost, newSteps0);
                    if (!intersection.IsZero()) {
                        Plan new_plan = plan;
                        new_plan.push_back(*(tr.getOpsIds().begin()));
                        some_action_found = true;
                        if (cur_cost == 0 && newSteps0== 0) {
                            if (fw) {
                                std::reverse(new_plan.begin(), new_plan.end());
                            }

                            // If bidir search we need to start backward recosntruction
                            // First simulate the plan to get the correct state then
                            // reconstruct from their...
                            if (fw && uni_search_bw) {
                                Bdd final_state = get_resulting_state(new_plan);
                                SymCut new_cut(0, sym_cut.get_h(), final_state);
                                extract_all_plans(new_cut, false, uni_search_bw->getClosedShared(), new_plan);
                            } else {
                                add_plan(new_plan);
                            }
                        } else {
                            SymCut new_cut(sym_cut.get_g(), sym_cut.get_h(), intersection);
                            extract_all_plans(new_cut, fw, closed, new_plan);    
                        }

                        if (found_enough_plans()) {
                            return true;
                        }
                    }
                }
            }
        }
        return some_action_found;
    }

    bool PlanReconstructor::reconstruct_cost_action(SymCut &sym_cut, bool fw, std::shared_ptr<ClosedList> closed, const Plan& plan) 
    {
        int cur_cost = fw ? sym_cut.get_g() : sym_cut.get_h();
        bool some_action_found = false;

        for (auto key : trs) 
        {
            int new_cost = cur_cost - key.first;
            if (key.first == 0 || new_cost < 0) {
                continue;
            }
            for (TransitionRelation &tr : key.second)
            {
                Bdd succ = fw ? tr.preimage(sym_cut.get_cut()) : tr.image(sym_cut.get_cut());
                Bdd intersection = succ * closed->get_closed_at(new_cost);
                if (intersection.IsZero())
                {
                    continue;
                }
                Plan new_plan = plan;
                new_plan.push_back(*(tr.getOpsIds().begin()));
                some_action_found = true;
                if (new_cost == 0 && closed->get_zero_cut(new_cost, intersection) == 0) 
                {
                    if (fw) 
                    {
                        std::reverse(new_plan.begin(), new_plan.end());
                    }

                    // If bidir search we need to start backward recosntruction
                    // First simulate the plan to get the correct state then
                    // reconstruct from their...
                    if (fw && uni_search_bw) {
                        Bdd final_state = get_resulting_state(new_plan);
                        SymCut new_cut(0, sym_cut.get_h(), final_state);
                        extract_all_plans(new_cut, false, uni_search_bw->getClosedShared(), new_plan);
                    } else {
                        add_plan(new_plan);
                    }
                } else {
                    SymCut new_cut(0, 0, intersection);
                    if (fw) {
                        new_cut.set_g(new_cost);
                        new_cut.set_h(sym_cut.get_h());
                    } else {
                        new_cut.set_g(sym_cut.get_g());
                        new_cut.set_h(new_cost);
                    }
                    extract_all_plans(new_cut, fw, closed, new_plan);
                }

                if (found_enough_plans()) {
                    return true;
                }
            }
        }
        return some_action_found;
    }

    PlanReconstructor::PlanReconstructor(UnidirectionalSearch* uni_search_fw, UnidirectionalSearch* uni_search_bw, std::shared_ptr<SymVariables> sym_vars, std::shared_ptr<StateRegistry> state_registry) : 
        uni_search_fw(uni_search_fw), 
        uni_search_bw(uni_search_bw),
        sym_vars(sym_vars),
        state_registry(state_registry) {
        auto cur_search = uni_search_fw ? uni_search_fw : uni_search_bw;
        auto cur_closed = cur_search->getClosedShared();
        trs = cur_search->getStateSpaceShared()->getIndividualTRs();
    }

    void PlanReconstructor::reconstruct_plans(const SymCut& cut, size_t desired_num_plans, std::vector<Plan>& plans) 
    {
        this->desired_num_plans = desired_num_plans;
        found_plans.clear();
        Plan plan;
        SymCut modifiable_cut = cut;
        if (uni_search_fw) {
            extract_all_plans(modifiable_cut, true, uni_search_fw->getClosedShared(), plan);
        } else {
            extract_all_plans(modifiable_cut, false, uni_search_bw->getClosedShared(), plan);
        }
        plans = found_plans;
    }


}
