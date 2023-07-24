#include "sym_utils.h"

using namespace std;

namespace symbolic {
TransitionRelation mergeTR(TransitionRelation tr, const TransitionRelation &tr2,
                           int maxSize) {
    tr.merge(tr2, maxSize);
    return tr;
}

BDD mergeAndBDD(const BDD &bdd, const BDD &bdd2, int maxSize) {
    return bdd.And(bdd2, maxSize);
}
BDD mergeOrBDD(const BDD &bdd, const BDD &bdd2, int maxSize) {
    return bdd.Or(bdd2, maxSize);
}

void partition_add_to_bdds(SymVariables *vars, ADD add, map<double, BDD> &res) {
    assert(res.empty());
    ADD cur_add = add;
    double min_value = Cudd_V(cur_add.FindMin().getNode());

    ADD inf = vars->constant(numeric_limits<double>::infinity());
    while (cur_add != inf) {
        res[min_value] = cur_add.BddInterval(min_value, min_value);
        cur_add = cur_add.Maximum(res[min_value].Add() * inf);
        min_value = Cudd_V(cur_add.FindMin().getNode());
    }
}

void partition_add_to_bdds(SymVariables *vars, ADD add, map<int, BDD> &res) {
    assert(res.empty());
    ADD cur_add = add;
    double min_value = Cudd_V(cur_add.FindMin().getNode());

    ADD inf = vars->constant(numeric_limits<double>::infinity());
    while (cur_add != inf) {
        int int_min_value = round(min_value);
        res[int_min_value] = cur_add.BddInterval(min_value, min_value);
        // vars->to_dot(res[int_min_value], "bdd_" + to_string(int_min_value) + "_util.dot");
        cur_add = cur_add + (res[int_min_value].Add() * inf);
        min_value = Cudd_V(cur_add.FindMin().getNode());
    }
}
}
