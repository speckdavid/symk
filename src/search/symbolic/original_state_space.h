#ifndef SYMBOLIC_ORIGINAL_STATE_SPACE_H
#define SYMBOLIC_ORIGINAL_STATE_SPACE_H

#include "../mutex_group.h"
#include "sym_state_space_manager.h"

namespace extra_tasks {
class SdacTask;
}

namespace symbolic {
class OriginalStateSpace : public SymStateSpaceManager {
    // Hold a reference to the task implementation and pass it to objects that
    // need it.
    const std::shared_ptr<AbstractTask> task;

    void create_single_trs();
    void create_single_sdac_trs(std::shared_ptr<extra_tasks::SdacTask> sdac_task,
                                bool fast_creation);


    void init_mutex(const std::vector<MutexGroup> &mutex_groups);
    void init_mutex(const std::vector<MutexGroup> &mutex_groups, bool genMutexBDD,
                    bool genMutexBDDByFluent, bool fw);

public:
    OriginalStateSpace(SymVariables *v, const SymParamsMgr &params,
                       const std::shared_ptr<AbstractTask> &task);

    // Individual TRs: Useful for shrink and plan construction
    std::map<int, std::vector<TransitionRelation>> indTRs;

    // notMutex relative for each fluent
    std::vector<std::vector<BDD>> notMutexBDDsByFluentFw, notMutexBDDsByFluentBw;
    std::vector<std::vector<BDD>> exactlyOneBDDsByFluent;

    virtual std::string tag() const override {return "original";}

    // Methods that require of mutex initialized

    inline const BDD &getNotMutexBDDFw(int var, int val) const {
        return notMutexBDDsByFluentFw[var][val];
    }

    // Methods that require of mutex initialized

    inline const BDD &getNotMutexBDDBw(int var, int val) const {
        return notMutexBDDsByFluentBw[var][val];
    }

    // Methods that require of mutex initialized

    inline const BDD &getExactlyOneBDD(int var, int val) const {
        return exactlyOneBDDsByFluent[var][val];
    }

    virtual const std::map<int, std::vector<TransitionRelation>> &
    getIndividualTRs() const {
        return indTRs;
    }
};
}
#endif
