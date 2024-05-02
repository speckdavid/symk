#include "sym_mutexes.h"

#include "sym_utils.h"

using namespace std;

namespace symbolic {
SymMutexes::SymMutexes(SymVariables *sym_vars, const SymParameters &sym_params) :
    sym_vars(sym_vars),
    sym_params(sym_params) {}

void SymMutexes::init(const shared_ptr<AbstractTask> task) {
    const vector<MutexGroup> &mutex_groups = task->get_mutex_groups();
    // If (a) is initialized OR not using mutex OR edeletion does not need mutex
    if (sym_params.mutex_type == MutexType::MUTEX_NOT)
        return; // Skip mutex initialization

    bool genMutexBDD = true;
    bool genMutexBDDByFluent = (sym_params.mutex_type == MutexType::MUTEX_EDELETION);

    if (genMutexBDDByFluent) {
        // Initialize structure for exactlyOneBDDsByFluent (common to both
        // init_mutex calls)
        exactlyOneBDDsByFluent.resize(task->get_num_variables());
        for (int i = 0; i < task->get_num_variables(); ++i) {
            exactlyOneBDDsByFluent[i].resize(task->get_variable_domain_size(i));
            for (int j = 0; j < task->get_variable_domain_size(i); ++j) {
                exactlyOneBDDsByFluent[i][j] = sym_vars->oneBDD();
            }
        }
    }
    init(task, genMutexBDD, genMutexBDDByFluent, false);
    init(task, genMutexBDD, genMutexBDDByFluent, true);
}

void SymMutexes::init(const shared_ptr<AbstractTask> task, bool genMutexBDD, bool genMutexBDDByFluent, bool fw) {
    const vector<MutexGroup> &mutex_groups = task->get_mutex_groups();

    vector<vector<BDD>> &notMutexBDDsByFluent = (fw ? notMutexBDDsByFluentFw : notMutexBDDsByFluentBw);

    vector<BDD> &notMutexBDDs = (fw ? notMutexBDDsFw : notMutexBDDsBw);

    // BDD validStates = vars->oneBDD();

    if (genMutexBDDByFluent) {
        // Initialize structure for notMutexBDDsByFluent
        notMutexBDDsByFluent.resize(task->get_num_variables());
        for (int i = 0; i < task->get_num_variables(); ++i) {
            notMutexBDDsByFluent[i].resize(task->get_variable_domain_size(i));
            for (int j = 0; j < task->get_variable_domain_size(i); ++j) {
                notMutexBDDsByFluent[i][j] = sym_vars->oneBDD();
            }
        }
    }

    // Initialize mBDDByVar and invariant_bdds_by_fluent
    vector<BDD> mBDDByVar;
    mBDDByVar.reserve(task->get_num_variables());
    vector<vector<BDD>> invariant_bdds_by_fluent(task->get_num_variables());
    for (size_t i = 0; i < invariant_bdds_by_fluent.size(); i++) {
        mBDDByVar.push_back(sym_vars->oneBDD());
        invariant_bdds_by_fluent[i].resize(task->get_variable_domain_size(i));
        for (size_t j = 0; j < invariant_bdds_by_fluent[i].size(); j++) {
            invariant_bdds_by_fluent[i][j] = sym_vars->oneBDD();
        }
    }

    for (auto &mg : mutex_groups) {
        if (mg.pruneFW() != fw)
            continue;
        const vector<FactPair> &invariant_group = mg.getFacts();
        if (mg.isExactlyOne()) {
            BDD bddInvariant = sym_vars->zeroBDD();
            int var = numeric_limits<int>::max();
            int val = 0;
            bool exactlyOneRelevant = true;

            for (auto &fluent : invariant_group) {
                bddInvariant += sym_vars->preBDD(fluent.var, fluent.value);
                if (fluent.var < var) {
                    var = fluent.var;
                    val = fluent.value;
                }
            }

            if (exactlyOneRelevant) {
                if (genMutexBDD) {
                    invariant_bdds_by_fluent[var][val] *= bddInvariant;
                }
                if (genMutexBDDByFluent) {
                    for (auto &fluent : invariant_group) {
                        exactlyOneBDDsByFluent[fluent.var][fluent.value] *= bddInvariant;
                    }
                }
            }
        }

        for (size_t i = 0; i < invariant_group.size(); ++i) {
            int var1 = invariant_group[i].var;
            int val1 = invariant_group[i].value;
            BDD f1 = sym_vars->preBDD(var1, val1);

            for (size_t j = i + 1; j < invariant_group.size(); ++j) {
                int var2 = invariant_group[j].var;
                int val2 = invariant_group[j].value;
                BDD f2 = sym_vars->preBDD(var2, val2);
                BDD mBDD = !(f1 * f2);
                if (genMutexBDD) {
                    mBDDByVar[min(var1, var2)] *= mBDD;
                    if (mBDDByVar[min(var1, var2)].nodeCount() > sym_params.max_mutex_size) {
                        notMutexBDDs.push_back(mBDDByVar[min(var1, var2)]);
                        mBDDByVar[min(var1, var2)] = sym_vars->oneBDD();
                    }
                }
                if (genMutexBDDByFluent) {
                    notMutexBDDsByFluent[var1][val1] *= mBDD;
                    notMutexBDDsByFluent[var2][val2] *= mBDD;
                }
            }
        }
    }

    if (genMutexBDD) {
        for (int var = 0; var < task->get_num_variables(); ++var) {
            if (!mBDDByVar[var].IsOne()) {
                notMutexBDDs.push_back(mBDDByVar[var]);
            }
            for (const BDD &bdd_inv : invariant_bdds_by_fluent[var]) {
                if (!bdd_inv.IsOne()) {
                    notMutexBDDs.push_back(bdd_inv);
                }
            }
        }

        merge(sym_vars, notMutexBDDs, merge_and_BDD, sym_params.max_mutex_time, sym_params.max_mutex_size);
        reverse(notMutexBDDs.begin(), notMutexBDDs.end());
    }
}
}
