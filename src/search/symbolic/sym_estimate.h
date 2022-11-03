#ifndef SYMBOLIC_SYM_ESTIMATE_H
#define SYMBOLIC_SYM_ESTIMATE_H

#include <fstream>
#include <iostream>
#include <map>
#include <utility>

namespace symbolic {
class SymParamsSearch;

class Estimation {
protected:
    double time;
    double nodes;
    bool failed;

public:
    Estimation(double time, double nodes, bool failed);

    void set_data(double time, double nodes, bool failed);

    double get_time() const {return time;}
    double get_nodes() const {return nodes;}
    bool get_failed() const {return failed;}

    bool operator<(const Estimation &other) const;

    friend std::ostream &operator<<(std::ostream &os, const Estimation &est);
};
}
#endif
