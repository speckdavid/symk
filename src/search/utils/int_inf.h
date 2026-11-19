#ifndef UTILS_INT_INF_H
#define UTILS_INT_INF_H

#include <iostream>
#include <limits>

namespace utils {
class IntInf {
    size_t value;
    static constexpr size_t INF_VAL = std::numeric_limits<size_t>::max();

public:
    static const IntInf INF;

    IntInf() : value(0) {
    }
    IntInf(size_t v) : value(v) {
    }

    // Inspection
    inline bool is_infinity() const {
        return value == INF_VAL;
    }
    inline size_t get_raw_value() const {
        return value;
    }

    inline double get_d() const {
        if (is_infinity()) {
            return std::numeric_limits<double>::infinity();
        }
        return static_cast<double>(value);
    }

    // Operators
    IntInf &operator+=(const IntInf &other);
    IntInf operator+(const IntInf &other) const;

    bool operator<(const IntInf &other) const {
        return value < other.value;
    }
    bool operator>(const IntInf &other) const {
        return value > other.value;
    }
    bool operator==(const IntInf &other) const {
        return value == other.value;
    }

    friend std::ostream &operator<<(std::ostream &os, const IntInf &ss);
};
}

#endif