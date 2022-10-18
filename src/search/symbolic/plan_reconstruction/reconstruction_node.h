#ifndef SYMBOLIC_PLAN_RECONSTRUCTION_RECONSTRUCTION_NODE_H
#define SYMBOLIC_PLAN_RECONSTRUCTION_RECONSTRUCTION_NODE_H

#include <memory>

#include "sym_solution_cut.h"

#include "../../operator_id.h"

using Plan = std::vector<OperatorID>;

namespace symbolic {
class SymSolutionCut;

class ReconstructionNode {
protected:
    int g; // cost left for fwd reconstruction (towards initial state)
    int h; // cost left for bwd reconstruction (towards goal state)
    BDD states; // relevant states
    bool fwd_phase; // reconstruction phase: changes from yes to false

    std::shared_ptr<ReconstructionNode> predecessor;
    std::shared_ptr<ReconstructionNode> successor;
    OperatorID to_predecessor_op;
    OperatorID to_successor_op;
    size_t plan_length;

public:
    ReconstructionNode() = delete;
    ReconstructionNode(int g, int h, BDD states, bool fwd_reconstruction, int plan_length);

    int get_g() const {return g;}
    int get_h() const {return h;}
    int get_f() const {return get_g() + get_h();}
    BDD get_states() const {return states;}

    std::shared_ptr<ReconstructionNode> get_predecessor() const;
    std::shared_ptr<ReconstructionNode> get_successor() const;
    OperatorID get_to_predecessor_op() const;
    OperatorID get_to_successor_op() const;
    size_t get_plan_length() const {return plan_length;}

    std::shared_ptr<ReconstructionNode> get_origin_predecessor() const;
    std::shared_ptr<ReconstructionNode> get_origin_successor() const;

    void set_g(int g) {this->g = g;}
    void set_h(int h) {this->h = h;}
    void set_state(BDD states) {this->states = states;}

    void set_predecessor(const std::shared_ptr<ReconstructionNode> &predecessor,
                         const OperatorID &to_predecessor_op);
    void set_successor(const std::shared_ptr<ReconstructionNode> &successor,
                       const OperatorID &to_successor_op);
    void set_plan_length(size_t plan_length) {this->plan_length = plan_length;}

    bool is_fwd_phase() const;
    void set_fwd_phase(bool fwd_phase) {this->fwd_phase = fwd_phase;}

    void get_plan(Plan &plan) const;

    friend std::ostream &operator<<(std::ostream &os,
                                    const ReconstructionNode &node) {
        return os << "symcut{g=" << node.get_g() << ", h=" << node.get_h()
                  << ", f=" << node.get_f()
                  << ", fwd_phase=" << node.is_fwd_phase()
                  << ", |plan|=" << node.get_plan_length()
                  << ", nodes=" << node.get_states().nodeCount()
                  << "}";
    }
};

// TODO: We can try to de
// TODO: We necessarily need to select here a reasonable choice:
// Sort only by cost => sort by f, then g, then plan length
// Sort by cost then by plan length => sort by plan_length, than f, than g
// However we can do some tricks: unit costs => simply sort by cost
// For simple planning we probably also want to take the number of states into account...

enum class ReconstructionPriority {
    REMAINING_COST = 0,
    PLAN_LENGTH = 1
};

struct CompareReconstructionNodes {
protected:
    ReconstructionPriority prio;
public:
    CompareReconstructionNodes() {}
    CompareReconstructionNodes(ReconstructionPriority prio) : prio(prio) {}

    bool sort_by_remaining_cost(const ReconstructionNode &node1, const ReconstructionNode &node2) {
        if (node2.get_f() < node1.get_f())
            return true;
        if (node2.get_f() > node1.get_f())
            return false;
        if (node2.get_g() < node1.get_g())
            return true;
        if (node2.get_g() > node1.get_g())
            return false;
        if (!node2.is_fwd_phase() && node1.is_fwd_phase())
            return true;
        if (node2.is_fwd_phase() && !node1.is_fwd_phase())
            return false;
        return node2.get_plan_length() < node1.get_plan_length();
    }

    bool sort_by_plan_length(const ReconstructionNode &node1, const ReconstructionNode &node2) {
        if (node2.get_plan_length() < node1.get_plan_length())
            return true;
        if (node2.get_plan_length() > node1.get_plan_length())
            return false;
        if (node2.get_f() < node1.get_f())
            return true;
        if (node2.get_f() > node1.get_f())
            return false;
        if (node2.get_g() < node1.get_g())
            return true;
        if (node2.get_g() > node1.get_g())
            return false;
        return !node2.is_fwd_phase() && node1.is_fwd_phase();
    }

    bool operator()(const ReconstructionNode &node1, const ReconstructionNode &node2) {
        switch (prio) {
        case ReconstructionPriority::REMAINING_COST:
            return sort_by_remaining_cost(node1, node2);
        case ReconstructionPriority::PLAN_LENGTH:
            return sort_by_plan_length(node1, node2);
        default:
            utils::exit_with(utils::ExitCode::SEARCH_UNSUPPORTED);
        }
        utils::exit_with(utils::ExitCode::SEARCH_UNSUPPORTED);
        return false;
    }
};
}
#endif
