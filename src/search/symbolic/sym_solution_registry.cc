#include "sym_solution_registry.h"
#include "../tasks/root_task.h"
#include "sym_plan_reconstruction.h"

namespace symbolic
{
SymCut::SymCut(int g, int h, Bdd cut) : g(g), h(h), cut(cut) {}

int SymCut::get_g() const { return g; }

int SymCut::get_h() const { return h; }

int SymCut::get_f() const { return g + h; }

Bdd SymCut::get_cut() const { return cut; }

void SymCut::merge(const SymCut &other)
{
    assert(*this == other);
    cut += other.get_cut();
}

void SymCut::set_g(int g) { this->g = g; }

void SymCut::set_h(int h) { this->h = h; }

void SymCut::set_cut(Bdd cut) { this->cut = cut; }

bool SymCut::operator<(const SymCut &other) const
{
    bool result = get_f() < other.get_f();
    result |= (get_f() == other.get_f() && get_g() < other.get_g());
    return result;
}

bool SymCut::operator>(const SymCut &other) const
{
    bool result = get_f() > other.get_f();
    result |= (get_f() == other.get_f() && get_g() > other.get_g());
    return result;
}

bool SymCut::operator==(const SymCut &other) const
{
    return get_g() == other.get_g() && get_h() == other.get_h();
}

bool SymCut::operator!=(const SymCut &other) const
{
    return !(get_g() == other.get_g() && get_h() == other.get_h());
}

SymSolutionRegistry::SymSolutionRegistry(int target_num_plans)
    : plan_reconstructor(nullptr), num_target_plans(target_num_plans),
      relevant_task(*tasks::g_root_task), state_registry(nullptr),
      sym_vars(nullptr), plan_cost_bound(-1)
{
}

void SymSolutionRegistry::register_solution(const SymSolution &solution)
{
    if (!solution.solved())
    {
        return;
    }
    if (!plan_reconstructor)
    {
        plan_reconstructor = std::make_shared<PlanReconstructor>(
            solution.get_fw_search(), solution.get_bw_search(), sym_vars,
            state_registry);
    }

    SymCut new_cut(solution.get_g(), solution.get_h(), solution.get_cut());
    // std::cout << "\nregister " << new_cut << std::endl;

    bool merged = false;
    size_t pos = 0;
    for (; pos < sym_cuts.size(); pos++)
    {
        // A cut with same g and h values exist!
        if (sym_cuts[pos] == new_cut)
        {
            sym_cuts[pos].merge(new_cut);
            merged = true;
            break;
        }
        if (sym_cuts[pos] > new_cut)
        {
            break;
        }
    }
    if (!merged)
    {
        sym_cuts.insert(sym_cuts.begin() + pos, new_cut);
    }
}

void SymSolutionRegistry::construct_cheaper_solutions(int bound)
{
  /*std::cout << "\nReconstruction bound: " << bound << std::endl;
  for (auto& cut : sym_cuts) {
      std::cout << cut << std::endl;
  }*/

    bool bound_used = false;
    while (sym_cuts.size() > 0 && sym_cuts.at(0).get_f() < bound &&
           !found_all_plans())
    {
        //std::cout << "Reconsturcting!" << plan_cost_bound << std::endl;
        // Ignore all cuts with costs smaller than the bound we already reconstructed
        if (sym_cuts.at(0).get_f() < plan_cost_bound)
        {
            sym_cuts.erase(sym_cuts.begin());
        } else {
            Bdd goal_path_states;
            num_found_plans += plan_reconstructor->reconstruct_plans(sym_cuts[0], missing_plans(), goal_path_states);
            states_on_goal_paths += goal_path_states;
            sym_cuts.erase(sym_cuts.begin());
            bound_used = true;
        }
    }

    if (bound_used) {
        plan_cost_bound = bound;
    }

}
} // namespace symbolic
