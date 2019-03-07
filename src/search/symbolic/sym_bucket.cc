#include "sym_bucket.h"

#include <algorithm>
#include <cassert>

using namespace std;

namespace symbolic {
void removeZero(Bucket &bucket) {
    bucket.erase(remove_if(std::begin(bucket), std::end(bucket),
                           [] (Bdd &bdd) {return bdd.IsZero(); }),
                 std::end(bucket));
}

void copyBucket(const Bucket &bucket, Bucket &res) {
    if (!bucket.empty()) {
        res.insert(std::end(res), std::begin(bucket), std::end(bucket));
    }
}

void moveBucket(Bucket &bucket, Bucket &res) {
    copyBucket(bucket, res);
    Bucket().swap(bucket);
}

int nodeCount(const Bucket &bucket) {
    int sum = 0;
    for (const Bdd &bdd : bucket) {
        sum += bdd.nodeCount();
    }
    return sum;
}


bool extract_states(Bucket &list,
                    const Bucket &pruned,
                    Bucket &res) {
    assert(!pruned.empty());

    bool somethingPruned = false;
    for (auto &bddList : list) {
        Bdd prun = pruned[0] * bddList;

        for (const auto &prbdd :  pruned) {
            prun += prbdd * bddList;
        }

        if (!prun.IsZero()) {
            somethingPruned = true;
            bddList -= prun;
            res.push_back(prun);
        }
    }
    removeZero(list);
    return somethingPruned;
}

bool bucket_contains_any_state(const Bucket &bucket, const Bdd &bdd) {
    for (const Bdd &bucket_bdd : bucket) {
        Bdd intersection = bucket_bdd * bdd;
        if (!intersection.IsZero()) {
            return true;
        }
    }
    return false;
}

}