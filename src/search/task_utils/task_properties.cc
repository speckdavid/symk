#include "task_properties.h"

#include "../utils/logging.h"
#include "../utils/system.h"

#include <algorithm>
#include <iostream>
#include <limits>

using namespace std;
using utils::ExitCode;


namespace task_properties {
bool is_unit_cost(TaskProxy task) {
    for (OperatorProxy op : task.get_operators()) {
        if (op.get_cost() != 1)
            return false;
    }
    return true;
}

bool has_axioms(TaskProxy task) {
    return !task.get_axioms().empty();
}

void verify_no_axioms(TaskProxy task) {
    if (has_axioms(task)) {
        cerr << "This configuration does not support axioms!"
             << endl << "Terminating." << endl;
        utils::exit_with(ExitCode::SEARCH_UNSUPPORTED);
    }
}


static int get_first_conditional_effects_op_id(TaskProxy task) {
    for (OperatorProxy op : task.get_operators()) {
        for (EffectProxy effect : op.get_effects()) {
            if (!effect.get_conditions().empty())
                return op.get_id();
        }
    }
    return -1;
}

bool has_conditional_effects(TaskProxy task) {
    return get_first_conditional_effects_op_id(task) != -1;
}

extern bool has_conditional_effects(TaskProxy task, OperatorID op_id) {
    OperatorProxy op = task.get_operators()[op_id];
    for (EffectProxy effect : op.get_effects()) {
        if (!effect.get_conditions().empty())
            return true;
    }
    return false;
}

void verify_no_conditional_effects(TaskProxy task) {
    int op_id = get_first_conditional_effects_op_id(task);
    if (op_id != -1) {
        OperatorProxy op = task.get_operators()[op_id];
        cerr << "This configuration does not support conditional effects "
             << "(operator " << op.get_name() << ")!" << endl
             << "Terminating." << endl;
        utils::exit_with(ExitCode::SEARCH_UNSUPPORTED);
    }
}

bool has_zero_cost_operator(TaskProxy task_proxy) {
    for (OperatorProxy op : task_proxy.get_operators()) {
        if (op.get_cost() == 0) {
            return true;
        }
    }
    return false;
}

bool has_sdac_cost_operator(TaskProxy task_proxy) {
    for (OperatorProxy op : task_proxy.get_operators()) {
        bool sdac = op.get_cost_function().find_first_not_of("0123456789")
            != string::npos;
        if (sdac) {
            return true;
        }
    }
    return false;
}

void verify_no_zero_cost_operator(TaskProxy task_proxy) {
    if (has_zero_cost_operator(task_proxy)) {
        cerr << "This configuration does not support zero operation cost "
             << endl << "Terminating." << endl;
        utils::exit_with(ExitCode::SEARCH_UNSUPPORTED);
    }
}

vector<int> get_operator_costs(const TaskProxy &task_proxy) {
    vector<int> costs;
    OperatorsProxy operators = task_proxy.get_operators();
    costs.reserve(operators.size());
    for (OperatorProxy op : operators)
        costs.push_back(op.get_cost());
    return costs;
}

double get_average_operator_cost(TaskProxy task_proxy) {
    double average_operator_cost = 0;
    for (OperatorProxy op : task_proxy.get_operators()) {
        average_operator_cost += op.get_cost();
    }
    average_operator_cost /= task_proxy.get_operators().size();
    return average_operator_cost;
}

int get_min_operator_cost(TaskProxy task_proxy) {
    int min_cost = numeric_limits<int>::max();
    for (OperatorProxy op : task_proxy.get_operators()) {
        min_cost = min(min_cost, op.get_cost());
    }
    return min_cost;
}

int get_max_operator_cost(TaskProxy task_proxy) {
    int max_cost = 0;
    for (OperatorProxy op : task_proxy.get_operators()) {
        max_cost = max(max_cost, op.get_cost());
        // std::cout << op.get_name() << ": " << op.get_cost() << std::endl;
    }
    return max_cost;
}

int get_num_facts(const TaskProxy &task_proxy) {
    int num_facts = 0;
    for (VariableProxy var : task_proxy.get_variables())
        num_facts += var.get_domain_size();
    return num_facts;
}

int get_num_total_effects(const TaskProxy &task_proxy) {
    int num_effects = 0;
    for (OperatorProxy op : task_proxy.get_operators())
        num_effects += op.get_effects().size();
    num_effects += task_proxy.get_axioms().size();
    return num_effects;
}

void print_variable_statistics(const TaskProxy &task_proxy) {
    const int_packer::IntPacker &state_packer = g_state_packers[task_proxy];

    int num_facts = 0;
    VariablesProxy variables = task_proxy.get_variables();
    for (VariableProxy var : variables)
        num_facts += var.get_domain_size();

    utils::g_log << "Variables: " << variables.size() << endl;
    utils::g_log << "FactPairs: " << num_facts << endl;
    utils::g_log << "Bytes per state: "
                 << state_packer.get_num_bins() * sizeof(int_packer::IntPacker::Bin)
                 << endl;
}

void dump_pddl(const State &state) {
    for (FactProxy fact : state) {
        string fact_name = fact.get_name();
        if (fact_name != "<none of those>")
            utils::g_log << fact_name << endl;
    }
}

void dump_fdr(const State &state) {
    for (FactProxy fact : state) {
        VariableProxy var = fact.get_variable();
        utils::g_log << "  #" << var.get_id() << " [" << var.get_name() << "] -> "
                     << fact.get_value() << endl;
    }
}

void dump_goals(const GoalsProxy &goals) {
    utils::g_log << "Goal conditions:" << endl;
    for (FactProxy goal : goals) {
        utils::g_log << "  " << goal.get_variable().get_name() << ": "
                     << goal.get_value() << endl;
    }
}

static void dump_operator(const OperatorProxy &op) {
    utils::g_log << "#" << op.get_id() << ": " << op.get_name() << endl;
    utils::g_log << " pre: [ " << flush;

    for (FactProxy pre : op.get_preconditions()) {
        utils::g_log << pre.get_pair() << " " << flush;
    }
    utils::g_log << "]" << endl;

    utils::g_log << " eff:" << endl;
    for (EffectProxy eff : op.get_effects()) {
        utils::g_log << " " << eff.get_fact().get_pair() << " if [ " << flush;
        for (auto cond: eff.get_conditions()) {
            utils::g_log << cond.get_pair() << " " << flush;
        }
        utils::g_log << "]" << endl;
    }
}

static void dump_operators(const OperatorsProxy &operators) {
    for (OperatorProxy op : operators) {
        dump_operator(op);
    }
}

static void dump_axioms(const AxiomsProxy &axioms) {
    for (OperatorProxy ax : axioms) {
        dump_operator(ax);
    }
}

void dump_task(const TaskProxy &task_proxy, bool with_operators, bool with_axioms) {
    OperatorsProxy operators = task_proxy.get_operators();
    int min_action_cost = numeric_limits<int>::max();
    int max_action_cost = 0;
    for (OperatorProxy op : operators) {
        min_action_cost = min(min_action_cost, op.get_cost());
        max_action_cost = max(max_action_cost, op.get_cost());
    }
    utils::g_log << "Min action cost: " << min_action_cost << endl;
    utils::g_log << "Max action cost: " << max_action_cost << endl;

    VariablesProxy variables = task_proxy.get_variables();
    utils::g_log << "Variables (" << variables.size() << "):" << endl;
    int var_id = 0;
    for (VariableProxy var : variables) {
        utils::g_log << "  #" << var_id << ": " << var.get_name()
                     << " (range " << var.get_domain_size() << ")" << endl;
        for (int val = 0; val < var.get_domain_size(); ++val) {
            utils::g_log << "    " << val << ": " << var.get_fact(val).get_name() << endl;
        }
        ++var_id;
    }
    State initial_state = task_proxy.get_initial_state();
    utils::g_log << "Initial state (PDDL):" << endl;
    dump_pddl(initial_state);
    utils::g_log << "Initial state (FDR):" << endl;
    dump_fdr(initial_state);
    dump_goals(task_proxy.get_goals());
    if (with_operators) {
        utils::g_log << "Operators:" << endl;
        dump_operators(task_proxy.get_operators());
    }
    if (with_axioms) {
        utils::g_log << "Axioms:" << endl;
        dump_axioms(task_proxy.get_axioms());
    }
}

PerTaskInformation<int_packer::IntPacker> g_state_packers(
    [](const TaskProxy &task_proxy) {
        VariablesProxy variables = task_proxy.get_variables();
        vector<int> variable_ranges;
        variable_ranges.reserve(variables.size());
        for (VariableProxy var : variables) {
            variable_ranges.push_back(var.get_domain_size());
        }
        return make_unique<int_packer::IntPacker>(variable_ranges);
    }
    );
}
