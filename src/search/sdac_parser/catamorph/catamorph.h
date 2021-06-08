#ifndef SDAC_PARSER_CATAMORPH_CATAMORPH_H
#define SDAC_PARSER_CATAMORPH_CATAMORPH_H

#include <string>
#include <vector>

#include "expression.h"
#include "factories.h"

using namespace boost::adaptors;

namespace sdac_parser {
class Catamorph {
public:
    template<typename Out, typename Algebra>
    static Out cata(Algebra f, Expression const &ast) {
        return f(
            fmap([f](Expression const &e) -> Out {return cata<Out>(f, e);},
                 ast.get()));
    }

    template<typename A, typename M>
    static auto fmap(M map, expression_r<A> const &e) {
        using B = decltype(map(std::declval<A>()));
        using Out = expression_r<B>;

        if (auto *o = Factories::get_as_add(e))
            // create an add operand with map applied on all operands
            return Out(add_op<B>(o->rands() | transformed(map)));

        if (auto *o = Factories::get_as_sub(e))
            return Out(sub_op<B>(o->rands() | transformed(map)));

        if (auto *o = Factories::get_as_mul(e))
            return Out(mul_op<B>(o->rands() | transformed(map)));

        if (auto *o = Factories::get_as_div(e))
            return Out(div_op<B>(o->rands() | transformed(map)));

        if (auto *o = Factories::get_as_greater(e))
            return Out(greater_op<B>(o->rands() | transformed(map)));

        if (auto *o = Factories::get_as_less(e))
            return Out(less_op<B>(o->rands() | transformed(map)));

        if (auto *o = Factories::get_as_greater_equals(e))
            return Out(greater_equals_op<B>(o->rands() | transformed(map)));

        if (auto *o = Factories::get_as_less_equals(e))
            return Out(less_equals_op<B>(o->rands() | transformed(map)));

        if (auto *o = Factories::get_as_and(e))
            return Out(and_op<B>(o->rands() | transformed(map)));

        if (auto *o = Factories::get_as_or(e))
            return Out(or_op<B>(o->rands() | transformed(map)));

        if (auto *o = Factories::get_as_not(e))
            return Out(not_op<B>(o->rands() | transformed(map)));

        if (auto *o = Factories::get_as_abs(e))
            return Out(abs_op<B>(o->rands() | transformed(map)));

        if (auto *o = Factories::get_as_equals(e))
            return Out(equals_op<B>(o->rands() | transformed(map)));

        if (auto *i = Factories::get_as_cst(e))
            return Out(*i);
        if (auto *v = Factories::get_as_var(e))
            return Out(*v);

        throw std::logic_error("Missing case in pattern matching in catamorph");
    }
};
}

#endif
