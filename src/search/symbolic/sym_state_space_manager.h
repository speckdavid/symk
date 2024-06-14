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

    void filter_mutex(Bucket &bucket, bool fw, bool initialization);
    void merge_bucket(Bucket &bucket) const;
    void merge_bucket_and(Bucket &bucket) const;

    BDD get_goal() {return goal;}
    BDD get_initial_state() {return initial_state;}

    BDD zeroBDD() const {return sym_vars->zeroBDD();}
    BDD oneBDD() const {return sym_vars->oneBDD();}

    bool merge_bucket(Bucket &bucket, int maxTime, int maxNodes) const {
        auto mergeBDDs = [](BDD bdd, BDD bdd2, int maxNodes) {
                return bdd.Or(bdd2, maxNodes);
            };
        merge(sym_vars, bucket, mergeBDDs, maxTime, maxNodes);
        remove_zero(bucket); // Be sure that we do not contain only the zero BDD

        return bucket.size() <= 1;
    }

    bool merge_bucket_and(Bucket &bucket, int maxTime, int maxNodes) const {
        auto mergeBDDs = [](BDD bdd, BDD bdd2, int maxNodes) {
                return bdd.And(bdd2, maxNodes);
            };
        merge(sym_vars, bucket, mergeBDDs, maxTime, maxNodes);
        remove_zero(bucket); // Be sure that we do not contain only the zero BDD

        return bucket.size() <= 1;
    }

    bool has_zero_cost_transition() const {
        return sym_transition_relations.has_zero_cost_transition();
    }

    int get_min_transition_cost() const {
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

    void set_time_limit(int maxTime) {sym_vars->set_time_limit(maxTime);}
    void unset_time_limit() {sym_vars->unset_time_limit();}

    // For plan solution reconstruction
    std::shared_ptr<SymTransitionRelations> get_transition_relations() const {
        return std::make_shared<SymTransitionRelations>(sym_transition_relations);
    }
};
}
#endif
