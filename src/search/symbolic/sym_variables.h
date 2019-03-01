#ifndef SYMBOLIC_SYM_VARIABLES_H
#define SYMBOLIC_SYM_VARIABLES_H

#include "sym_bucket.h"

#include "../utils/timer.h"
#include <math.h>
#include <memory>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <string>
#include <map>
#include <cassert>

namespace options {
class Options;
class OptionParser;
}

namespace symbolic {

class SymVariables {
    // Var order used by the algorithm.
    //const VariableOrderType variable_ordering;
    //Parameters to initialize the CUDD manager
    const long cudd_init_nodes; //Number of initial nodes
    const long cudd_init_cache_size; //Initial cache size
    const long cudd_init_available_memory; //Maximum available memory (bytes)
    const bool gamer_ordering;

    int numBDDVars; //Number of binary variables (just one set, the total number is numBDDVars*3
    std::vector<Bdd> variables; // BDD variables

    //The variable order must be complete.
    std::vector <int> var_order; //Variable(FD) order in the BDD
    std::vector <std::vector <int>> bdd_index_pre, bdd_index_eff, bdd_index_abs; //vars(BDD) for each var(FD)
    
    std::vector <std::vector <Bdd>> preconditionBDDs; // BDDs associated with the precondition of a predicate
    std::vector <std::vector <Bdd>> effectBDDs;      // BDDs associated with the effect of a predicate
    std::vector<Bdd> biimpBDDs;  //BDDs associated with the biimplication of one variable(FD)
    std::vector<Bdd> validValues; // BDD that represents the valid values of all the variables
    Bdd validBDD;  // BDD that represents the valid values of all the variables


    //Vector to store the binary description of an state
    //Avoid allocating memory during heuristic evaluation
    std::vector <int> binState;

    void init(const std::vector <int> &v_order);

public:
    SymVariables(const options::Options &opts);
    void init();

    //State getStateFrom(const Bdd & bdd) const;
    Bdd getStateBDD(const std::vector<int> &state) const;

    Bdd getPartialStateBDD(const std::vector<std::pair<int, int>> &state) const;

    inline const std::vector<int> &vars_index_pre(int variable) const {
        return bdd_index_pre[variable];
    }
    inline const std::vector<int> &vars_index_eff(int variable) const {
        return bdd_index_eff[variable];
    }
    inline const std::vector<int> &vars_index_abs(int variable) const {
        return bdd_index_abs[variable];
    }

    inline const Bdd &preBDD(int variable, int value) const {
        return preconditionBDDs [variable] [value];
    }

    inline const Bdd &effBDD(int variable, int value) const {
        return effectBDDs [variable] [value];
    }


    inline Bdd getCubePre(int var) const {
        return getCube(var, bdd_index_pre);
    }
    inline Bdd getCubePre(const std::set <int> &vars) const {
        return getCube(vars, bdd_index_pre);
    }

    inline Bdd getCubeEff(int var) const {
        return getCube(var, bdd_index_eff);
    }
    inline Bdd getCubeEff(const std::set <int> &vars) const {
        return getCube(vars, bdd_index_eff);
    }


    inline Bdd getCubeAbs(int var) const {
        return getCube(var, bdd_index_abs);
    }
    inline Bdd getCubeAbs(const std::set <int> &vars) const {
        return getCube(vars, bdd_index_abs);
    }


    inline const Bdd &biimp(int variable) const {
        return biimpBDDs[variable];
    }

    inline std::vector <Bdd> getBDDVarsPre() const {
        return getBDDVars(var_order, bdd_index_pre);
    }
    inline std::vector <Bdd> getBDDVarsEff() const {
        return getBDDVars(var_order, bdd_index_eff);
    }
    inline std::vector <Bdd> getBDDVarsAbs() const {
        return getBDDVars(var_order, bdd_index_abs);
    }
    inline std::vector <Bdd> getBDDVarsPre(const std::vector <int> &vars) const {
        return getBDDVars(vars, bdd_index_pre);
    }
    inline std::vector <Bdd> getBDDVarsEff(const std::vector <int> &vars) const {
        return getBDDVars(vars, bdd_index_eff);
    }
    inline std::vector <Bdd> getBDDVarsAbs(const std::vector <int> &vars) const {
        return getBDDVars(vars, bdd_index_abs);
    }

    inline Bdd zeroBDD() const {
        return Bdd::BddZero();
    }

    inline Bdd oneBDD() const {
        return Bdd::BddOne();
    }

    inline Bdd validStates() const {
        return validBDD;
    }

    inline Bdd bddVar(int index) const {
        return variables[index];
    }

    inline void setTimeLimit(int maxTime) {
        Bdd::set_time_limit(maxTime);
        Bdd::reset_start_time();
    }

    inline void unsetTimeLimit() {
        Bdd::unset_time_limit();
    }
    
    template <class T> 
    int *getBinaryDescription(const T &state) {
        int pos = 0;
        //  cout << "State " << endl;
        for (int v : var_order) {
            //cout << v << "=" << state[v] << " " << g_variable_domain[v] << " assignments and  " << binary_len[v] << " variables   " ;
            //preconditionBDDs[v] [state[v]].PrintMinterm();

            for (size_t j = 0; j < bdd_index_pre[v].size(); j++) {
                binState[pos++] = ((state[v] >> j) % 2);
                binState[pos++] = 0; //Skip interleaving variable
            }
        }
        /* cout << "Binary description: ";
           for(int i = 0; i < pos; i++){
           cout << binState[i];
           }
           cout << endl;*/

        return &(binState[0]);
    }

    static void add_options_to_parser(options::OptionParser &parser);

    void print_options() const;


private:
    //Auxiliar function helping to create precondition and effect BDDs
    //Generates value for bddVars.
    Bdd generateBDDVar(const std::vector<int> &_bddVars, int value) const;
    Bdd getCube(int var, const std::vector<std::vector<int>> &v_index) const;
    Bdd getCube(const std::set <int> &vars, const std::vector<std::vector<int>> &v_index) const;
    Bdd createBiimplicationBDD(const std::vector<int> &vars, const std::vector<int> &vars2) const;
    std::vector <Bdd> getBDDVars(const std::vector <int> &vars, const std::vector<std::vector<int>> &v_index) const;


    inline Bdd createPreconditionBDD(int variable, int value) const {
        return generateBDDVar(bdd_index_pre[variable], value);
    }

    inline Bdd createEffectBDD(int variable, int value) const {
        return generateBDDVar(bdd_index_eff[variable], value);
    }

    inline int getNumBDDVars() const {
        return numBDDVars;
    }
};
}

#endif