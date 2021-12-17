#ifndef SDAC_PARSER_CATAMORPH_FACTORIES_H
#define SDAC_PARSER_CATAMORPH_FACTORIES_H

#include "expression.h"

namespace sdac_parser {
class Factories {
public:
    static Expression cst(float i);
    static Expression var(ID id);
    static Expression add(std::vector<Expression> const &rands);
    static Expression sub(std::vector<Expression> const &rands);
    static Expression mul(std::vector<Expression> const &rands);
    static Expression div(std::vector<Expression> const &rands);
    static Expression greater(std::vector<Expression> const &rands);
    static Expression less(std::vector<Expression> const &rands);
    static Expression greater_equals(std::vector<Expression> const &rands);
    static Expression less_equals(std::vector<Expression> const &rands);
    static Expression land(std::vector<Expression> const &rands);
    static Expression equals(std::vector<Expression> const &rands);
    static Expression lor(std::vector<Expression> const &rands);
    // TODO can we implement not and abs as unary operators?
    static Expression lnot(std::vector<Expression> const &rands);
    static Expression abs(std::vector<Expression> const &rands);

    template<typename T>
    static float const *get_as_cst(expression_r<T> const &e) {
        return boost::get<float>(&e);
    }

    template<typename T>
    static ID const *get_as_var(expression_r<T> const &e) {
        return (ID *)boost::get<ID>(&e);
    }

    template<typename T>
    static add_op<T> const *get_as_add(expression_r<T> const &e) {
        return boost::get<add_op<T>>(&e);
    }

    template<typename T>
    static sub_op<T> const *get_as_sub(expression_r<T> const &e) {
        return boost::get<sub_op<T>>(&e);
    }

    template<typename T>
    static mul_op<T> const *get_as_mul(expression_r<T> const &e) {
        return boost::get<mul_op<T>>(&e);
    }

    template<typename T>
    static div_op<T> const *get_as_div(expression_r<T> const &e) {
        return boost::get<div_op<T>>(&e);
    }

    template<typename T>
    static greater_op<T> const *get_as_greater(expression_r<T> const &e) {
        return boost::get<greater_op<T>>(&e);
    }

    template<typename T>
    static less_op<T> const *get_as_less(expression_r<T> const &e) {
        return boost::get<less_op<T>>(&e);
    }

    template<typename T>
    static greater_equals_op<T> const *get_as_greater_equals(
        expression_r<T> const &e) {
        return boost::get<greater_equals_op<T>>(&e);
    }

    template<typename T>
    static less_equals_op<T> const *get_as_less_equals(
        expression_r<T> const &e) {
        return boost::get<less_equals_op<T>>(&e);
    }

    template<typename T>
    static and_op<T> const *get_as_and(expression_r<T> const &e) {
        return boost::get<and_op<T>>(&e);
    }

    template<typename T>
    static equals_op<T> const *get_as_equals(expression_r<T> const &e) {
        return boost::get<equals_op<T>>(&e);
    }

    template<typename T>
    static or_op<T> const *get_as_or(expression_r<T> const &e) {
        return boost::get<or_op<T>>(&e);
    }

    template<typename T>
    static not_op<T> const *get_as_not(expression_r<T> const &e) {
        return boost::get<not_op<T>>(&e);
    }

    template<typename T>
    static abs_op<T> const *get_as_abs(expression_r<T> const &e) {
        return boost::get<abs_op<T>>(&e);
    }
};
}

#endif
