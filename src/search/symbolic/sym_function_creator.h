#ifndef SYMBOLIC_SYM_FUNCTION_CREATOR_H
#define SYMBOLIC_SYM_FUNCTION_CREATOR_H

#include "sym_variables.h"
#include "../sdac_parser/parser.h"

namespace symbolic {
class SymbolicFunctionCreator {
protected:
    SymVariables *sym_vars; // To call basic BDD creation methods
    TaskProxy task_proxy;

    size_t get_var_id_by_name(const std::string &name) {
        for (size_t id = 0; id < task_proxy.get_variables().size(); ++id) {
            if (task_proxy.get_variables()[id].get_name() == name) {
                return id;
            }
        }
        std::cerr << "Variable " << name << " does not exist!" << std::endl;
        utils::exit_with(utils::ExitCode::SEARCH_INPUT_ERROR);
        return -1;
    }

    ADD get_variable_add(const std::string &name) {
        int var_id = get_var_id_by_name(name);
        ADD res = sym_vars->constant(0);
        for (int val = 0; val < task_proxy.get_variables()[var_id].get_domain_size(); ++val) {
            ADD cur = sym_vars->get_axiom_compiliation()->get_primary_representation(var_id, val).Add();
            res += sym_vars->constant(val) * cur;
        }
        return res;
    }

    auto create_add_alg() {
        return [this](sdac_parser::expression_r<ADD> const &e) -> ADD {
                   if (const NBR *cst = sdac_parser::Factories::get_as_cst(e)) {
                       return sym_vars->constant(*cst);
                   }
                   if (auto *var = sdac_parser::Factories::get_as_var(e)) {
                       return this->get_variable_add(*var);
                   }
                   if (auto *o = sdac_parser::Factories::get_as_add(e)) {
                       ADD res = o->rands().at(0);
                       for (size_t i = 1; i < o->rands().size(); ++i) {
                           res += o->rands().at(i);
                       }
                       return res;
                   }
                   if (auto *o = sdac_parser::Factories::get_as_sub(e)) {
                       ADD res = o->rands().at(0);
                       for (size_t i = 1; i < o->rands().size(); ++i) {
                           res -= o->rands().at(i);
                       }
                       return res;
                   }
                   if (auto *o = sdac_parser::Factories::get_as_mul(e)) {
                       ADD res = o->rands().at(0);
                       for (size_t i = 1; i < o->rands().size(); ++i) {
                           res *= o->rands().at(i);
                       }
                       return res;
                   }
                   if (auto *o = sdac_parser::Factories::get_as_div(e)) {
                       ADD res = o->rands().at(0);
                       for (size_t i = 1; i < o->rands().size(); ++i) {
                           res = res.Divide(o->rands().at(i));
                       }
                       return res;
                   }
                   if (auto *o = sdac_parser::Factories::get_as_abs(e)) {
                       ADD res = o->rands().at(0);
                       ADD n_res = o->rands().at(0) * sym_vars->constant(-1);
                       return res.Maximum(n_res);
                   }
                   if (auto *o = sdac_parser::Factories::get_as_greater_equals(e)) {
                       assert(o->rands().size() == 2);
                       return o->rands().at(0).GreaterThanEquals(o->rands().at(1));
                   }
                   if (auto *o = sdac_parser::Factories::get_as_greater(e)) {
                       assert(o->rands().size() == 2);
                       return o->rands().at(0).GreaterThan(o->rands().at(1));
                   }
                   if (auto *o = sdac_parser::Factories::get_as_less_equals(e)) {
                       assert(o->rands().size() == 2);
                       return o->rands().at(0).LessThanEquals(o->rands().at(1));
                   }
                   if (auto *o = sdac_parser::Factories::get_as_less(e)) {
                       assert(o->rands().size() == 2);
                       return o->rands().at(0).LessThan(o->rands().at(1));
                   }
                   if (auto *o = sdac_parser::Factories::get_as_equals(e)) {
                       assert(o->rands().size() == 2);
                       return o->rands().at(0).Equals(o->rands().at(1));
                   }
                   if (auto *o = sdac_parser::Factories::get_as_and(e)) {
                       assert(o->rands().size() == 2);
                       return o->rands().at(0) * o->rands().at(1);
                   }
                   if (auto *o = sdac_parser::Factories::get_as_or(e)) {
                       assert(o->rands().size() == 2);
                       return o->rands().at(0) + o->rands().at(1);
                   }
                   if (auto *o = sdac_parser::Factories::get_as_not(e)) {
                       assert(o->rands().size() == 1);
                       return sym_vars->constant(1) - o->rands().at(0);
                   }
                   throw std::logic_error("Unknown Operator in Create EVMDD");
               };
    }


    ADD create_add(sdac_parser::Expression const &expr) {
        return sdac_parser::Catamorph::cata<ADD>(
            [this](sdac_parser::expression_r<ADD> const &expr_r) -> ADD {
                return create_add_alg()(expr_r);
            },
            expr);
    }


public:
    ADD create_add(std::string const &term) {
        sdac_parser::InfixParser parser;
        sdac_parser::Expression expr = parser.parse(term);
        return create_add(expr);
    }

    SymbolicFunctionCreator(SymVariables *sym_vars,
                            const std::shared_ptr<AbstractTask> &task)
        : sym_vars(sym_vars), task_proxy(*task) {}
};
}

#endif
