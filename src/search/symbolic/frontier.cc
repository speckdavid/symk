#include "frontier.h"

#include "sym_state_space_manager.h"

#include "../utils/timer.h"

using namespace std;

namespace symbolic {
Frontier::Frontier() : mgr(nullptr), g_value(0) {}

void Frontier::init(SymStateSpaceManager *mgr_, const BDD &bdd) {
    mgr = mgr_;
    Sfilter.push_back(bdd);
    g_value = 0;
}

void Frontier::set(int g, Bucket &bdd) {
    assert(empty());
    g_value = g;
    Sfilter.swap(bdd);
}

bool Frontier::nextStepZero() const {
    return !Szero.empty() || (S.empty() && mgr->has_zero_cost_transition());
}

Result Frontier::prepare(int maxTime, int maxNodes, bool fw,
                         bool initialization) {
    utils::Timer filterTime;
    if (!Sfilter.empty()) {
        int numFiltered =
            mgr->filterMutexBucket(Sfilter, fw, initialization, maxTime, maxNodes);
        if (numFiltered > 0) {
            Smerge.insert(Smerge.end(), Sfilter.begin(), Sfilter.begin() + numFiltered);
        }
        if (numFiltered == (int)(Sfilter.size())) {
            Bucket().swap(Sfilter);
        } else {
            Sfilter.erase(Sfilter.begin(), Sfilter.begin() + numFiltered);
            return Result(TruncatedReason::FILTER_MUTEX, filterTime());
        }
    }

    if (!Smerge.empty()) {
        if (Smerge.size() > 1) {
            mgr->merge_bucket(Smerge, 60000, 10000000);
        }

        if (mgr->has_zero_cost_transition()) {
            S.insert(S.end(), Smerge.begin(), Smerge.end());
            assert(Szero.empty());
            Szero.swap(Smerge);
        } else {
            S.swap(Smerge);
        }
    }

    // If there are zero cost operators, merge S
    if (mgr->has_zero_cost_transition() && Szero.empty()) {
        if (S.size() > 1) {
            mgr->merge_bucket(S, 60000, 10000000);
        }
    }

    return Result(filterTime());
}

bool Frontier::bucketReady() const {
    return !(Szero.empty() && S.empty() && Sfilter.empty() && Smerge.empty());
}

bool Frontier::expansionReady() const {
    return Sfilter.empty() && Smerge.empty() && !(Szero.empty() && S.empty());
}

bool Frontier::empty() const {
    return Szero.empty() && S.empty() && Sfilter.empty() && Smerge.empty();
}

int Frontier::nodes() const {
    if (!Szero.empty()) {
        return nodeCount(Szero);
    } else if (!S.empty()) {
        return nodeCount(S);
    } else {
        return nodeCount(Sfilter) + nodeCount(Smerge);
    }
}

int Frontier::buckets() const {
    if (!Szero.empty()) {
        return Szero.size();
    } else if (!S.empty()) {
        return S.size();
    } else {
        return Sfilter.size() + Smerge.size();
    }
}

ResultExpansion Frontier::expand_zero(int maxTime, int maxNodes, bool fw) {
    // Image with respect to 0-cost actions
    utils::Timer image_time;

    mgr->set_time_limit(maxTime);
    // Compute image, storing the result on Simg
    try {
        for (size_t i = 0; i < Szero.size(); i++) {
            Simg.push_back(map<int, Bucket>());
            mgr->zero_image(fw, Szero[i], Simg[i][0], maxNodes);
        }
        mgr->unset_time_limit();
    } catch (const BDDError &e) {
        mgr->unset_time_limit();
        return ResultExpansion(true, TruncatedReason::IMAGE_ZERO, image_time());
    }

    Bucket().swap(Szero); // Delete Szero because it has been expanded

    return ResultExpansion(true, Simg, image_time());
}

ResultExpansion Frontier::expand_cost(int maxTime, int maxNodes, bool fw) {
    utils::Timer image_time;
    mgr->set_time_limit(maxTime);
    // cout << maxTime << " + " << maxNodes << endl;
    try {
        for (size_t i = 0; i < S.size(); i++) {
            Simg.push_back(map<int, Bucket>());
            mgr->cost_image(fw, S[i], Simg[i], maxNodes);
        }
        mgr->unset_time_limit();
    } catch (const BDDError &e) {
        // Update estimation
        mgr->unset_time_limit();

        return ResultExpansion(false, TruncatedReason::IMAGE_COST, image_time());
    }

    Bucket().swap(S); // Delete Szero because it has been expanded
    return ResultExpansion(false, Simg, image_time());
}

ostream &operator<<(ostream &os, const Frontier &frontier) {
    if (!frontier.Sfilter.empty())
        os << "Sf: " << nodeCount(frontier.Sfilter) << " ";
    if (!frontier.Smerge.empty())
        os << "Sm: " << nodeCount(frontier.Smerge) << " ";
    if (!frontier.Szero.empty())
        os << "Sz: " << nodeCount(frontier.Szero) << " ";
    if (!frontier.S.empty())
        os << "S: " << nodeCount(frontier.S) << " ";
    return os;
}
}
