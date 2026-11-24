#include "int_inf.h"

using namespace std;

namespace utils {
const IntInf IntInf::INF = numeric_limits<size_t>::max();

IntInf &IntInf::operator+=(const IntInf &other) {
    if (is_infinity() || other.is_infinity()) {
        value = INF_VAL;
    } else {
        // Check for addition overflow: if (value + other.value) > INF_VAL
        if (INF_VAL - value < other.value) {
            value = INF_VAL;
        } else {
            value += other.value;
        }
    }
    return *this;
}

IntInf IntInf::operator+(const IntInf &other) const {
    IntInf result = *this;
    result += other;
    return result;
}

ostream &operator<<(ostream &os, const IntInf &ss) {
    if (ss.is_infinity()) {
        os << numeric_limits<double>::infinity();
    } else {
        os << ss.value;
    }
    return os;
}
}