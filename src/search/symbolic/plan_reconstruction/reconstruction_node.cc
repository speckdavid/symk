#include "reconstruction_node.h"
#include "../transition_relation.h"
#include "../../utils/logging.h"

using namespace std;

namespace symbolic {
ReconstructionNode::ReconstructionNode(int g, int h, int zero_layer, BDD states, BDD visited_states,
                                       bool fwd_phase, int plan_length) :
    g(g),
    h(h),
    zero_layer(zero_layer),
    states(states),
    visited_states(visited_states),
    fwd_phase(fwd_phase),
    predecessor(nullptr),
    successor(nullptr),
    to_predecessor_tr(nullptr),
    to_successor_tr(nullptr),
    plan_length(plan_length) {}

shared_ptr<ReconstructionNode> ReconstructionNode::get_successor() const {
    return successor;
}

shared_ptr<ReconstructionNode> ReconstructionNode::get_predecessor() const {
    return predecessor;
}

shared_ptr<ReconstructionNode> ReconstructionNode::get_origin_predecessor() const {
    auto cur = make_shared<ReconstructionNode>(*this);
    while (cur->get_predecessor()) {
        cur = cur->get_predecessor();
    }
    return cur;
}

shared_ptr<ReconstructionNode> ReconstructionNode::get_origin_successor() const {
    auto cur = make_shared<ReconstructionNode>(*this);
    while (cur->get_successor()) {
        cur = cur->get_successor();
    }
    return cur;
}

shared_ptr<TransitionRelation> ReconstructionNode::get_to_predecessor_tr() const {
    return to_predecessor_tr;
}

shared_ptr<TransitionRelation> ReconstructionNode::get_to_successor_tr() const {
    return to_successor_tr;
}

void ReconstructionNode::set_predecessor(const shared_ptr<ReconstructionNode> &predecessor,
                                         const shared_ptr<TransitionRelation> &to_predecessor_tr) {
    this->predecessor = predecessor;
    this->to_predecessor_tr = to_predecessor_tr;
}

void ReconstructionNode::set_successor(const shared_ptr<ReconstructionNode> &successor,
                                       const shared_ptr<TransitionRelation> &to_successor_tr) {
    this->successor = successor;
    this->to_successor_tr = to_successor_tr;
}

bool ReconstructionNode::is_fwd_phase() const {
    return fwd_phase;
}

void ReconstructionNode::get_plan(Plan &plan) const {
    assert(plan.empty());
    // assert(this->get_f() == 0);
    shared_ptr<ReconstructionNode> cur_node = make_shared<ReconstructionNode>(*this);

    Plan suffix_plan;
    while (cur_node->get_successor()) {
        assert(cur_node->get_to_successor_tr());
        suffix_plan.push_back(cur_node->get_to_successor_tr()->getUniqueOpId());
        cur_node = cur_node->get_successor();
    }
    reverse(suffix_plan.begin(), suffix_plan.end());

    while (cur_node->get_predecessor()) {
        assert(cur_node->get_to_predecessor_tr());
        plan.push_back(cur_node->get_to_predecessor_tr()->getUniqueOpId());
        cur_node = cur_node->get_predecessor();
    }
    plan.insert(plan.end(), suffix_plan.begin(), suffix_plan.end());
    assert(plan.size() == get_plan_length());
}

BDD ReconstructionNode::get_middle_state(BDD initial_state) const {
    assert(get_successor() == nullptr);
    shared_ptr<ReconstructionNode> cur_node = make_shared<ReconstructionNode>(*this);
    BDD cur_state = initial_state;

    while (cur_node->get_predecessor()) {
        assert(cur_node->get_to_predecessor_tr());
        cur_state = cur_node->get_to_predecessor_tr()->image(cur_state);
        assert(cur_node->get_to_predecessor_tr()->get_sym_vars()->numStates(cur_state) == 1);
        cur_node = cur_node->get_predecessor();
    }
    return cur_state;
}
}
