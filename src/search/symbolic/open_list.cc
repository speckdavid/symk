#include "open_list.h"

#include "frontier.h"

#include <cassert>

using namespace std;

namespace symbolic {
void OpenList::insert(const Bucket &bucket, int g) {
    assert(!bucket.empty());
    copyBucket(bucket, open[g]);
}

void OpenList::insert(const BDD &bdd, int g) {
    assert(!bdd.IsZero());
    open[g].push_back(bdd);
}

int OpenList::minNextG(const Frontier &frontier, int min_action_cost) const {
    int next_g = (frontier.empty() ? numeric_limits<int>::max()
                  : frontier.g() + min_action_cost);
    if (!open.empty()) {
        return min(next_g, open.begin()->first);
    }
    return next_g;
}

void OpenList::pop(Frontier &frontier) {
    assert(frontier.empty());
    int g = open.begin()->first;
    frontier.set(g, open.begin()->second);
    open.erase(g);
}

int OpenList::minG() const {
    return open.empty() ? numeric_limits<int>::max() : open.begin()->first;
}

bool OpenList::contains_any_state(const BDD &bdd) const {
    for (auto &key : open) {
        if (bucket_contains_any_state(key.second, bdd)) {
            return true;
        }
    }
    return false;
}

ostream &operator<<(ostream &os, const OpenList &exp) {
    os << " open{";
    for (auto &o : exp.open) {
        os << o.first << " ";
    }
    return os << "}";
}
}
