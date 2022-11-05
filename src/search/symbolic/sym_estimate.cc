#include "sym_estimate.h"

#include <algorithm>
#include <iostream>

#include "sym_params_search.h"
#include "sym_utils.h"
#include "../utils/logging.h"

using namespace std;

namespace symbolic {
Estimation::Estimation(double time, double nodes, bool failed) :
    time(time),
    nodes(nodes),
    failed(failed) {}

void Estimation::set_data(double time, double nodes, bool failed) {
    this->time = time;
    this->nodes = nodes;
    this->failed = failed;
}

bool Estimation::operator<(const Estimation &other) const {
    if (!get_failed() && other.get_failed())
        return true;
    if (get_failed() && !other.get_failed())
        return false;
    if (this->time < other.time)
        return true;
    if (this->time > other.time)
        return false;
    return this->nodes < other.nodes;
}

ostream &operator<<(ostream &os, const Estimation &est) {
    return os << "{Time=" << est.time << ",Nodes=" << est.nodes << ",Failed=" << est.failed << "}";
}
}
