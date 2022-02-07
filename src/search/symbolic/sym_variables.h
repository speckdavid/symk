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

namespace options {
class Options;
class OptionParser;
} // namespace options

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

    Cudd *manager; // manager associated with this symbolic search
    std::shared_ptr<SymAxiomCompilation> ax_comp; // used for axioms

    int numBDDVars; // Number of binary variables (just one set, the total number
    // is numBDDVars*3
    std::vector<BDD> variables; // BDD variables

    // The variable order must be complete.
    std::vector<int> var_order; // Variable(FD) order in the BDD
    std::vector<std::vector<int>> bdd_index_pre, bdd_index_eff,
                                  bdd_index_abs; // vars(BDD) for each var(FD)

    std::vector<std::vector<BDD>>
    preconditionBDDs;   // BDDs associated with the precondition of a predicate
    std::vector<std::vector<BDD>>
    effectBDDs;   // BDDs associated with the effect of a predicate
    std::vector<BDD>
    biimpBDDs;   // BDDs associated with the biimplication of one variable(FD)
    std::vector<BDD>
    validValues;   // BDD that represents the valid values of all the variables
    BDD validBDD;  // BDD that represents the valid values of all the variables

    // Vector to store the binary description of an state
    // Avoid allocating memory during heuristic evaluation
    std::vector<int> binState;

    void init(const std::vector<int> &v_order);

public:
    SymVariables(const options::Options &opts,
                 const std::shared_ptr<AbstractTask> &task);

    void init();

    std::shared_ptr<SymAxiomCompilation> get_axiom_compiliation() {
        return ax_comp;
    }

    double numStates(const BDD &bdd) const {
        return bdd.CountMinterm(numBDDVars);
    }

    State getStateFrom(const BDD &bdd) const;
    BDD getStateBDD(const std::vector<int> &state) const;
    BDD getStateBDD(const GlobalState &state) const;

    BDD getPartialStateBDD(const std::vector<std::pair<int, int>> &state) const;

    inline const std::vector<int> &vars_index_pre(int variable) const {
        return bdd_index_pre[variable];
    }

    inline const std::vector<int> &vars_index_eff(int variable) const {
        return bdd_index_eff[variable];
    }

    inline const std::vector<int> &vars_index_abs(int variable) const {
        return bdd_index_abs[variable];
    }

    inline const BDD &preBDD(int variable, int value) const {
        return preconditionBDDs[variable][value];
    }

    inline const BDD &effBDD(int variable, int value) const {
        return effectBDDs[variable][value];
    }

    inline BDD getCubePre(int var) const {return getCube(var, bdd_index_pre);}

    inline BDD getCubePre(const std::set<int> &vars) const {
        return getCube(vars, bdd_index_pre);
    }

    inline BDD getCubeEff(int var) const {return getCube(var, bdd_index_eff);}

    inline BDD getCubeEff(const std::set<int> &vars) const {
        return getCube(vars, bdd_index_eff);
    }

    inline BDD getCubeAbs(int var) const {return getCube(var, bdd_index_abs);}

    inline BDD getCubeAbs(const std::set<int> &vars) const {
        return getCube(vars, bdd_index_abs);
    }

    inline const BDD &biimp(int variable) const {return biimpBDDs[variable];}

    inline std::vector<BDD> getBDDVarsPre() const {
        return getBDDVars(var_order, bdd_index_pre);
    }

    inline std::vector<BDD> getBDDVarsEff() const {
        return getBDDVars(var_order, bdd_index_eff);
    }

    inline std::vector<BDD> getBDDVarsAbs() const {
        return getBDDVars(var_order, bdd_index_abs);
    }

    inline std::vector<BDD> getBDDVarsPre(const std::vector<int> &vars) const {
        return getBDDVars(vars, bdd_index_pre);
    }

    inline std::vector<BDD> getBDDVarsEff(const std::vector<int> &vars) const {
        return getBDDVars(vars, bdd_index_eff);
    }

    inline std::vector<BDD> getBDDVarsAbs(const std::vector<int> &vars) const {
        return getBDDVars(vars, bdd_index_abs);
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

    inline void setTimeLimit(int maxTime) {
        manager->SetTimeLimit(maxTime);
        manager->ResetStartTime();
    }

    inline void unsetTimeLimit() {manager->UnsetTimeLimit();}

    void to_dot(const BDD &bdd, const std::string &file_name) const;
    void to_dot(const ADD &bdd, const std::string &file_name) const;

    static void add_options_to_parser(options::OptionParser &parser);

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
