#ifndef SEARCH_SYMBOLIC_SYM_AXIOM_SYM_AXIOM_COMPILATION_H_
#define SEARCH_SYMBOLIC_SYM_AXIOM_SYM_AXIOM_COMPILATION_H_

#include "../../task_proxy.h"
#include "cuddObj.hh"
#include <map>
#include <memory>

namespace symbolic {
class SymVariables;

class SymAxiomCompilation {
public:
    // We need to use a raw pointer of sym_vars. Otherwise we get a cylic pointer
    // issue as SymAxiomComplation only lives in SymVariables
    SymAxiomCompilation(SymVariables *sym_vars, const std::shared_ptr<AbstractTask> &task);

    bool is_derived_variable(int var) const;
    bool is_in_body(int var, int axiom_id) const;

    bool is_trivial_axiom(int axiom_id) const;
    int get_axiom_level(VariableProxy var) const;
    int get_axiom_level(int axiom_id) const;
    int num_axiom_levels() const;

    void init_axioms();
    BDD get_compilied_init_state() const;
    BDD get_compilied_goal_state() const;
    BDD get_primary_representation(int var, int val) const;

protected:
    SymVariables *sym_vars; // For axiom creation
    TaskProxy task_proxy;
    std::vector<int> axiom_body_layer;
    std::map<int, BDD> primary_representations;

    void create_primary_representations();
    void create_primary_representations(int layer);
    void create_axiom_body_layer();
    BDD get_body_bdd(int axiom_id) const;
};
}

#endif
