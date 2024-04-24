#include "sym_utils.h"

namespace symbolic {
DisjunctiveTransitionRelation disjunctive_tr_merge(DisjunctiveTransitionRelation tr,
                                                   const DisjunctiveTransitionRelation &tr2,
                                                   int maxSize) {
    tr.disjunctive_merge(tr2, maxSize);
    return tr;
}

DisjunctiveTransitionRelation conjunctive_tr_merge(DisjunctiveTransitionRelation tr,
                                                   const DisjunctiveTransitionRelation &tr2,
                                                   int maxSize) {
    tr.conjunctive_merge(tr2, maxSize);
    return tr;
}

BDD merge_and_BDD(const BDD &bdd, const BDD &bdd2, int maxSize) {
    return bdd.And(bdd2, maxSize);
}
BDD merge_or_BDD(const BDD &bdd, const BDD &bdd2, int maxSize) {
    return bdd.Or(bdd2, maxSize);
}
}
