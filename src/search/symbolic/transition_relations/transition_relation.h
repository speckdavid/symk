#ifndef SYMBOLIC_TRANSITION_RELATIONS_TRANSITION_RELATION_H
#define SYMBOLIC_TRANSITION_RELATIONS_TRANSITION_RELATION_H

#include "cuddObj.hh"

#include "../../task_proxy.h"

namespace symbolic {
/*
 * Represents a base transition relation with BDDs.
 */
class TransitionRelation {
public:
    TransitionRelation() = default;

    virtual ~TransitionRelation() = default;

    virtual BDD image(const BDD &from, int maxNodes = 0U) const = 0;
    virtual BDD preimage(const BDD &from, int maxNodes = 0U) const = 0;
    virtual BDD preimage(const BDD &from, const BDD &constraint_to, int max_nodes = 0U) const = 0;

    virtual int size() const {return 1;}

    // It's important to retain the name "nodeCount" as BDDs share the same functionality,
    // allowing us to utilize it within the templated merge functions.
    virtual int nodeCount() const = 0;

    virtual const OperatorID &get_unique_operator_id() const = 0;
};

typedef std::shared_ptr<TransitionRelation> TransitionRelationPtr;
}
#endif
