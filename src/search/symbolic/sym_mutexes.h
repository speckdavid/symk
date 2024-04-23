#ifndef SYMBOLIC_SYM_MUTEXES_H
#define SYMBOLIC_SYM_MUTEXES_H

#include <algorithm>

#include "sym_parameters.h"
#include "sym_variables.h"

#include "../mutex_group.h"

namespace symbolic {
struct SymMutexes {
    SymVariables *sym_vars;
    const SymParameters &sym_params;

    // BDD representation of valid states (wrt mutex) for fw and bw search
    std::vector<BDD> notMutexBDDsFw, notMutexBDDsBw;

    // Dead ends for fw and bw searches. They are always removed in
    // filter_mutex (it does not matter which mutex_type we are using).
    std::vector<BDD> notDeadEndFw, notDeadEndBw;

    // notMutex relative for each fluent
    std::vector<std::vector<BDD>> notMutexBDDsByFluentFw, notMutexBDDsByFluentBw;
    std::vector<std::vector<BDD>> exactlyOneBDDsByFluent;

    SymMutexes(SymVariables *sym_vars, const SymParameters &sym_params);

    void init(const std::shared_ptr<AbstractTask> task);

protected:
    void init(const std::shared_ptr<AbstractTask> task, bool genMutexBDD, bool genMutexBDDByFluent, bool fw);
};
}

#endif
