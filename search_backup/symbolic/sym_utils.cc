#include "sym_utils.h"

namespace symbolic {
TransitionRelation mergeTR(TransitionRelation tr, const TransitionRelation &tr2, int maxSize) {
    tr.merge(tr2, maxSize);
    return tr;
}

Bdd mergeAndBDD(const Bdd &bdd, const Bdd &bdd2, int maxSize) {
    return bdd.And(bdd2, maxSize);
}
Bdd mergeOrBDD(const Bdd &bdd, const Bdd &bdd2, int maxSize) {
    return bdd.Or(bdd2, maxSize);
}
}