#include "reconstruction_node.h"

using namespace std;

namespace symbolic {
ReconstructionNode::ReconstructionNode(int g, int h, BDD states, bool fwd_phase) :
    g(g),
    h(h),
    states(states),
    fwd_phase(fwd_phase),
    predecessor(nullptr),
    successor(nullptr),
    to_predecessor_op(OperatorID::no_operator),
    to_successor_op(OperatorID::no_operator) {}

shared_ptr<ReconstructionNode> ReconstructionNode::get_successor() const {
    return successor;
}

shared_ptr<ReconstructionNode> ReconstructionNode::get_predecessor() const {
    return predecessor;
}

OperatorID ReconstructionNode::get_to_predecessor_op() const {
    return to_predecessor_op;
}

OperatorID ReconstructionNode::get_to_successor_op() const {
    return to_successor_op;
}

void ReconstructionNode::set_predecessor(const std::shared_ptr<ReconstructionNode> &predecessor,
                                         const OperatorID &to_predecessor_op) {
    this->predecessor = predecessor;
    this->to_predecessor_op = to_predecessor_op;
}

void ReconstructionNode::set_successor(const std::shared_ptr<ReconstructionNode> &successor,
                                       const OperatorID &to_successor_op) {
    this->successor = successor;
    this->to_successor_op = to_successor_op;
}

bool ReconstructionNode::is_fwd_phase() const {
    return fwd_phase;
}

bool ReconstructionNode::is_solution() const {
    return get_f() == 0;
}

void ReconstructionNode::get_plan(Plan &plan) const {
    assert(plan.empty());
    // assert(this->get_f() == 0);
    shared_ptr<ReconstructionNode> cur_node = make_shared<ReconstructionNode>(*this);

    while (cur_node->get_successor()) {
        assert(cur_node->get_to_successor_op() != OperatorID::no_operator);
        plan.push_back(cur_node->get_to_successor_op());
        cur_node = cur_node->get_successor();
    }

    reverse(plan.begin(), plan.end());

    while (cur_node->get_predecessor()) {
        assert(cur_node->get_to_predecessor_op() != OperatorID::no_operator);
        plan.push_back(cur_node->get_to_predecessor_op());
        cur_node = cur_node->get_predecessor();
    }
}
}
