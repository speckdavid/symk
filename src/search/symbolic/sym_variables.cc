#include "sym_variables.h"

#include "../global_state.h"
#include "../options/option_parser.h"
#include "../options/options.h"
#include "../task_utils/task_properties.h"
#include "../utils/logging.h"
#include "opt_order.h"
#include "sym_axiom/sym_axiom_compilation.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;
using options::Options;

namespace symbolic {
void exceptionError(string /*message*/) {
    // utils::g_log << message << endl;
    throw BDDError();
}

SymVariables::SymVariables(const Options &opts,
                           const shared_ptr<AbstractTask> &task)
    : task_proxy(*task), task(task),
      cudd_init_nodes(16000000L), cudd_init_cache_size(16000000L),
      cudd_init_available_memory(0L),
      gamer_ordering(opts.get<bool>("gamer_ordering")),
      ax_comp(make_shared<SymAxiomCompilation>(this, task)) {}

void SymVariables::init() {
    vector<int> var_order;
    if (gamer_ordering) {
        InfluenceGraph::compute_gamer_ordering(var_order, task);
    } else {
        for (size_t i = 0; i < task_proxy.get_variables().size(); ++i) {
            var_order.push_back(i);
        }
    }
    utils::g_log << "Sym variable order: ";
    for (int v : var_order)
        utils::g_log << v << " ";
    utils::g_log << endl;

    init(var_order);
}

// Constructor that makes use of global variables to initialize the
// symbolic_search structures

void SymVariables::init(const vector<int> &v_order) {
    utils::g_log << "Initializing Symbolic Variables" << endl;
    var_order = vector<int>(v_order);
    int num_fd_vars = var_order.size();

    // Initialize binary representation of variables.
    numBDDVars = 0;
    bdd_index_pre = vector<vector<int>>(v_order.size());
    bdd_index_eff = vector<vector<int>>(v_order.size());
    bdd_index_abs = vector<vector<int>>(v_order.size());
    int _numBDDVars = 0; // numBDDVars;
    for (int var : var_order) {
        int var_len = ceil(log2(task_proxy.get_variables()[var].get_domain_size()));
        numBDDVars += var_len;
        for (int j = 0; j < var_len; j++) {
            bdd_index_pre[var].push_back(_numBDDVars);
            bdd_index_eff[var].push_back(_numBDDVars + 1);
            _numBDDVars += 2;
        }
    }
    utils::g_log << "Num variables: " << var_order.size() << " => " << numBDDVars << endl;

    // Initialize manager
    utils::g_log << "Initialize Symbolic Manager(" << _numBDDVars << ", "
                 << cudd_init_nodes / _numBDDVars << ", " << cudd_init_cache_size << ", "
                 << cudd_init_available_memory << ")" << endl;
    manager =
        new Cudd(_numBDDVars, 0, cudd_init_nodes / _numBDDVars,
                 cudd_init_cache_size, cudd_init_available_memory);

    manager->setHandler(exceptionError);
    manager->setTimeoutHandler(exceptionError);
    manager->setNodesExceededHandler(exceptionError);

    utils::g_log << "Generating binary variables" << endl;
    // Generate binary_variables
    for (int i = 0; i < _numBDDVars; i++) {
        variables.push_back(manager->bddVar(i));
    }

    preconditionBDDs.resize(num_fd_vars);
    effectBDDs.resize(num_fd_vars);
    biimpBDDs.resize(num_fd_vars);
    validValues.resize(num_fd_vars);
    validBDD = oneBDD();
    // Generate predicate (precondition (s) and effect (s')) BDDs
    for (int var : var_order) {
        for (int j = 0; j < task_proxy.get_variables()[var].get_domain_size();
             j++) {
            preconditionBDDs[var].push_back(createPreconditionBDD(var, j));
            effectBDDs[var].push_back(createEffectBDD(var, j));
        }
        validValues[var] = zeroBDD();
        for (int j = 0; j < task_proxy.get_variables()[var].get_domain_size();
             j++) {
            validValues[var] += preconditionBDDs[var][j];
        }
        validBDD *= validValues[var];
        biimpBDDs[var] =
            createBiimplicationBDD(bdd_index_pre[var], bdd_index_eff[var]);
    }

    binState.resize(_numBDDVars, 0);

    utils::g_log << "Symbolic Variables... Done." << endl;

    if (task_properties::has_axioms(task_proxy)) {
        utils::g_log << "Creating Primary Representation for Derived Predicates..."
                     << endl;
        ax_comp->init_axioms();
        utils::g_log << "Primary Representation... Done!" << endl;
    }
}

State SymVariables::getStateFrom(const BDD &bdd) const {
    vector<int> vals;
    BDD current = bdd;
    for (int var = 0; var < tasks::g_root_task->get_num_variables(); var++) {
        for (int val = 0; val < tasks::g_root_task->get_variable_domain_size(var);
             val++) {
            BDD aux = current * preconditionBDDs[var][val];
            if (!aux.IsZero()) {
                current = aux;
                vals.push_back(val);
                break;
            }
        }
    }
    return State(*tasks::g_root_task, move(vals));
}

BDD SymVariables::getStateBDD(const vector<int> &state) const {
    BDD res = oneBDD();
    for (int i = var_order.size() - 1; i >= 0; i--) {
        res = res * preconditionBDDs[var_order[i]][state[var_order[i]]];
    }
    return res;
}

BDD SymVariables::getStateBDD(const GlobalState &state) const {
    BDD res = oneBDD();
    for (int i = var_order.size() - 1; i >= 0; i--) {
        res = res * preconditionBDDs[var_order[i]][state[var_order[i]]];
    }
    return res;
}

BDD SymVariables::getPartialStateBDD(
    const vector<pair<int, int>> &state) const {
    BDD res = validBDD;
    for (int i = state.size() - 1; i >= 0; i--) {
        // if(find(var_order.begin(), var_order.end(),
        //               state[i].first) != var_order.end()) {
        res = res * preconditionBDDs[state[i].first][state[i].second];
        //}
    }
    return res;
}

BDD SymVariables::generateBDDVar(const vector<int> &_bddVars,
                                 int value) const {
    BDD res = oneBDD();
    for (int v : _bddVars) {
        if (value % 2) { // Check if the binary variable is asserted or negated
            res = res * variables[v];
        } else {
            res = res * (!variables[v]);
        }
        value /= 2;
    }
    return res;
}

BDD SymVariables::createBiimplicationBDD(const vector<int> &vars,
                                         const vector<int> &vars2) const {
    BDD res = oneBDD();
    for (size_t i = 0; i < vars.size(); i++) {
        res *= variables[vars[i]].Xnor(variables[vars2[i]]);
    }
    return res;
}

vector<BDD> SymVariables::getBDDVars(const vector<int> &vars,
                                     const vector<vector<int>> &v_index) const {
    vector<BDD> res;
    for (int v : vars) {
        for (int bddv : v_index[v]) {
            res.push_back(variables[bddv]);
        }
    }
    return res;
}

BDD SymVariables::getCube(int var, const vector<vector<int>> &v_index) const {
    BDD res = oneBDD();
    for (int bddv : v_index[var]) {
        res *= variables[bddv];
    }
    return res;
}

BDD SymVariables::getCube(const set<int> &vars,
                          const vector<vector<int>> &v_index) const {
    BDD res = oneBDD();
    for (int v : vars) {
        for (int bddv : v_index[v]) {
            res *= variables[bddv];
        }
    }
    return res;
}

void SymVariables::to_dot(const BDD &bdd,
                          const string &file_name) const {
    to_dot(bdd.Add(), file_name);
}

void SymVariables::to_dot(const ADD &add,
                          const string &file_name) const {
    vector<string> var_names(numBDDVars * 2);
    for (int v : var_order) {
        int exp = 0;
        for (int j : bdd_index_pre[v]) {
            var_names[j] = tasks::g_root_task->get_variable_name(v) + "_2^" +
                to_string(exp);
            var_names[j + 1] = tasks::g_root_task->get_variable_name(v) + "_2^" +
                to_string(exp++) + "_primed";
        }
    }

    vector<char *> names(numBDDVars * 2);
    for (int i = 0; i < numBDDVars * 2; ++i) {
        names[i] = &var_names[i].front();
    }
    FILE *outfile = fopen(file_name.c_str(), "w");
    DdNode **ddnodearray = (DdNode **)malloc(sizeof(add.getNode()));
    ddnodearray[0] = add.getNode();
    Cudd_DumpDot(manager->getManager(), 1, ddnodearray, names.data(), NULL,
                 outfile); // dump the function to .dot file
    free(ddnodearray);
    fclose(outfile);
}


void SymVariables::print_options() const {
    utils::g_log << "CUDD Init: nodes=" << cudd_init_nodes
                 << " cache=" << cudd_init_cache_size
                 << " max_memory=" << cudd_init_available_memory
                 << " ordering: " << (gamer_ordering ? "gamer" : "fd") << endl;
}

void SymVariables::add_options_to_parser(options::OptionParser &parser) {
    parser.add_option<bool>("gamer_ordering", "Use Gamer ordering optimization",
                            "true");
}
}
