#include "sym_enums.h"
#include "../utils/system.h"
#include "../plugins/plugin.h"

using namespace std;

namespace symbolic {
static plugins::TypedEnumPlugin<MutexType> _mutex_type_enum_plugin({
        {"mutex_not", "no mutexes"},
        {"mutex_and", "mutexes as BDD and used with not and"},
        {"mutex_edeletion", "mutexes added to transitions relations as edeletion"}
    });

static plugins::TypedEnumPlugin<ConditionalEffectsTransitionType> _conditional_effect_transition_type__enum_plugin({
        {"monolithic", "one transition relation for each action combining all conditional effects"},
        {"conjunctive", "multiple conjunctive transition relations, one for each conditional effect"},
        {"conjunctive_early_quantification", "same as conjunctive but with early quantification of variables"}
    });

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
