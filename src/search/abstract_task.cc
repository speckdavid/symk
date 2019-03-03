#include "abstract_task.h"

#include "per_task_information.h"
#include "plugin.h"

#include <iostream>

using namespace std;

const FactPair FactPair::no_fact = FactPair(-1, -1);

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
        std::cout << " (cost=" << get_operator_cost(op_id, axioms) << "):" << std::flush;
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

            std::cout << "[" << get_operator_effect(op_id, eff, axioms) << "]" << std::flush;
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

static PluginTypePlugin<AbstractTask> _type_plugin(
    "AbstractTask",
    // TODO: Replace empty string by synopsis for the wiki page.
    "");
