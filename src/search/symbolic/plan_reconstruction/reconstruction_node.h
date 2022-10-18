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
                  << ", nodes=" << node.get_states().nodeCount() << "}";
    }
};

// struct CompareReconstructionNodes {
// public:
//     bool operator()(const ReconstructionNode &node1, ReconstructionNode &node2) {
//         return node2.get_f() < node1.get_f() ||
//                (node2.get_f() == node1.get_f() && node2.get_g() < node1.get_g());
//     }
// };

// Correct for sorting according to plan cost and than plan length!
struct CompareReconstructionNodes {
public:
    bool operator()(const ReconstructionNode &node1, ReconstructionNode &node2) {
        if (node2.get_plan_length() < node1.get_plan_length()) return true;
        if (node2.get_plan_length() > node1.get_plan_length()) return false;

        return node2.get_f() < node1.get_f() ||
               (node2.get_f() == node1.get_f() && node2.get_g() < node1.get_g());
    }
};

// struct CompareReconstructionNodes {
// public:
//     bool operator()(const ReconstructionNode &node1, ReconstructionNode &node2) {

//         return node2.get_f() < node1.get_f() ||
//                (node2.get_f() == node1.get_f() && node2.get_g() < node1.get_g()) ||
//                (node2.get_f() == node1.get_f() && node2.get_g() == node1.get_g() && node2.get_plan_length() < node1.get_plan_length());
//     }
// };
}
#endif
