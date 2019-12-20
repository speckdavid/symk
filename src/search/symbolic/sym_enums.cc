#include "sym_enums.h"
#include "../utils/system.h"

namespace symbolic {

std::ostream &operator<<(std::ostream &os, const Dir &dir) {
  switch (dir) {
  case Dir::FW:
    return os << "fw";
  case Dir::BW:
    return os << "bw";
  case Dir::BIDIR:
    return os << "bd";
  default:
    std::cerr << "Name of Dir not known";
    utils::exit_with(utils::ExitCode::SEARCH_UNSUPPORTED);
  }
}

std::ostream &operator<<(std::ostream &os, const MutexType &m) {
  switch (m) {
  case MutexType::MUTEX_NOT:
    return os << "not";
  case MutexType::MUTEX_EDELETION:
    return os << "edeletion";
  case MutexType::MUTEX_AND:
    return os << "and";
    /*case MutexType::MUTEX_RESTRICT:
        return os << "restrict";
    case MutexType::MUTEX_NPAND:
        return os << "npand";
    case MutexType::MUTEX_CONSTRAIN:
        return os << "constrain";
    case MutexType::MUTEX_LICOMP:
        return os << "licompaction";*/
  default:
    std::cerr << "Name of MutexType not known";
    utils::exit_with(utils::ExitCode::SEARCH_UNSUPPORTED);
  }
}

const std::vector<std::string> MutexTypeValues{
    "MUTEX_NOT", "MUTEX_AND", "MUTEX_EDELETION",
    /*"MUTEX_RESTRICT", "MUTEX_NPAND", "MUTEX_CONSTRAIN", "MUTEX_LICOMP"*/};

const std::vector<std::string> DirValues{"FW", "BW", "BIDIR"};
} // namespace symbolic
