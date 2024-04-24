#ifndef SYMBOLIC_SYM_STATE_SPACE_MANAGER_H
#define SYMBOLIC_SYM_STATE_SPACE_MANAGER_H


#include "sym_bucket.h"
#include "sym_enums.h"
#include "sym_mutexes.h"
#include "sym_parameters.h"
#include "sym_transition_relations.h"
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

class SymStateSpaceManager {
protected:
    SymVariables *sym_vars;
    const SymParameters &sym_params;
    const std::shared_ptr<AbstractTask> task;

    BDD initial_state;
    BDD goal;

    SymMutexes sym_mutexes;
    SymTransitionRelations sym_transition_relations;

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

    bool hasTransitions0() const {
        return sym_transition_relations.has_zero_cost_transition();
    }

    int getAbsoluteMinTransitionCost() const {
        return sym_transition_relations.get_min_transition_cost();
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
    const std::map<int, std::vector<DisjunctiveTransitionRelation>> &getIndividualTRs() const {
        return sym_transition_relations.get_individual_transition_relations();
    }
};
}
#endif
