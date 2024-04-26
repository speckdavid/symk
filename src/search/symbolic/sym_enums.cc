#include "sym_enums.h"
#include "../utils/system.h"

using namespace std;

namespace symbolic {
const vector<string> &MutexTypeValues{
    "MUTEX_NOT", "MUTEX_AND", "MUTEX_EDELETION",
};

const vector<string> &ConditionalEffectsTransitionTypeValues{
    "MONOLITHIC", "CONJUNCTIVE", "CONJUNCTIVE_EARLY_QUANTIFICATION"
};

const vector<string> &DirValues{"FW", "BW", "BIDIR"};

ostream &operator<<(ostream &os, const MutexType &m) {
    switch (m) {
    case MutexType::MUTEX_NOT:
        return os << "not";
    case MutexType::MUTEX_EDELETION:
        return os << "edeletion";
    case MutexType::MUTEX_AND:
        return os << "and";
    default:
        cerr << "Name of MutexType not known";
        utils::exit_with(utils::ExitCode::SEARCH_UNSUPPORTED);
    }
}

ostream &operator<<(ostream &os, const ConditionalEffectsTransitionType &ce_type) {
    switch (ce_type) {
    case ConditionalEffectsTransitionType::MONOLITHIC:
        return os << "monolithic";
    case ConditionalEffectsTransitionType::CONJUNCTIVE:
        return os << "conjunctive";
    case ConditionalEffectsTransitionType::CONJUNCTIVE_EARLY_QUANTIFICATION:
        return os << "conjunctive with early quantification";
    default:
        cerr << "Name of ConditionalEffectsTransitionType not known";
        utils::exit_with(utils::ExitCode::SEARCH_UNSUPPORTED);
    }
}

ostream &operator<<(ostream &os, const Dir &dir) {
    switch (dir) {
    case Dir::FW:
        return os << "fw";
    case Dir::BW:
        return os << "bw";
    case Dir::BIDIR:
        return os << "bd";
    default:
        cerr << "Name of Dir not known";
        utils::exit_with(utils::ExitCode::SEARCH_UNSUPPORTED);
    }
}

bool is_ce_transition_type_conjunctive(const ConditionalEffectsTransitionType &ce_type) {
    return ce_type == ConditionalEffectsTransitionType::CONJUNCTIVE || ce_type == ConditionalEffectsTransitionType::CONJUNCTIVE_EARLY_QUANTIFICATION;
}
}
