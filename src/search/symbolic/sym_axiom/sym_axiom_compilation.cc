#include "sym_axiom_compilation.h"
#include "../../plugin.h"
#include "../option_parser.h"
#include "../sym_variables.h"

#include <limits>
#include <queue>

namespace symbolic {
using namespace std;

SymAxiomCompilation::SymAxiomCompilation(std::shared_ptr<SymVariables> sym_vars)
    : sym_vars(sym_vars), task(*tasks::g_root_task) {}

bool SymAxiomCompilation::is_derived_variable(int var) const {
  return task.get_variables()[var].is_derived();
}

bool SymAxiomCompilation::is_in_body(int var, int axiom_id) const {
  EffectProxy eff = task.get_axioms()[axiom_id].get_effects()[0];
  for (size_t cond_i = 0; cond_i < eff.get_conditions().size(); cond_i++) {
    if (eff.get_conditions()[cond_i].get_variable().get_id() == var) {
      return true;
    }
  }
  return false;
}

bool SymAxiomCompilation::is_trivial_axiom(int axiom_id) const {
  int head = task.get_axioms()[axiom_id]
                 .get_effects()[0]
                 .get_fact()
                 .get_variable()
                 .get_id();
  int val = task.get_axioms()[axiom_id].get_effects()[0].get_fact().get_value();
  int default_val = task.get_variables()[head].get_default_axiom_value();
  return val == default_val;
}

int SymAxiomCompilation::get_axiom_level(int axiom_id) const {
  return task.get_axioms()[axiom_id]
      .get_effects()[0]
      .get_fact()
      .get_variable()
      .get_axiom_layer();
}

int SymAxiomCompilation::num_axiom_levels() const {
  int num_level = -1;
  for (size_t i = 0; i < task.get_variables().size(); i++) {
    num_level = std::max(num_level, task.get_variables()[i].get_axiom_layer());
  }
  return num_level + 1;
}

void SymAxiomCompilation::init_axioms() {
  utils::Timer timer;
  create_primary_representations();

  std::cout << std::fixed << "Symbolic Axiom initialization: " << timer
            << std::endl;
}

BDD SymAxiomCompilation::get_compilied_init_state() const {
  BDD res = sym_vars->oneBDD();

  // Initial state is a real state => no need to use primary rep.
  for (size_t var = 0; var < task.get_initial_state().size(); var++) {
    int val = task.get_initial_state()[var].get_value();

    if (!is_derived_variable(var)) {
      res *= sym_vars->preBDD(var, val);
    }
  }
  return res;
}

BDD SymAxiomCompilation::get_compilied_goal_state() const {
  BDD res = sym_vars->oneBDD();
  for (size_t i = 0; i < task.get_goals().size(); i++) {
    int var = task.get_goals()[i].get_variable().get_id();
    int val = task.get_goals()[i].get_value();
    if (is_derived_variable(var)) {
      res *= get_primary_representation(var, val);
    } else {
      res *= sym_vars->preBDD(var, val);
    }
  }
  return res;
}

BDD SymAxiomCompilation::get_primary_representation(int var, int val) const {
  if (!is_derived_variable(var)) {
    return sym_vars->preBDD(var, val);
  }

  BDD res = primary_representations.at(var);
  // Negation because default derived variable has default value
  return task.get_variables()[var].get_default_axiom_value() == val ? !res
                                                                    : res;
}

void SymAxiomCompilation::create_primary_representations() {
  create_axiom_body_layer();
  for (size_t i = 0; i < task.get_variables().size(); i++) {
    if (is_derived_variable(i)) {
      primary_representations[i] = sym_vars->zeroBDD();
    }
  }

  // Call for each layer the recursive procedure
  for (int i = 0; i < num_axiom_levels(); i++) {
    create_primary_representations(i);
  }
}

void SymAxiomCompilation::create_primary_representations(int layer) {
  std::cout << "LAYER " << layer << "..." << std::flush;
  std::vector<int> rules_in_layer;
  // add all "unproblematic" axioms to var bdd
  for (size_t i = 0; i < task.get_axioms().size(); i++) {
    if (is_trivial_axiom(i)) {
      continue;
    }
    auto axiom = task.get_axioms()[i];
    int head = axiom.get_effects()[0].get_fact().get_variable().get_id();

    if (get_axiom_level(i) == layer && axiom_body_layer.at(i) < layer) {
      BDD body = get_body_bdd(i);
      primary_representations[head] += body;
    }
  }
  // add vars of this layer to queue
  std::queue<int> open_vars;
  for (auto &cur : primary_representations) {
    int head = cur.first;
    int head_level = task.get_variables()[head].get_axiom_layer();
    if (head_level == layer) {
      // std::cout << g_variable_name[var] << std::endl;
      open_vars.push(head);
    }
  }

  // Recursively evaluate var bdd
  while (!open_vars.empty()) {
    int var = open_vars.front();
    open_vars.pop();
    for (size_t i = 0; i < task.get_axioms().size(); i++) {
      auto axiom = task.get_axioms()[i];
      int head = axiom.get_effects()[0].get_fact().get_variable().get_id();
      int head_level = task.get_variables()[head].get_axiom_layer();
      if (is_trivial_axiom(i) || head_level != layer) {
        continue;
      }
      if (is_in_body(var, i)) {

        BDD res = primary_representations[head];
        BDD body = get_body_bdd(i);
        res += (body * primary_representations[var]);
        if (res != primary_representations[head]) {
          open_vars.push(head);
        }
        primary_representations[head] = res;
        // std::cout << g_variable_name[head] << " updated" << std::endl;
      }
    }
  }
  std::cout << "done!" << std::endl;
}

void SymAxiomCompilation::create_axiom_body_layer() {
  for (size_t i = 0; i < task.get_axioms().size(); i++) {
    int body_level = -1;
    const EffectProxy eff = task.get_axioms()[i].get_effects()[0];
    for (size_t cond_i = 0; cond_i < eff.get_conditions().size(); cond_i++) {
      int level = eff.get_conditions()[cond_i].get_variable().get_axiom_layer();
      body_level = std::max(body_level, level);
    }
    axiom_body_layer.push_back(body_level);
  }
}

BDD SymAxiomCompilation::get_body_bdd(int axiom_id) const {
  BDD res = sym_vars->oneBDD();
  EffectProxy eff = task.get_axioms()[axiom_id].get_effects()[0];
  for (size_t cond_i = 0; cond_i < eff.get_conditions().size(); cond_i++) {
    int var = eff.get_conditions()[cond_i].get_variable().get_id();
    int val = eff.get_conditions()[cond_i].get_value();
    if (!is_derived_variable(var)) {
      res *= sym_vars->preBDD(var, val);
    } else {
      res *= get_primary_representation(var, val);
    }
  }
  return res;
}

} // namespace symbolic
