#ifndef SYMBOLIC_OPEN_LIST_H
#define SYMBOLIC_OPEN_LIST_H

#include <map>
#include "sym_bucket.h"
#include <cassert>

namespace symbolic {
class SymStateSpaceManager;
class Frontier;

class OpenList {
    std::map<int, Bucket> open;//States in open with unkwown h-value

    //At any point in the search we can close all the states in
    // open[minG()] because they cannot be generated with lower
    // cost. Doing that we can set hNotClosed to the next bucket.
    void closeMinOpen();

    
public:
    bool empty() const {
	assert(open.empty() || !open.begin()->second.empty());
        return open.empty();
    }

    void insert(const Bucket &bucket, int g);
    void insert(const Bdd &bdd, int g);

    void extract_states(int f, int g, Bucket &res);
    void extract_states_directly(int g, Bucket &res);
    void extract_states(Bucket &bucket, int f, int g,
                        Bucket &res, bool open);
    int minG() const;

    int minNextG(const Frontier & frontier, int min_action_cost) const;
    void pop (Frontier & frontier);

    bool contains_any_state(const Bdd& bdd) const;

    friend std::ostream & operator<<(std::ostream &os, const OpenList & open);
};
}
#endif