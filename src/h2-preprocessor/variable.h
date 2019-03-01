#ifndef VARIABLE_H
#define VARIABLE_H

#include <iostream>
#include <vector>
using namespace std;

class Variable {
    vector<string> values;
    string name;
    int layer;
    int level;
    bool necessary;
    vector<bool> reachable; //atorralba: added to prune unreachable values
    int reachable_values;
public:
    Variable(istream &in);
    void set_level(int level);
    void set_necessary();
    void reset_necessary(){necessary= false;}
    int get_level() const;
    bool is_necessary() const;
    int get_range() const;
    string get_name() const;
    int get_layer() const {return layer; }
    bool is_derived() const {return layer != -1; }
    void generate_cpp_input(ofstream &outfile) const;
    void dump() const;

    string get_fact_name(int value) const {
        return values[value];
    }

    inline void set_unreachable(int value) {
        if (reachable[value]) {
            reachable[value] = false;
            reachable_values--;
        }
    }

    inline bool is_reachable(int value) const {
        return reachable[value];
    }

    void remove_unreachable_facts();
    int get_new_id(int value) const;
};

#endif
