#include "sym_axiom_compilation.h"
#include "../option_parser.h"
#include "../sym_variables.h"

#include "../../plugin.h"
#include "../../utils/logging.h"

#include <limits>
#include <queue>

namespace symbolic {
using namespace std;

SymAxiomCompilation::SymAxiomCompilation(
    SymVariables *sym_vars,
    const shared_ptr<AbstractTask> &task)
    : sym_vars(sym_vars), task_proxy(*task) {}

bool SymAxiomCompilation::is_derived_variable(int var) const {
    return task_proxy.get_variables()[var].is_derived();
}

bool SymAxiomCompilation::is_in_body(int var, int axiom_id) const {
    EffectProxy eff = task_proxy.get_axioms()[axiom_id].get_effects()[0];
    for (size_t cond_i = 0; cond_i < eff.get_conditions().size(); cond_i++) {
        if (eff.get_conditions()[cond_i].get_variable().get_id() == var) {
            return true;
        }
    }
    return false;
}

bool SymAxiomCompilation::is_trivial_axiom(int axiom_id) const {
    int head = task_proxy.get_axioms()[axiom_id]
        .get_effects()[0]
        .get_fact()
        .get_variable()
        .get_id();
    int val =
        task_proxy.get_axioms()[axiom_id].get_effects()[0].get_fact().get_value();
    int default_val = task_proxy.get_variables()[head].get_default_axiom_value();
    return val == default_val;
}

int SymAxiomCompilation::get_axiom_level(const VariableProxy var) const {
    if (!var.is_derived()) {
        return -1;
    }
    return var.get_axiom_layer();
}

int SymAxiomCompilation::get_axiom_level(int axiom_id) const {
    return get_axiom_level(task_proxy.get_axioms()[axiom_id]
                           .get_effects()[0]
                           .get_fact()
                           .get_variable());
}

int SymAxiomCompilation::num_axiom_levels() const {
    int num_level = -1;
    for (size_t i = 0; i < task_proxy.get_variables().size(); i++) {
        num_level = max(num_level, get_axiom_level(task_proxy.get_variables()[i]));
    }
    return num_level + 1;
}

void SymAxiomCompilation::init_axioms() {
    utils::Timer timer;
    create_primary_representations();

    utils::g_log << fixed << "Symbolic Axiom initialization: " << timer
                 << endl;
}

BDD SymAxiomCompilation::get_compilied_init_state() const {
    BDD res = sym_vars->oneBDD();

    // Initial state is a real state => no need to use primary rep.
    for (size_t var = 0; var < task_proxy.get_initial_state().size(); var++) {
        int val = task_proxy.get_initial_state()[var].get_value();

        if (!is_derived_variable(var)) {
            res *= sym_vars->preBDD(var, val);
        }
    }
    return res;
}

BDD SymAxiomCompilation::get_compilied_goal_state() const {
    BDD res = sym_vars->oneBDD();
    for (size_t i = 0; i < task_proxy.get_goals().size(); i++) {
        int var = task_proxy.get_goals()[i].get_variable().get_id();
        int val = task_proxy.get_goals()[i].get_value();
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
    return task_proxy.get_variables()[var].get_default_axiom_value() == val ? !res
           : res;
}

void SymAxiomCompilation::create_primary_representations() {
    create_axiom_body_layer();
    for (size_t i = 0; i < task_proxy.get_variables().size(); i++) {
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
    utils::g_log << "LAYER " << layer << "..." << flush;
    vector<int> rules_in_layer;
    // add all "unproblematic" axioms to var bdd
    for (size_t i = 0; i < task_proxy.get_axioms().size(); i++) {
        if (is_trivial_axiom(i)) {
            continue;
        }
        auto axiom = task_proxy.get_axioms()[i];
        int head = axiom.get_effects()[0].get_fact().get_variable().get_id();

        if (get_axiom_level(i) == layer && axiom_body_layer.at(i) < layer) {
            BDD body = get_body_bdd(i);
            primary_representations[head] += body;
        }
    }
    // add vars of this layer to queue
    queue<int> open_vars;
    for (auto &cur : primary_representations) {
        int head = cur.first;
        int head_level = get_axiom_level(task_proxy.get_variables()[head]);
        if (head_level == layer) {
            // utils::g_log << g_variable_name[var] << endl;
            open_vars.push(head);
        }
    }

    // Recursively evaluate var bdd
    while (!open_vars.empty()) {
        int var = open_vars.front();
        open_vars.pop();
        for (size_t i = 0; i < task_proxy.get_axioms().size(); i++) {
            auto axiom = task_proxy.get_axioms()[i];
            int head = axiom.get_effects()[0].get_fact().get_variable().get_id();
            int head_level = get_axiom_level(task_proxy.get_variables()[head]);
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
                // utils::g_log << g_variable_name[head] << " updated" << endl;
            }
        }
    }
    utils::g_log << "done!" << endl;
}

void SymAxiomCompilation::create_axiom_body_layer() {
    for (size_t i = 0; i < task_proxy.get_axioms().size(); i++) {
        int body_level = -1;
        const EffectProxy eff = task_proxy.get_axioms()[i].get_effects()[0];
        for (size_t cond_i = 0; cond_i < eff.get_conditions().size(); cond_i++) {
            int level = get_axiom_level(eff.get_conditions()[cond_i].get_variable());
            body_level = max(body_level, level);
        }
        axiom_body_layer.push_back(body_level);
    }
}

BDD SymAxiomCompilation::get_body_bdd(int axiom_id) const {
    BDD res = sym_vars->oneBDD();
    EffectProxy eff = task_proxy.get_axioms()[axiom_id].get_effects()[0];
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
