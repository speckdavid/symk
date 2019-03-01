#include "variable.h"

#include "helper_functions.h"

#include <cassert>
#include <fstream>
#include <iostream>

using namespace std;

Variable::Variable(istream &in) {
    int range;
    check_magic(in, "begin_variable");
    in >> ws >> name >> layer >> range >> ws;
    values.resize(range);
    for (int i = 0; i < range; ++i)
        getline(in, values[i]);
    check_magic(in, "end_variable");
    level = -1;
    necessary = false;
    reachable_values = range;
    reachable = vector<bool> (range, true);
}

void Variable::set_level(int theLevel) {
    level = theLevel;
}

int Variable::get_level() const {
    return level;
}

void Variable::set_necessary() {
    assert(necessary == false);
    necessary = true;
}

int Variable::get_range() const {
    return values.size();
}

string Variable::get_name() const {
    return name;
}

bool Variable::is_necessary() const {
    return necessary && reachable_values > 1;
}

void Variable::dump() const {
    // TODO: Dump values (and other information that might be missing?)
    //       or get rid of this if it's no longer needed.
    cout << name << " [range " << get_range();
    if (level != -1)
        cout << "; level " << level << " values: ";
    if (is_derived())
        cout << "; derived; layer: " << layer << " values: ";
    for (size_t i = 0; i < values.size(); ++i)
        cout << " " << values[i];
    cout << "]" << endl;
}

void Variable::generate_cpp_input(ofstream &outfile) const {
    outfile << "begin_variable" << endl
            << name << endl
            << layer << endl
            << reachable_values << endl;
    for (size_t i = 0; i < values.size(); ++i)
        if (reachable[i])
            outfile << values[i] << endl;
    outfile << "end_variable" << endl;
}

void Variable::remove_unreachable_facts() {
    vector<string> new_values;
    for (size_t i = 0; i < values.size(); i++) {
        if (reachable[i]) {
            new_values.push_back(values[i]);
        }
    }
    new_values.swap(values);
    vector<bool> (values.size(), true).swap(reachable);
}

int Variable::get_new_id(int value) const {
    assert(reachable[value]);
    if (!reachable[value]) {
        cerr << "ERROR: Tried to update an unreachable fact" << endl;
        exit(-1);
    }
    int id = 0;
    for (int i = 0; i < value; ++i) {
        if (reachable[i]) {
            id++;
        }
    }
    return id;
}
