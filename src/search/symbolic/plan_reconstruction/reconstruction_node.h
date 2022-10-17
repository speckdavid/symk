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

public:
    ReconstructionNode() = delete;
    ReconstructionNode(int g, int h, BDD states, bool fwd_reconstruction);

    int get_g() const {return g;}
    int get_h() const {return h;}
    int get_f() const {return get_g() + get_h();}
    BDD get_states() const {return states;}

    std::shared_ptr<ReconstructionNode> get_predecessor() const;
    std::shared_ptr<ReconstructionNode> get_successor() const;
    OperatorID get_to_predecessor_op() const;
    OperatorID get_to_successor_op() const;

    void set_g(int g) {this->g = g;}
    void set_h(int h) {this->h = h;}
    void set_state(BDD states) {this->states = states;}

    void set_predecessor(const std::shared_ptr<ReconstructionNode> &predecessor,
                         const OperatorID &to_predecessor_op);
    void set_successor(const std::shared_ptr<ReconstructionNode> &successor,
                       const OperatorID &to_successor_op);

    bool is_fwd_phase() const;

    bool is_solution() const;

    void get_plan(Plan &plan) const;

    friend std::ostream &operator<<(std::ostream &os,
                                    const ReconstructionNode &node) {
        return os << "symcut{g=" << node.get_g() << ", h=" << node.get_h()
                  << ", f=" << node.get_f()
                  << ", fwd_phase=" << node.is_fwd_phase()
                  << ", nodes=" << node.get_states().nodeCount() << "}";
    }
};
}
#endif
