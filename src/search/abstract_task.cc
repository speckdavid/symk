#include "abstract_task.h"

#include "per_task_information.h"
#include "plugin.h"
#include "task_proxy.h"

#include <iostream>

using namespace std;

const FactPair FactPair::no_fact = FactPair(-1, -1);

void AbstractTask::dump_version(std::ostream &os) const
{
    os << "begin_version" << std::endl;
    os << 3 << std::endl;
    os << "end_version" << std::endl;
}

void AbstractTask::dump_metric(std::ostream &os) const
{
    os << "begin_metric" << std::endl;
    os << get_use_metric() << std::endl;
    os << "end_metric" << std::endl;
}

void AbstractTask::dump_operator_pre_post_to_SAS(
    std::ostream &os, int pre, const FactPair &eff,
    const std::vector<FactPair> &eff_conds) const
{
    os << eff_conds.size() << " ";
    for (FactPair cond : eff_conds)
    {
        os << cond.value << " " << cond.var << std::endl;
    }
    os << eff.var << " " << pre << " " << eff.value << std::endl;
}

void AbstractTask::dump_axioms_to_SAS(std::ostream &os) const
{
    os << get_num_axioms() << endl;
    for (int op_no = 0; op_no < get_num_axioms(); ++op_no)
    {
        dump_axiom_to_SAS(os, op_no);
    }
}

void AbstractTask::dump_axiom_to_SAS(std::ostream &os, int op_no) const
{
    os << "begin_rule" << endl;
    vector<FactPair> cond;
    for (int cond_ind = 0;
         cond_ind < get_num_operator_effect_conditions(op_no, 0, true);
         ++cond_ind)
    {
        FactPair fact = get_operator_effect_condition(op_no, 0, cond_ind, true);
        cond.push_back(fact);
    }
    FactPair eff = get_operator_effect(op_no, 0, true);
    int eff_pre_val = -1;
    for (int pre_ind = 0; pre_ind < get_num_operator_preconditions(op_no, true);
         ++pre_ind)
    {
        FactPair fact = get_operator_precondition(op_no, pre_ind, true);
        if (eff.var == fact.var)
        {
            eff_pre_val = fact.value;
            break;
        }
    }
    dump_operator_pre_post_to_SAS(os, eff_pre_val, eff, cond);
    os << "end_rule" << endl;
}

void AbstractTask::dump_operators_to_SAS(std::ostream &os) const
{
    os << get_num_operators() << endl;
    for (int op_no = 0; op_no < get_num_operators(); ++op_no)
    {
        dump_operator_to_SAS(os, op_no);
    }
}

void AbstractTask::dump_operator_to_SAS(std::ostream &os, int op_no) const
{
    vector<FactPair> prevail;
    vector<int> eff_pre_val;
    vector<FactPair> all_preconditions;
    vector<FactPair> all_effects;
    vector<vector<FactPair>> all_effects_condition;
    for (int pre_ind = 0; pre_ind < get_num_operator_preconditions(op_no, false);
         ++pre_ind)
    {
        FactPair fact = get_operator_precondition(op_no, pre_ind, false);
        all_preconditions.push_back(fact);
    }
    for (int eff_ind = 0; eff_ind < get_num_operator_effects(op_no, false);
         ++eff_ind)
    {
        vector<FactPair> cond;
        for (int cond_ind = 0;
             cond_ind < get_num_operator_effect_conditions(op_no, eff_ind, false);
             ++cond_ind)
        {
            FactPair fact =
                get_operator_effect_condition(op_no, eff_ind, cond_ind, false);
            cond.push_back(fact);
        }
        FactPair eff = get_operator_effect(op_no, eff_ind, false);
        all_effects.push_back(eff);
        all_effects_condition.push_back(cond);
    }
    eff_pre_val.assign(all_effects.size(), -1);
    for (FactPair c : all_preconditions)
    {
        // Checking if in any effect
        bool found = false;
        for (size_t i = 0; i < all_effects.size(); ++i)
        {
            const FactPair &eff = all_effects[i];
            if (eff.var != c.var)
                continue;
            found = true;
            eff_pre_val[i] = c.value;
        }

        if (!found)
            prevail.push_back(c);
    }

    os << "begin_operator" << endl;
    os << get_operator_name(op_no, false) << endl;
    os << prevail.size() << endl;
    for (FactPair cond : prevail)
    {
        os << cond.var << " " << cond.value << endl;
    }
    os << all_effects.size() << endl;
    for (size_t i = 0; i < all_effects.size(); ++i)
    {
        dump_operator_pre_post_to_SAS(os, eff_pre_val[i], all_effects[i],
                                      all_effects_condition[i]);
    }
    os << get_operator_cost(op_no, false) << endl;
    os << "end_operator" << endl;
}

void AbstractTask::dump_variables_to_SAS(std::ostream &os) const
{
    os << get_num_variables() << endl;
    for (int var = 0; var < get_num_variables(); ++var)
    {
        vector<string> vals;
        for (int val = 0; val < get_variable_domain_size(var); ++val)
        {
            vals.push_back(get_fact_name(FactPair(var, val)));
        }
        os << "begin_variable" << endl;
        os << get_variable_name(var) << endl;
        os << get_variable_axiom_layer(var) << endl;
        os << get_variable_domain_size(var) << endl;
        for (int n = 0; n < get_variable_domain_size(var); n++)
        {
            os << get_fact_name(FactPair(var, n)) << endl;
        }
        os << "end_variable" << endl;
    }
}

void AbstractTask::dump_initial_state_to_SAS(std::ostream &os) const
{
    os << "begin_state" << endl;
    for (int val : get_initial_state_values())
        os << val << endl;
    os << "end_state" << endl;
}

void AbstractTask::dump_goal_to_SAS(std::ostream &os) const
{
    os << "begin_goal" << endl;
    os << get_num_goals() << endl;
    for (int i = 0; i < get_num_goals(); ++i)
    {
        FactPair fact = get_goal_fact(i);
        os << fact.var << " " << fact.value << endl;
    }
    os << "end_goal" << endl;
}

void AbstractTask::dump_to_SAS(ostream &os) const
{
    dump_version(os);
    dump_metric(os);
    dump_variables_to_SAS(os);

    dump_mutexes(os);

    dump_initial_state_to_SAS(os);
    dump_goal_to_SAS(os);

    dump_operators_to_SAS(os);
    dump_axioms_to_SAS(os);
}

void AbstractTask::dump() const
{
    dump_variables();
    dump_operators(false);
    if (get_num_axioms() > 0)
    {
        dump_operators(true);
    }
}

void AbstractTask::dump_variables() const
{
    std::cout << "Variables (|V|=" << get_num_variables() << "):" << std::endl;
    for (int var = 0; var < get_num_variables(); var++)
    {
        std::cout << " - " << get_variable_name(var);
        std::cout << " (|D|=" << get_variable_domain_size(var) << "): [";
        std::cout << get_fact_name(FactPair(var, 0));
        for (int val = 1; val < get_variable_domain_size(var); val++)
        {
            std::cout << ", " << get_fact_name(FactPair(var, val));
        }
        std::cout << "]" << std::endl;
    }
}

void AbstractTask::dump_operators(bool axioms) const
{
    std::cout << "Operators (|O|=" << get_num_operators() << "):" << std::endl;
    for (int op_id = 0; op_id < get_num_operators(); op_id++)
    {
        std::cout << " - " << get_operator_name(op_id, axioms) << std::flush;
        std::cout << " (cost=" << get_operator_cost(op_id, axioms)
                  << "):" << std::flush;
        std::cout << " <[" << std::flush;
        for (int pre = 0; pre < get_num_operator_preconditions(op_id, axioms);
             pre++)
        {
            std::cout << get_operator_precondition(op_id, pre, axioms);
            if (pre + 1 < get_num_operator_preconditions(op_id, axioms))
            {
                std::cout << ", " << std::flush;
            }
        }
        std::cout << "], " << std::flush;

        for (int eff = 0; eff < get_num_operator_effects(op_id, axioms); eff++)
        {
            if (get_num_operator_effect_conditions(op_id, eff, axioms) > 0)
            {
                std::cout << "[" << std::flush;
            }
            for (int cond = 0;
                 cond < get_num_operator_effect_conditions(op_id, eff, axioms);
                 cond++)
            {
                std::cout << get_operator_effect_condition(op_id, eff, cond, axioms);
                if (cond + 1 < get_num_operator_effect_conditions(op_id, eff, axioms))
                {
                    std::cout << ", " << std::flush;
                }
            }
            if (get_num_operator_effect_conditions(op_id, eff, axioms) > 0)
            {
                std::cout << "] =>" << std::flush;
            }

            std::cout << "[" << get_operator_effect(op_id, eff, axioms) << "]"
                      << std::flush;
            if (eff + 1 < get_num_operator_effects(op_id, axioms))
            {
                std::cout << ", " << std::flush;
            }
        }
        std::cout << ">" << std::endl;
    }
}

ostream &operator<<(ostream &os, const FactPair &fact_pair)
{
    os << fact_pair.var << "=" << fact_pair.value;
    return os;
}

static PluginTypePlugin<AbstractTask>
    _type_plugin("AbstractTask",
                 // TODO: Replace empty string by synopsis for the wiki page.
                 "");
