#include "mutex_group.h"

#include "tasks/root_task.h"
#include "abstract_task.h" //For FactPair

using namespace std;

MutexGroup::MutexGroup(istream &in) {
    string exactly_one_str, dir;
    int num_facts;
    in >> exactly_one_str;
    if (!exactly_one_str.empty() && exactly_one_str.find_first_not_of("0123456789") == string::npos) {
        num_facts = stoi(exactly_one_str);
        exactly_one_str = "mutex";
        dir = "fw";
    } else {
        in >> dir;
        in >> num_facts;
    }

    facts.reserve(num_facts);
    for (int j = 0; j < num_facts; ++j) {
        int var, val;
        in >> var >> val;
        facts.push_back(FactPair(var, val));
    }
    exactly_one = (exactly_one_str == "exactly_one");
    detected_fw = (dir == "fw");
}

bool MutexGroup::hasPair(int var, int val) const {
    for (size_t i = 0; i < facts.size(); ++i) {
        if (facts[i].var == var && facts[i].value == val) {
            return true;
        }
    }
    return false;
}

ostream &operator<<(ostream &os, const MutexGroup &mg) {
    os << (mg.exactly_one ? "[ExactlyOne_" : "[MutexGroup_") <<
    (mg.detected_fw ? "fw" : "bw");
    for (size_t i = 0; i < mg.facts.size(); ++i) {
        os << "   " << tasks::g_root_task->get_fact_name(mg.facts[i]);
    }
    return os << "]";
}
