#include "sym_enums.h"
#include "../utils/system.h"

using namespace std;

namespace symbolic {
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

const vector<string> MutexTypeValues{
    "MUTEX_NOT", "MUTEX_AND", "MUTEX_EDELETION",
};

const vector<string> DirValues{"FW", "BW", "BIDIR"};
}
