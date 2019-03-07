#include "sym_variables.h"

#include <sstream>
#include <iostream>
#include <fstream>
#include <string>

#include "../options/options.h"
#include "../options/option_parser.h"
#include "opt_order.h"
#include "debug_macros.h"
#include "../tasks/root_task.h"
#include "../global_state.h"

using namespace std;
using options::Options;

namespace symbolic
{
SymVariables::SymVariables(const Options &opts) : cudd_init_nodes(16000000L),
                                                  cudd_init_cache_size(16000000L),
                                                  cudd_init_available_memory(0L),
                                                  gamer_ordering(opts.get<bool>("gamer_ordering"))
{
}

void SymVariables::init()
{
    vector<int> var_order;
    if (gamer_ordering)
    {
        InfluenceGraph::compute_gamer_ordering(var_order);
    }
    else
    {
        for (int i = 0; i < tasks::g_root_task->get_num_variables(); ++i)
        {
            var_order.push_back(i);
        }
    }
    cout << "Sym variable order: ";
    for (int v : var_order)
        cout << v << " ";
    cout << endl;

    init(var_order);
}

//Constructor that makes use of global variables to initialize the symbolic_search structures
void SymVariables::init(const vector<int> &v_order)
{
    cout << "Initializing Symbolic Variables" << endl;
    var_order = vector<int>(v_order);
    int num_fd_vars = var_order.size();

    //Initialize binary representation of variables.
    numBDDVars = 0;
    bdd_index_pre = vector<vector<int>>(v_order.size());
    bdd_index_eff = vector<vector<int>>(v_order.size());
    bdd_index_abs = vector<vector<int>>(v_order.size());
    int _numBDDVars = 0; // numBDDVars;
    for (int var : var_order)
    {
        int var_len = ceil(log2(tasks::g_root_task->get_variable_domain_size(var)));
        numBDDVars += var_len;
        for (int j = 0; j < var_len; j++)
        {
            bdd_index_pre[var].push_back(_numBDDVars);
            bdd_index_eff[var].push_back(_numBDDVars + 1);
            _numBDDVars += 2;
        }
    }
    cout << "Num variables: " << var_order.size() << " => " << numBDDVars << endl;

    //Initialize manager
    cout << "Initialize Symbolic Manager(" << _numBDDVars << ", "
         << cudd_init_nodes / _numBDDVars << ", "
         << cudd_init_cache_size << ", "
         << cudd_init_available_memory << ")" << endl;
    Bdd::initalize_manager(_numBDDVars, 0, cudd_init_nodes / _numBDDVars,
                                                     cudd_init_cache_size,
                                                     cudd_init_available_memory);

    cout << "Generating binary variables" << endl;
    //Generate binary_variables
    for (int i = 0; i < _numBDDVars; i++)
    {
        variables.push_back(Bdd::BddVar(i));
    }

    DEBUG_MSG(cout << "Generating predicate BDDs: " << num_fd_vars << endl;);
    preconditionBDDs.resize(num_fd_vars);
    effectBDDs.resize(num_fd_vars);
    biimpBDDs.resize(num_fd_vars);
    validValues.resize(num_fd_vars);
    validBDD = oneBDD();
    //Generate predicate (precondition (s) and effect (s')) BDDs
    for (int var : var_order)
    {
        for (int j = 0; j < tasks::g_root_task->get_variable_domain_size(var); j++)
        {
            preconditionBDDs[var].push_back(createPreconditionBDD(var, j));
            effectBDDs[var].push_back(createEffectBDD(var, j));
        }
        validValues[var] = zeroBDD();
        for (int j = 0; j < tasks::g_root_task->get_variable_domain_size(var); j++)
        {
            validValues[var] += preconditionBDDs[var][j];
        }
        validBDD *= validValues[var];
        biimpBDDs[var] = createBiimplicationBDD(bdd_index_pre[var], bdd_index_eff[var]);
    }

    binState.resize(_numBDDVars, 0);
    cout << "Symbolic Variables... Done." << endl;
}

Bdd SymVariables::getStateBDD(const std::vector<int> &state) const
{
    Bdd res = oneBDD();
    for (int i = var_order.size() - 1; i >= 0; i--)
    {
        res = res * preconditionBDDs[var_order[i]][state[var_order[i]]];
    }
    return res;
}

Bdd SymVariables::getStateBDD(const GlobalState &state) const
{
    Bdd res = oneBDD();
    for (int i = var_order.size() - 1; i >= 0; i--)
    {
        res = res * preconditionBDDs[var_order[i]][state[var_order[i]]];
    }
    return res;
}

Bdd SymVariables::getPartialStateBDD(const vector<pair<int, int>> &state) const
{
    Bdd res = validBDD;
    for (int i = state.size() - 1; i >= 0; i--)
    {
        // if(find(var_order.begin(), var_order.end(),
        //               state[i].first) != var_order.end()) {
        res = res * preconditionBDDs[state[i].first][state[i].second];
        //}
    }
    return res;
}

Bdd SymVariables::generateBDDVar(const std::vector<int> &_bddVars, int value) const
{
    Bdd res = oneBDD();
    for (int v : _bddVars)
    {
        if (value % 2)
        { //Check if the binary variable is asserted or negated
            res = res * variables[v];
        }
        else
        {
            res = res * (!variables[v]);
        }
        value /= 2;
    }
    return res;
}

Bdd SymVariables::createBiimplicationBDD(const std::vector<int> &vars, const std::vector<int> &vars2) const
{
    Bdd res = oneBDD();
    for (size_t i = 0; i < vars.size(); i++)
    {
        res *= variables[vars[i]].Xnor(variables[vars2[i]]);
    }
    return res;
}

vector<Bdd> SymVariables::getBDDVars(const vector<int> &vars, const vector<vector<int>> &v_index) const
{
    vector<Bdd> res;
    for (int v : vars)
    {
        for (int bddv : v_index[v])
        {
            res.push_back(variables[bddv]);
        }
    }
    return res;
}

Bdd SymVariables::getCube(int var, const vector<vector<int>> &v_index) const
{
    Bdd res = oneBDD();
    for (int bddv : v_index[var])
    {
        res *= variables[bddv];
    }
    return res;
}

Bdd SymVariables::getCube(const set<int> &vars, const vector<vector<int>> &v_index) const
{
    Bdd res = oneBDD();
    for (int v : vars)
    {
        for (int bddv : v_index[v])
        {
            res *= variables[bddv];
        }
    }
    return res;
}

void SymVariables::print_options() const
{
    cout << "CUDD Init: nodes=" << cudd_init_nodes << " cache=" << cudd_init_cache_size << " max_memory=" << cudd_init_available_memory << " ordering: " << (gamer_ordering ? "gamer" : "fd") << endl;
}

void SymVariables::add_options_to_parser(options::OptionParser &parser)
{
    // const std::vector<std::string> VariableOrderValues {
    //  "CG_GOAL_LEVEL", "CG_GOAL_RANDOM",
    //      "GOAL_CG_LEVEL", "RANDOM",
    //      "LEVEL", "REVERSE_LEVEL"};

    // parser.add_enum_option("var_order", VariableOrderValues,
    //                     "variable ordering for the symbolic manager", "REVERSE_LEVEL");

    /*parser.add_option<std::string> ("cudd_init_nodes", "Initial number of nodes in the cudd manager.",
                             "16000000L");

    parser.add_option<long> ("cudd_init_cache_size",
                             "Initial number of cache entries in the cudd manager.", "16000000L");

    parser.add_option<long> ("cudd_init_available_memory",
                             "Total available memory for the cudd manager.", "0L");*/
    parser.add_option<bool>("gamer_ordering", "Use Gamer ordering optimization", "true");
}
} // namespace symbolic
