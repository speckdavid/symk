#ifndef SYMBOLIC_SYM_STATE_SPACE_MANAGER_H
#define SYMBOLIC_SYM_STATE_SPACE_MANAGER_H


#include "sym_bucket.h"
#include "sym_enums.h"
#include "sym_parameters.h"
#include "sym_utils.h"
#include "sym_variables.h"

#include "../tasks/root_task.h"
#include "../utils/system.h"

#include <cassert>
#include <map>
#include <memory>
#include <set>
#include <vector>

namespace extra_tasks {
class SdacTask;
}

namespace options {
class OptionParser;
class Options;
}

namespace symbolic {
class SymVariables;
class TransitionRelation;

class SymStateSpaceManager {
protected:
    SymVariables *sym_vars;
    const SymParameters &sym_params;
    const std::shared_ptr<AbstractTask> task;

    BDD initial_state; // initial state
    BDD goal; // bdd representing the true (i.e. not simplified) goal-state

    std::map<int, std::vector<TransitionRelation>> indTRs; // individual TRs (for plan reconstruction)
    std::map<int, std::vector<TransitionRelation>> transitions; // Merged TRs
    int min_transition_cost; // minimum cost of non-zero cost transitions
    bool has_zero_cost_transition;           // If there is transitions with cost 0

    // BDD representation of valid states (wrt mutex) for fw and bw search
    std::vector<BDD> notMutexBDDsFw, notMutexBDDsBw;
    // Dead ends for fw and bw searches. They are always removed in
    // filter_mutex (it does not matter which mutex_type we are using).
    std::vector<BDD> notDeadEndFw, notDeadEndBw;
    // notMutex relative for each fluent
    std::vector<std::vector<BDD>> notMutexBDDsByFluentFw, notMutexBDDsByFluentBw;
    std::vector<std::vector<BDD>> exactlyOneBDDsByFluent;

    void init_individual_transitions();
    void init_transitions(const std::map<int, std::vector<TransitionRelation>> &(indTRs));
    void create_single_trs();
    void create_single_sdac_trs(std::shared_ptr<extra_tasks::SdacTask> sdac_task, bool fast_creation);

    void init_mutex(const std::vector<MutexGroup> &mutex_groups);
    void init_mutex(const std::vector<MutexGroup> &mutex_groups, bool genMutexBDD, bool genMutexBDDByFluent, bool fw);

    // All the methods may throw exceptions in case the time or nodes are exceeded.
    void zero_preimage(BDD bdd, std::vector<BDD> &res, int max_nodes) const;
    void cost_preimage(BDD bdd, std::map<int, std::vector<BDD>> &res, int max_nodes) const;
    void zero_image(BDD bdd, std::vector<BDD> &res, int maxn_nodes) const;
    void cost_image(BDD bdd, std::map<int, std::vector<BDD>> &res, int max_nodes) const;

public:
    SymStateSpaceManager(SymVariables *sym_vars, const SymParameters &sym_params, const std::shared_ptr<AbstractTask> &task);

    virtual ~SymStateSpaceManager() {}

    void filterMutex(Bucket &bucket, bool fw, bool initialization);
    void mergeBucket(Bucket &bucket) const;
    void mergeBucketAnd(Bucket &bucket) const;

    BDD getGoal() {return goal;}
    BDD getInitialState() {return initial_state;}

    BDD zeroBDD() const {return sym_vars->zeroBDD();}
    BDD oneBDD() const {return sym_vars->oneBDD();}

    bool mergeBucket(Bucket &bucket, int maxTime, int maxNodes) const {
        auto mergeBDDs = [](BDD bdd, BDD bdd2, int maxNodes) {
                return bdd.Or(bdd2, maxNodes);
            };
        merge(sym_vars, bucket, mergeBDDs, maxTime, maxNodes);
        removeZero(bucket); // Be sure that we do not contain only the zero BDD

        return bucket.size() <= 1;
    }

    bool mergeBucketAnd(Bucket &bucket, int maxTime, int maxNodes) const {
        auto mergeBDDs = [](BDD bdd, BDD bdd2, int maxNodes) {
                return bdd.And(bdd2, maxNodes);
            };
        merge(sym_vars, bucket, mergeBDDs, maxTime, maxNodes);
        removeZero(bucket); // Be sure that we do not contain only the zero BDD

        return bucket.size() <= 1;
    }

    // Methods that require of TRs initialized

    int getMinTransitionCost() const {
        assert(!transitions.empty());
        return min_transition_cost;
    }

    int getAbsoluteMinTransitionCost() const {
        assert(!transitions.empty());
        return has_zero_cost_transition ? 0 : min_transition_cost;
    }

    bool hasTransitions0() const {
        assert(!transitions.empty());
        return has_zero_cost_transition;
    }

    void zero_image(bool fw, BDD bdd, std::vector<BDD> &res, int max_nodes) {
        if (fw) {
            zero_image(bdd, res, max_nodes);
        } else {
            zero_preimage(bdd, res, max_nodes);
        }
    }

    void cost_image(bool fw, BDD bdd, std::map<int, std::vector<BDD>> &res, int max_nodes) {
        if (fw) {
            cost_image(bdd, res, max_nodes);
        } else {
            cost_preimage(bdd, res, max_nodes);
        }
    }

    BDD filter_mutex(BDD bdd, bool fw, int maxNodes, bool initialization);
    int filterMutexBucket(std::vector<BDD> &bucket, bool fw, bool initialization, int max_time, int max_nodes);

    void setTimeLimit(int maxTime) {sym_vars->setTimeLimit(maxTime);}
    void unsetTimeLimit() {sym_vars->unsetTimeLimit();}

    // For plan solution reconstruction
    const std::map<int, std::vector<TransitionRelation>> &getIndividualTRs() const {
        return indTRs;
    }
};
}
#endif
