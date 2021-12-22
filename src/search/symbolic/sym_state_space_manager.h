#ifndef SYMBOLIC_SYM_STATE_SPACE_MANAGER_H
#define SYMBOLIC_SYM_STATE_SPACE_MANAGER_H

#include "../tasks/root_task.h"
#include "../utils/system.h"
#include "sym_bucket.h"
#include "sym_enums.h"
#include "sym_utils.h"
#include "sym_variables.h"

#include <cassert>
#include <map>
#include <memory>
#include <set>
#include <vector>

namespace options {
class OptionParser;
class Options;
} // namespace options

namespace symbolic {
class SymVariables;
class TransitionRelation;

/*
 * All the methods may throw exceptions in case the time or nodes are exceeded.
 *
 */
class SymParamsMgr {
public:
    // Parameters to generate the TRs
    int max_tr_size, max_tr_time;

    // Parameters to generate the mutex BDDs
    MutexType mutex_type;
    int max_mutex_size, max_mutex_time;

    // Time and memory bounds for auxiliary operations
    int max_aux_nodes, max_aux_time;

    bool fast_sdac_generation;


    SymParamsMgr(const options::Options &opts,
                 const std::shared_ptr<AbstractTask> &task);
    static void add_options_to_parser(options::OptionParser &parser);
    void print_options() const;
};

class SymStateSpaceManager {
    void zero_preimage(const BDD &bdd, std::vector<BDD> &res, int maxNodes) const;
    void cost_preimage(const BDD &bdd, std::map<int, std::vector<BDD>> &res,
                       int maxNodes) const;
    void zero_image(const BDD &bdd, std::vector<BDD> &res, int maxNodes) const;
    void cost_image(const BDD &bdd, std::map<int, std::vector<BDD>> &res,
                    int maxNodes) const;

protected:
    SymVariables *vars;
    const SymParamsMgr p;

    BDD initialState; // initial state
    BDD goal; // bdd representing the true (i.e. not simplified) goal-state

    std::map<int, std::vector<TransitionRelation>> transitions; // TRs
    int min_transition_cost; // minimum cost of non-zero cost transitions
    bool hasTR0;           // If there is transitions with cost 0

    // BDD representation of valid states (wrt mutex) for fw and bw search
    std::vector<BDD> notMutexBDDsFw, notMutexBDDsBw;

    // Dead ends for fw and bw searches. They are always removed in
    // filter_mutex (it does not matter which mutex_type we are using).
    std::vector<BDD> notDeadEndFw, notDeadEndBw;

    virtual std::string tag() const = 0;

    void init_transitions(
        const std::map<int, std::vector<TransitionRelation>> &(indTRs));

public:
    SymStateSpaceManager(SymVariables *v, const SymParamsMgr &params);

    virtual ~SymStateSpaceManager() {}

    void filterMutex(Bucket &bucket, bool fw, bool initialization);
    void mergeBucket(Bucket &bucket) const;
    void mergeBucketAnd(Bucket &bucket) const;

    void shrinkBucket(Bucket &bucket, int maxNodes);

    SymVariables *getVars() const {return vars;}

    const SymParamsMgr getParams() const {return p;}

    const BDD &getGoal() {return goal;}

    const BDD &getInitialState() {return initialState;}

    BDD getBDD(int variable, int value) const {
        return vars->preBDD(variable, value);
    }

    BDD zeroBDD() const {return vars->zeroBDD();}

    BDD oneBDD() const {return vars->oneBDD();}

    const std::vector<BDD> &getNotMutexBDDs(bool fw) const {
        return fw ? notMutexBDDsFw : notMutexBDDsBw;
    }

    bool mergeBucket(Bucket &bucket, int maxTime, int maxNodes) const {
        auto mergeBDDs = [](BDD bdd, BDD bdd2, int maxNodes) {
                return bdd.Or(bdd2, maxNodes);
            };
        merge(vars, bucket, mergeBDDs, maxTime, maxNodes);
        removeZero(bucket); // Be sure that we do not contain only the zero BDD

        return bucket.size() <= 1;
    }

    bool mergeBucketAnd(Bucket &bucket, int maxTime, int maxNodes) const {
        auto mergeBDDs = [](BDD bdd, BDD bdd2, int maxNodes) {
                return bdd.And(bdd2, maxNodes);
            };
        merge(vars, bucket, mergeBDDs, maxTime, maxNodes);
        removeZero(bucket); // Be sure that we do not contain only the zero BDD

        return bucket.size() <= 1;
    }

    void dumpMutexBDDs(bool fw) const;

    // Methods that require of TRs initialized

    int getMinTransitionCost() const {
        assert(!transitions.empty());
        return min_transition_cost;
    }

    int getAbsoluteMinTransitionCost() const {
        assert(!transitions.empty());
        if (hasTR0)
            return 0;
        return min_transition_cost;
    }

    bool hasTransitions0() const {
        assert(!transitions.empty());
        return hasTR0;
    }

    void zero_image(bool fw, const BDD &bdd, std::vector<BDD> &res,
                    int maxNodes) {
        if (fw) {
            zero_image(bdd, res, maxNodes);
        } else {
            zero_preimage(bdd, res, maxNodes);
        }
    }

    void cost_image(bool fw, const BDD &bdd, std::map<int, std::vector<BDD>> &res,
                    int maxNodes) {
        if (fw) {
            cost_image(bdd, res, maxNodes);
        } else {
            cost_preimage(bdd, res, maxNodes);
        }
    }

    BDD filter_mutex(const BDD &bdd, bool fw, int maxNodes, bool initialization);

    int filterMutexBucket(std::vector<BDD> &bucket, bool fw, bool initialization,
                          int maxTime, int maxNodes);

    void setTimeLimit(int maxTime) {vars->setTimeLimit(maxTime);}

    void unsetTimeLimit() {vars->unsetTimeLimit();}

    friend std::ostream &operator<<(std::ostream &os,
                                    const SymStateSpaceManager &state_space);

    virtual void print(std::ostream &os, bool /*fullInfo*/) const {os << tag();}

    // For plan solution reconstruction. Only avaialble in original state space

    virtual const std::map<int, std::vector<TransitionRelation>> &
    getIndividualTRs() const {
        std::cerr << "Error: trying to get individual TRs from an invalid state "
            "space type"
                  << std::endl;
        utils::exit_with(utils::ExitCode::SEARCH_CRITICAL_ERROR);
    }
};
}
#endif
