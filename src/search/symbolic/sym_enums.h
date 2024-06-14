#ifndef SYMBOLIC_SYM_ENUMS_H
#define SYMBOLIC_SYM_ENUMS_H

#include <iostream>
#include <string>
#include <vector>

namespace symbolic {
enum class MutexType {
    MUTEX_NOT,
    MUTEX_AND,
    MUTEX_EDELETION,
};
std::ostream &operator<<(std::ostream &os, const MutexType &m);

enum class ConditionalEffectsTransitionType {
    MONOLITHIC,
    CONJUNCTIVE,
    CONJUNCTIVE_EARLY_QUANTIFICATION
};
std::ostream &operator<<(std::ostream &os, const ConditionalEffectsTransitionType &ce_type);
extern bool is_ce_transition_type_conjunctive(const ConditionalEffectsTransitionType &ce_type);

enum class Dir {FW, BW, BIDIR};
std::ostream &operator<<(std::ostream &os, const Dir &dir);

// We use this enumerate to know why the current operation was truncated
enum class TruncatedReason {
    FILTER_MUTEX,
    MERGE_BUCKET,
    MERGE_BUCKET_COST,
    IMAGE_ZERO,
    IMAGE_COST
};
}
#endif
