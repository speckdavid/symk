#ifndef SYMBOLIC_FRONTIER_H
#define SYMBOLIC_FRONTIER_H

#include "searches/sym_search.h"
#include "sym_bucket.h"

#include <cassert>
#include <map>

namespace symbolic {
class SymStateSpaceManager;

class Result {
public:
    bool ok;
    TruncatedReason truncated_reason;
    double time_spent;

    Result(double t) : ok(true), time_spent(t) {}
    Result(TruncatedReason reason, double t)
        : ok(false), truncated_reason(reason), time_spent(t) {}
};

class ResultExpansion : public Result {
public:
    bool step_zero;
    std::vector<std::map<int, Bucket>> buckets;
    ResultExpansion(bool step_zero_, std::vector<std::map<int, Bucket>> &buckets_,
                    double t)
        : Result(t), step_zero(step_zero_) {
        buckets.swap(buckets_);
    }

    ResultExpansion(bool step_zero_, TruncatedReason reason, double t)
        : Result(reason, t), step_zero(step_zero_) {}
};

class Frontier { // Current states extracted from the open list
    SymStateSpaceManager *mgr;

    Bucket Sfilter; // current g-bucket without duplicates and h-classified (still
                    // not filtered mutexes)
    Bucket Smerge; // bucket before applying merge
    Bucket Szero; // bucket to expand 0-cost transitions
    Bucket S;     // bucket to expand cost transitions

    // bucket to store temporary image results in expand_zero() and expand_cost()
    // For each BDD in Szero or S, stores a map with pairs <cost, resImage>
    std::vector<std::map<int, Bucket>> Simg;

    int g_value;

    ResultExpansion expand_zero(int maxTime, int maxNodes, bool fw);
    ResultExpansion expand_cost(int maxTime, int maxNodes, bool fw);

public:
    Frontier();

    void init(SymStateSpaceManager *mgr, const BDD &bdd);
    void set(int g, Bucket &open);

    Result prepare(int maxTime, int maxNodes, bool fw, bool initialization);

    bool empty() const;
    bool bucketReady() const;
    bool expansionReady() const;
    bool nextStepZero() const;

    int nodes() const;
    int buckets() const;

    int g() const {return g_value;}

    Bucket &prepared_bucket() {
        assert(Sfilter.empty());
        assert(Smerge.empty());
        // assert(Szero.empty() || S.empty());
        // assert(!Szero.empty() || !S.empty());

        return Szero.empty() ? S : Szero;
    }
    Bucket &bucket() {
        assert(Smerge.empty());
        assert(Szero.empty());
        assert(S.empty());
        return Sfilter;
    }

    void filter(const BDD &bdd) {
        assert(Smerge.empty() && Szero.empty() && S.empty());
        for (BDD &b : Sfilter) {
            b *= !bdd;
        }
    }

    ResultExpansion expand(int maxTime, int maxNodes, bool fw) {
        assert(Smerge.empty() && Sfilter.empty());
        if (!Szero.empty()) {
            return expand_zero(maxTime, maxNodes, fw);
        }

        assert(!S.empty());
        // Image with respect to cost actions
        return expand_cost(maxTime, maxNodes, fw);
    }

    friend std::ostream &operator<<(std::ostream &os, const Frontier &frontier);
};
}
#endif
