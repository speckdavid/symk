#include "sym_solution_registry.h"
#include "sym_plan_reconstruction.h"
#include "../tasks/root_task.h"

namespace symbolic {
    SymCut::SymCut(int g, int h, Bdd cut) : g(g), h(h), cut(cut) {}

    int SymCut::get_g() const {
        return g;
    }

    int SymCut::get_h() const {
        return h;
    }
    
    int SymCut::get_f() const {
        return g + h;
    }

    Bdd SymCut::get_cut() const {
        return cut;
    }
        
    void SymCut::merge(const SymCut &other) {
        assert(*this == other);
        cut += other.get_cut();
    }

    void SymCut::set_g(int g) {
        this->g = g;
    }

    void SymCut::set_h(int h) {
        this->h = h;
    }

    void SymCut::set_cut(Bdd cut) {
        this->cut = cut;
    }

    bool SymCut::operator<(const SymCut &other) const {
        bool result = get_f() < other.get_f();
        result |= (get_f() == other.get_f() && get_g() < other.get_g());
        return result;
    }

    bool SymCut::operator>(const SymCut &other) const {
        bool result = get_f() > other.get_f();
        result |= (get_f() == other.get_f() && get_g() > other.get_g());
        return result;
    }

    bool SymCut::operator==(const SymCut &other) const {
        return get_g() == other.get_g() && get_h() == other.get_h();
    }

    bool SymCut::operator!=(const SymCut &other) const {
        return !(get_g() == other.get_g() && get_h() == other.get_h());
    }

    SymSolutionRegistry::SymSolutionRegistry(int target_num_plans) : 
        plan_reconstructor(nullptr), 
        target_num_plans(target_num_plans), 
        relevant_task(*tasks::g_root_task),
        state_registry(nullptr),
        sym_vars(nullptr)
        {}

    Bdd SymSolutionRegistry::states_on_path(const Plan& plan) {
        GlobalState cur = state_registry->get_initial_state();
        Bdd res = sym_vars->getStateBDD(cur);
        for (auto &op : plan)
        {
            cur = state_registry->get_successor_state(
            cur, relevant_task.get_operators()[op]);
            res += sym_vars->getStateBDD(cur);
        }
        return res;
    }

    void SymSolutionRegistry::register_solution(const SymSolution& solution) {
        if (!solution.solved()) {
            return;
        }
        if (!plan_reconstructor) {
            plan_reconstructor = std::make_shared<PlanReconstructor>(solution.get_fw_search(), solution.get_bw_search(), sym_vars, state_registry);
        }

        SymCut new_cut(solution.get_g(), solution.get_h(), solution.get_cut());

        bool merged = false;
        size_t pos = 0;
        for (; pos < sym_cuts.size(); pos++) {
            // A cut with same g and h values exist!
            if (sym_cuts[pos] == new_cut) {
                sym_cuts[pos].merge(new_cut);
                merged = true;
                break;
            }
            if (sym_cuts[pos] > new_cut)
            {
                break;
            }
        }
        if (!merged) {
            sym_cuts.insert(sym_cuts.begin() + pos, new_cut);
        }
    }

    void SymSolutionRegistry::construct_cheaper_solutions(int bound) {
        /*std::cout << "\nReconstruction bound: " << bound << std::endl;
        for (auto& cut : sym_cuts) {
            std::cout << cut << std::endl;
        }*/

        while (sym_cuts.size() > 0 && sym_cuts.at(0).get_f() < bound && !found_all_plans()) {
            std::vector<Plan> new_plans;
            plan_reconstructor->reconstruct_plans(sym_cuts[0], missing_plans(), new_plans);
            found_plans.insert(found_plans.end(), new_plans.begin(), new_plans.end());
            for (auto& plan : new_plans) {
                plan_mgr.save_plan(plan, relevant_task, false, true);
                if (!found_all_plans()) {
                    states_on_goal_paths += states_on_path(plan);
                }
            }
            sym_cuts.erase(sym_cuts.begin());
        }
    }
}