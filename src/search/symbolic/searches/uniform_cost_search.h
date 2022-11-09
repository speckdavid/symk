#ifndef SYMBOLIC_UNIFORM_COST_SEARCH_H
#define SYMBOLIC_UNIFORM_COST_SEARCH_H

#include "sym_search.h"
#include "../closed_list.h"
#include "../frontier.h"
#include "../open_list.h"
#include "../sym_bucket.h"
#include "../sym_estimate.h"
#include "../sym_state_space_manager.h"
#include "../sym_utils.h"

#include <memory>

namespace symbolic {
/*
 * This class allows to perform a BDD search.  It is designed to
 * mantain the current state in the search.  We consider four
 * different points at which we may truncate the search:
 * pop(), filter_mutex(), expand_zero(), expand_cost()
 * We mantain 3 BDDs to know the current state: Stmp, S and Szero.
 * Briefly:
 * 1) if Sfilter, Szero and S are empty => pop() => Szero.
 * 2) else if Stfilter => filter_mutex() => Szero
 * 3) else if Szero => expand_zero => S (passing by Sfilter)
 * 4) else (S must have something) => expand_cost()
 *
 * Zero cost operators have been expanded iff !S.IsZero() && Szero.IsZero()
 */
class SymController;
class ClosedList;

class UniformCostSearch : public SymSearch {
protected:
    bool fw; // Direction of the search. true=forward, false=backward

    Estimation step_estimation;

    // Current state of the search:
    std::shared_ptr<ClosedList> closed; // Closed list is a shared ptr to share
    OpenList open_list;
    Frontier frontier;

    // Opposite direction. Mostly relevant when bidirectional search ist used
    std::shared_ptr<ClosedList> perfectHeuristic;

    bool lastStepCost; // If the last step was a cost step (to know if we are in
                       // estimationDisjCost or Zero)

    int last_g_cost;

    void violated(TruncatedReason reason, double time, int maxTime, int maxNodes);

    bool initialization() const {return frontier.g() == 0 && lastStepCost;}

    /*
     * Check if we can proof that no more plans exist
     */
    virtual bool provable_no_more_plans();

    /*
     * Check generated or closed states with other frontiers => solution check
     */
    virtual void checkFrontierCut(Bucket &bucket, int g);

    void closeStates(Bucket &bucket, int g);

    bool prepareBucket();

    virtual void filterFrontier();

    //////////////////////////////////////////////////////////////////////////////
public:
    UniformCostSearch(SymbolicSearch *eng, const SymParamsSearch &params);
    UniformCostSearch(const UniformCostSearch &) = delete;
    UniformCostSearch(UniformCostSearch &&) = default;
    UniformCostSearch &operator=(const UniformCostSearch &) = delete;
    UniformCostSearch &operator=(UniformCostSearch &&) = default;
    virtual ~UniformCostSearch() = default;

    virtual bool finished() const {
        return open_list.empty() && frontier.empty();
    }

    void step() override {
        /*if (step_estimation.get_failed()) {
            p.increase_bound();
        }
        stepImage(p.maxAllotedTime, p.maxAllotedNodes);*/
        stepImage(0, 0);
    }

    virtual std::string get_last_dir() const override {
        return fw ? "FW" : "BW";
    }

    virtual void stepImage(int maxTime, int maxNodes);

    bool
    init(std::shared_ptr<SymStateSpaceManager> manager, bool fw,
         UniformCostSearch *opposite_search); // Init forward or backward search

    virtual int getF() const override {
        return open_list.minNextG(frontier, mgr->getAbsoluteMinTransitionCost());
    }

    virtual int getG() const {
        return frontier.empty() ? open_list.minG() : frontier.g();
    }

    std::shared_ptr<ClosedList> getClosedShared() const {return closed;}

    void filterDuplicates(Bucket &bucket);

    // Returns the nodes that have been expanded by the algorithm (closed without
    // the current frontier)
    BDD getExpanded() const;
    void getNotExpanded(Bucket &res) const;

    Estimation *get_step_estimator() {return &step_estimation;}

    // void write(const std::string & file) const;

    void filterMutex(Bucket &bucket) {
        mgr->filterMutex(bucket, fw, initialization());
    }
};
}
#endif
