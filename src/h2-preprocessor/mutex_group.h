#ifndef MUTEX_GROUP_H
#define MUTEX_GROUP_H

#include <iostream>
#include <vector>
#include "operator.h"
#include "state.h"
using namespace std;

class Variable;

enum Dir {FW, BW};
class MutexGroup {
    //Direction of the mutex.
    // Fw mutexes are not reachable from the initial state (should be pruned in bw search)
    // Bw mutexes cannot reach the goal (should be pruned in fw search)
    // Both mutex groups contain fw and bw mutexes so they should be pruned in both directions
    Dir dir;
    vector<pair<const Variable *, int>> facts;
public:
    MutexGroup(istream &in, const vector<Variable *> &variables);

    MutexGroup(const vector<pair<int, int>> &f,
               const vector<Variable *> &variables,
               bool regression);

    MutexGroup(const Variable *var);
    void strip_unimportant_facts();
    bool is_redundant() const;

    bool is_fw() const {
        return dir == FW;
    }
    int get_encoding_size() const;
    int num_facts() const {
        return facts.size();
    }
    void generate_cpp_input(ofstream &outfile) const;
    void dump() const;
    void get_mutex_group(vector<pair<int, int>> &invariant_group) const;

    void remove_unreachable_facts();

    bool hasPair(int var, int val) const;

    inline const vector<pair<const Variable *, int>> &getFacts() const {
        return facts;
    }

    void add_tuples(std::set<std::vector<int>> &tuples) const {
        for (size_t i = 0; i < facts.size(); ++i) {
            for (size_t j = i + 1; j < facts.size(); ++j) {
                int v1 = facts[i].first->get_level();
                int v2 = facts[j].first->get_level();
                int a1 = facts[i].second;
                int a2 = facts[j].second;
                if (v1 == v2)
                    continue;
                if (v2 < v1) {
                    int tmp = v1;
                    v1 = v2;
                    v2 = tmp;
                    tmp = a1;
                    a1 = a2;
                    a2 = tmp;
                }
                vector<int> tup;
                tup.push_back(v1);
                tup.push_back(a1);
                tup.push_back(v2);
                tup.push_back(a2);
                tuples.insert(tup);
            }
        }
    }
};

extern void strip_mutexes(vector<MutexGroup> &mutexes);

#endif
