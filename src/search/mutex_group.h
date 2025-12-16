#ifndef MUTEX_GROUP_H
#define MUTEX_GROUP_H

#include "abstract_task.h"

#include <iostream>
#include <vector>

class MutexGroup {
    std::vector<FactPair> facts;
    bool detected_fw;

public:
    MutexGroup(const std::vector<FactPair> &&facts, bool detected_fw);

    void dump() const;

    bool hasPair(int var, int val) const;

    inline const std::vector<FactPair> &getFacts() const {
        return facts;
    }

    inline bool detectedFW() const {
        return detected_fw;
    }

    // If the mutex was detected bw is used to prune fw search and vice versa
    inline bool pruneFW() const {
        return !detected_fw;
    }

    friend std::ostream &operator<<(std::ostream &os, const MutexGroup &mg);
};

#endif
