#ifndef SYMBOLIC_TRANSITION_RELATIONS_DISJUNCTIVE_TRANSITION_RELATION_H
#define SYMBOLIC_TRANSITION_RELATIONS_DISJUNCTIVE_TRANSITION_RELATION_H

#include "transition_relation.h"

#include "../sym_variables.h"

#include "../../task_proxy.h"

#include <set>
#include <vector>

class GlobalOperator;

namespace symbolic {
class SymStateSpaceManager;
class OriginalStateSpace;

/*
 * Represents a disjunctive transition relation with BDDs.
 */
class DisjunctiveTransitionRelation : public TransitionRelation {
    SymVariables *sym_vars; // To call basic BDD creation methods
    TaskProxy task_proxy; // Use task_proxy to access task information.
    int cost; // transition cost
    BDD tr_bdd; // bdd for making the relprod

    std::vector<int> eff_vars;   // FD Index of eff variables. Must be sorted!!
    BDD exists_vars, exists_bw_vars; // Cube with variables to existentialize
    std::vector<BDD> swap_vars, swap_vars_p; // Swap variables from unprimed to primed

    std::set<OperatorID> ops_ids; // List of operators represented by the TR

public:
    DisjunctiveTransitionRelation(SymVariables *sym_vars, OperatorID op_id, const std::shared_ptr<AbstractTask> &task);
    void init();
    void init_from_tr(const DisjunctiveTransitionRelation &other);

    void add_condition(BDD cond);

    BDD image(const BDD &from) const override;
    BDD preimage(const BDD &from) const override;
    BDD image(const BDD &from, int max_nodes) const override;
    BDD preimage(const BDD &from, int max_nodes) const override;

    virtual int nodeCount() const override;
    const OperatorID &get_unique_operator_id() const override;

    void edeletion(const std::vector<std::vector<BDD>> &notMutexBDDsByFluentFw,
                   const std::vector<std::vector<BDD>> &notMutexBDDsByFluentBw,
                   const std::vector<std::vector<BDD>> &exactlyOneBDDsByFluent);

    void disjunctive_merge(const DisjunctiveTransitionRelation &t2, int max_nodes);
    void conjunctive_merge(const DisjunctiveTransitionRelation &t2, int max_nodes);

    int get_cost() const {return cost;}

    void set_cost(int cost_) {cost = cost_;}

    const std::set<OperatorID> &get_operator_ids() const {return ops_ids;}

    void setOpsIds(const std::set<OperatorID> &operator_ids) {ops_ids = operator_ids;}

    const std::vector<int> &get_eff_vars() const {return eff_vars;}

    BDD get_exists_vars() const {return exists_vars;}
    BDD get_exists_bw_vars() const {return exists_bw_vars;}
    const std::vector<BDD> &get_swap_vars() const {return swap_vars;}
    const std::vector<BDD> &get_swap_vars_p() const {return swap_vars_p;}

    BDD get_tr_BDD() const {return tr_bdd;}

    SymVariables *get_sym_vars() const {return sym_vars;}
};
}
#endif
