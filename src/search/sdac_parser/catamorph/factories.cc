#include "factories.h"

using namespace std;

namespace sdac_parser {
Expression Factories::mul(vector<Expression> const &rands) {
    return Expression(mul_op<Expression>{rands});
}

Expression Factories::sub(vector<Expression> const &rands) {
    return Expression(sub_op<Expression>{rands});
}

Expression Factories::add(vector<Expression> const &rands) {
    return Expression(add_op<Expression>{rands});
}

Expression Factories::var(ID id) {
    return Expression(id);
}

Expression Factories::cst(float i) {
    return Expression(i);
}

Expression Factories::div(vector<Expression> const &rands) {
    return Expression(div_op<Expression>{rands});
}

Expression Factories::greater(vector<Expression> const &rands) {
    return Expression(greater_op<Expression>{rands});
}

Expression Factories::less(vector<Expression> const &rands) {
    return Expression(less_op<Expression>{rands});
}

Expression Factories::greater_equals(vector<Expression> const &rands) {
    return Expression(greater_equals_op<Expression>{rands});
}

Expression Factories::less_equals(vector<Expression> const &rands) {
    return Expression(less_equals_op<Expression>{rands});
}
Expression Factories::land(vector<Expression> const &rands) {
    return Expression(and_op<Expression>{rands});
}

Expression Factories::equals(vector<Expression> const &rands) {
    return Expression(equals_op<Expression>{rands});
}

Expression Factories::lor(vector<Expression> const &rands) {
    return Expression(or_op<Expression>{rands});
}

Expression Factories::lnot(vector<Expression> const &rands) {
    return Expression(not_op<Expression>{rands});
}

Expression Factories::abs(vector<Expression> const &rands) {
    return Expression(abs_op<Expression>{rands});
}
}
