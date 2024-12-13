#ifndef STATE_H
#define STATE_H

#include <iostream>
#include <map>
#include <vector>
using namespace std;

class Variable;

class State {
    map<Variable *, int> values;
public:
    State() {} // TODO: Entfernen (erfordert kleines Redesign)
    State(istream &in, const vector<Variable *> &variables);

    int operator[](Variable *var) const;
    void dump() const;
    // Returns true, if the state contains an unreachable fact
    bool remove_unreachable_facts();

    bool is_goal_state(const vector<pair<Variable *, int>> &goals) const;
};

#endif
