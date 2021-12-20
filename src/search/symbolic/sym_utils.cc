#include "sym_utils.h"

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
}
