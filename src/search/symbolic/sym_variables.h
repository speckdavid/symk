#ifndef SYMBOLIC_SYM_VARIABLES_H
#define SYMBOLIC_SYM_VARIABLES_H

#include "sym_bucket.h"

#include "sym_axiom/sym_axiom_compilation.h"
#include "../tasks/root_task.h"
#include "../utils/timer.h"
#include "../sdac_parser/catamorph/expression.h"
#include "../sdac_parser/catamorph/catamorph.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <map>
#include <math.h>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>

class GlobalState;

namespace plugins {
class Feature;
class Options;
}

namespace symbolic {
/*
 * BDD-Variables for a symbolic exploration.
 * This information is global for every class using symbolic search.
 * The only decision fixed here is the variable ordering, which is assumed to be
 * always fixed.
 */
struct BDDError {};
extern void exceptionError(std::string message);

class SymVariables {
    // Use task_proxy to access task information.
    TaskProxy task_proxy;
    const std::shared_ptr<AbstractTask> task;

    // Var order used by the algorithm.
    // const VariableOrderType variable_ordering;
    // Parameters to initialize the CUDD manager
    const long cudd_init_nodes;          // Number of initial nodes
    const long cudd_init_cache_size;     // Initial cache size
    const long cudd_init_available_memory; // Maximum available memory (bytes)
    const bool gamer_ordering;
    const bool dynamic_reordering;

    Cudd *manager; // manager associated with this symbolic search
    std::shared_ptr<SymAxiomCompilation> ax_comp; // used for axioms

    int numBDDVars; // Number of binary variables (just one set, the total number is numBDDVars*2
    int numPrimaryBDDVars; // Number of binary variables that represent the primary variables
    std::vector<BDD> variables; // BDD variables
    std::vector<BDD> pre_variables; // Unprimed variables
    std::vector<BDD> eff_variables; // Primed variables
    std::vector<BDD> aux_variables; // Aux variables for conjunctive partioning
    BDD aux_cube;

    // The variable order must be complete.
    std::vector<int> var_order; // Variable(FD) order in the BDD
    std::vector<std::vector<int>> bdd_index_pre, bdd_index_eff; // vars(BDD) for each var(FD)

    std::vector<std::vector<BDD>> preconditionBDDs;   // BDDs associated with the precondition of a predicate
    std::vector<std::vector<BDD>> effectBDDs;   // BDDs associated with the effect of a predicate
    std::vector<BDD> biimpBDDs;   // BDDs associated with the biimplication of one variable(FD)
    std::vector<BDD> validValues;   // BDD that represents the valid values of all the variables
    BDD validBDD;  // BDD that represents the valid values of all the variables

    void init(const std::vector<int> &v_order);

public:
    SymVariables(const plugins::Options &opts, const std::shared_ptr<AbstractTask> &task);

    void init();

    std::shared_ptr<SymAxiomCompilation> get_axiom_compiliation() {
        return ax_comp;
    }

    double numStates(const BDD &bdd) const;

    std::vector<BDD> get_variables() {
        return variables;
    }

    std::vector<BDD> get_pre_variables() {
        return pre_variables;
    }

    std::vector<BDD> get_eff_variables() {
        return eff_variables;
    }

    State getStateFrom(const BDD &bdd) const;
    BDD getSinglePrimaryStateFrom(const BDD &bdd) const;
    BDD getStateBDD(const std::vector<int> &state) const;
    BDD getStateBDD(const State &state) const;

    BDD getPartialStateBDD(const std::vector<std::pair<int, int>> &state) const;

    inline const std::vector<int> &vars_index_pre(int variable) const {
        return bdd_index_pre[variable];
    }

    inline const std::vector<int> &vars_index_eff(int variable) const {
        return bdd_index_eff[variable];
    }

    inline const BDD &preBDD(int variable, int value) const {
        return preconditionBDDs[variable][value];
    }

    inline const BDD &effBDD(int variable, int value) const {
        return effectBDDs[variable][value];
    }


    BDD auxBDD(int variable, int value);

    inline BDD get_aux_cube() const {
        return aux_cube;
    }

    BDD get_aux_variables_in_support(BDD bdd) const;
    bool has_aux_variables_in_support(BDD bdd) const {
        return !get_aux_variables_in_support(bdd).IsOne();
    }

    void get_variable_value_bdds(const std::vector<int> &bdd_vars,
                                 int value,
                                 std::vector<BDD> &value_bdds) const;

    inline int get_num_aux_variables() const {
        return aux_variables.size();
    }

    inline BDD getCubePre(int var) const {return getCube(var, bdd_index_pre);}

    inline BDD getCubePre(const std::set<int> &vars) const {
        return getCube(vars, bdd_index_pre);
    }

    inline BDD getCubeEff(int var) const {return getCube(var, bdd_index_eff);}

    inline BDD getCubeEff(const std::set<int> &vars) const {
        return getCube(vars, bdd_index_eff);
    }

    inline const BDD &biimp(int variable) const {return biimpBDDs[variable];}

    inline std::vector<BDD> getBDDVarsPre() const {
        return getBDDVars(var_order, bdd_index_pre);
    }

    inline std::vector<BDD> getBDDVarsEff() const {
        return getBDDVars(var_order, bdd_index_eff);
    }

    inline std::vector<BDD> getBDDVarsPre(const std::vector<int> &vars) const {
        return getBDDVars(vars, bdd_index_pre);
    }

    inline std::vector<BDD> getBDDVarsEff(const std::vector<int> &vars) const {
        return getBDDVars(vars, bdd_index_eff);
    }

    inline BDD levelBDD(int level) const {
        return manager->bddVar(level);
    }

    inline BDD zeroBDD() const {
        return manager->bddZero();
    }

    inline BDD oneBDD() const {
        return manager->bddOne();
    }

    ADD constant(double c) const {
        return manager->constant(c);
    }

    inline BDD validStates() const {return validBDD;}

    inline BDD bddVar(int index) const {return variables[index];}

    inline void set_time_limit(int maxTime) {
        if (maxTime > 0) {
            manager->SetTimeLimit(maxTime);
            manager->ResetStartTime();
        }
    }

    inline void unset_time_limit() {
        manager->UnsetTimeLimit();
    }

    long forest_node_count() const {
        return manager->ReadNodeCount();
    }

    void reoder(int max_time);

    void to_dot(const BDD &bdd, const std::string &file_name) const;
    void to_dot(const ADD &bdd, const std::string &file_name) const;

    static void add_options_to_feature(plugins::Feature &feature);

    void print_options() const;

private:
    // Auxiliar function helping to create precondition and effect BDDs
    // Generates value for bddVars.
    BDD generateBDDVar(const std::vector<int> &_bddVars, int value) const;
    BDD getCube(int var, const std::vector<std::vector<int>> &v_index) const;
    BDD getCube(const std::set<int> &vars,
                const std::vector<std::vector<int>> &v_index) const;
    BDD createBiimplicationBDD(const std::vector<int> &vars,
                               const std::vector<int> &vars2) const;
    std::vector<BDD>
    getBDDVars(const std::vector<int> &vars,
               const std::vector<std::vector<int>> &v_index) const;

    inline BDD createPreconditionBDD(int variable, int value) const {
        return generateBDDVar(bdd_index_pre[variable], value);
    }

    inline BDD createEffectBDD(int variable, int value) const {
        return generateBDDVar(bdd_index_eff[variable], value);
    }

    inline int getNumBDDVars() const {return numBDDVars;}
};
}

#endif
