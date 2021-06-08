#include "factories.h"

namespace sdac_parser {
Expression Factories::mul(std::vector<Expression> const &rands) {
    return Expression(mul_op<Expression>{rands});
}

Expression Factories::sub(std::vector<Expression> const &rands) {
    return Expression(sub_op<Expression>{rands});
}

Expression Factories::add(std::vector<Expression> const &rands) {
    return Expression(add_op<Expression>{rands});
}

Expression Factories::var(ID id) {
    return Expression(id);
}

Expression Factories::cst(float i) {
    return Expression(i);
}

Expression Factories::div(std::vector<Expression> const &rands) {
    return Expression(div_op<Expression>{rands});
}

Expression Factories::greater(std::vector<Expression> const &rands) {
    return Expression(greater_op<Expression>{rands});
}

Expression Factories::less(std::vector<Expression> const &rands) {
    return Expression(less_op<Expression>{rands});
}

Expression Factories::greater_equals(std::vector<Expression> const &rands) {
    return Expression(greater_equals_op<Expression>{rands});
}

Expression Factories::less_equals(std::vector<Expression> const &rands) {
    return Expression(less_equals_op<Expression>{rands});
}
Expression Factories::land(std::vector<Expression> const &rands) {
    return Expression(and_op<Expression>{rands});
}

Expression Factories::equals(std::vector<Expression> const &rands) {
    return Expression(equals_op<Expression>{rands});
}

Expression Factories::lor(std::vector<Expression> const &rands) {
    return Expression(or_op<Expression>{rands});
}

Expression Factories::lnot(std::vector<Expression> const &rands) {
    return Expression(not_op<Expression>{rands});
}

Expression Factories::abs(std::vector<Expression> const &rands) {
    return Expression(abs_op<Expression>{rands});
}
}
