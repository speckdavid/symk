#include "sym_enums.h"

#include "../plugins/plugin.h"
#include "../utils/system.h"

using namespace std;

namespace symbolic {
static plugins::TypedEnumPlugin<MutexType> _mutex_type_enum_plugin(
    {{"mutex_not", "no mutexes"},
     {"mutex_and", "mutexes as BDD and used with not and"},
     {"mutex_edeletion",
      "mutexes added to transitions relations as edeletion"}});

static plugins::TypedEnumPlugin<ConditionalEffectsTransitionType> _conditional_effect_transition_type__enum_plugin(
    {{"monolithic",
      "one transition relation for each action combining all conditional effects"},
     {"var_based_conjunctive",
      "multiple conjunctive transition relations, one for each effect variable"},
     {"var_based_conjunctive_early_quantification",
      "same as var_based_conjunctive but with early quantification of variables"},
     {"eff_based_conjunctive",
      "multiple conjunctive transition relations, one for each conditional effect; adds auxiliary variables"},
     {"eff_based_conjunctive_early_quantification",
      "same as eff_based_conjunctive but with early quantification of variables"},
     {"dynamic",
      "tries to use variable-based for each transition relation, and if this does not succeed, falls back to effect-based, always with early quantification of variables"}});

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

ostream &operator<<(
    ostream &os, const ConditionalEffectsTransitionType &ce_type) {
    switch (ce_type) {
    case ConditionalEffectsTransitionType::MONOLITHIC:
        return os << "monolithic";
    case ConditionalEffectsTransitionType::VAR_BASED_CONJUNCTIVE:
        return os << "variable based conjunctive";
    case ConditionalEffectsTransitionType::
        VAR_BASED_CONJUNCTIVE_EARLY_QUANTIFICATION:
        return os << "variable based conjunctive with early quantification";
    case ConditionalEffectsTransitionType::EFF_BASED_CONJUNCTIVE:
        return os << "effect based conjunctive";
    case ConditionalEffectsTransitionType::
        EFF_BASED_CONJUNCTIVE_EARLY_QUANTIFICATION:
        return os << "effect based conjunctive with early quantification";
    case ConditionalEffectsTransitionType::DYNAMIC:
        return os
               << "variable + effect based conjunctive with early quantification";
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

bool is_ce_transition_type_early_quantification(
    const ConditionalEffectsTransitionType &ce_type) {
    return ce_type == ConditionalEffectsTransitionType::
                          VAR_BASED_CONJUNCTIVE_EARLY_QUANTIFICATION ||
           ce_type == ConditionalEffectsTransitionType::
                          EFF_BASED_CONJUNCTIVE_EARLY_QUANTIFICATION ||
           ce_type == ConditionalEffectsTransitionType::DYNAMIC;
}

bool is_ce_transition_type_variable_based(
    const ConditionalEffectsTransitionType &ce_type) {
    return ce_type == ConditionalEffectsTransitionType::VAR_BASED_CONJUNCTIVE ||
           ce_type == ConditionalEffectsTransitionType::
                          VAR_BASED_CONJUNCTIVE_EARLY_QUANTIFICATION ||
           ce_type == ConditionalEffectsTransitionType::DYNAMIC;
}

bool is_ce_transition_type_conjunctive(
    const ConditionalEffectsTransitionType &ce_type) {
    return ce_type == ConditionalEffectsTransitionType::VAR_BASED_CONJUNCTIVE ||
           ce_type == ConditionalEffectsTransitionType::
                          VAR_BASED_CONJUNCTIVE_EARLY_QUANTIFICATION ||
           ce_type == ConditionalEffectsTransitionType::EFF_BASED_CONJUNCTIVE ||
           ce_type == ConditionalEffectsTransitionType::
                          EFF_BASED_CONJUNCTIVE_EARLY_QUANTIFICATION ||
           ce_type == ConditionalEffectsTransitionType::DYNAMIC;
}
}
