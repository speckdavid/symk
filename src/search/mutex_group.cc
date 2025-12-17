#include "mutex_group.h"

#include "abstract_task.h"

#include "tasks/root_task.h"

using namespace std;

MutexGroup::MutexGroup(const vector<FactPair> &&facts, bool detected_fw)
    : facts(move(facts)), detected_fw(detected_fw){};

bool MutexGroup::hasPair(int var, int val) const {
    for (size_t i = 0; i < facts.size(); ++i) {
        if (facts[i].var == var && facts[i].value == val) {
            return true;
        }
    }
    return false;
}

ostream &operator<<(ostream &os, const MutexGroup &mg) {
    os << (mg.detected_fw ? "fw" : "bw");
    for (size_t i = 0; i < mg.facts.size(); ++i) {
        os << "   " << tasks::g_root_task->get_fact_name(mg.facts[i]);
    }
    return os << "]";
}
