#ifndef SYMBOLIC_PLAN_RECONSTRUCTION_RECONSTRUCTION_NODE_H
#define SYMBOLIC_PLAN_RECONSTRUCTION_RECONSTRUCTION_NODE_H

#include <memory>

#include "sym_solution_cut.h"

#include "../../operator_id.h"

using Plan = std::vector<OperatorID>;

namespace symbolic {
class SymSolutionCut;
class TransitionRelation;

class ReconstructionNode {
protected:
    int g; // cost left for fwd reconstruction (towards initial state)
    int h; // cost left for bwd reconstruction (towards goal state)
    int zero_layer; // current layer of zero cost BDD
    BDD states; // relevant states
    BDD visited_states; // states visited (relevant for simple plans)
    bool fwd_phase; // reconstruction phase: changes from yes to false

    std::shared_ptr<ReconstructionNode> predecessor;
    std::shared_ptr<ReconstructionNode> successor;
    std::shared_ptr<TransitionRelation> to_predecessor_tr;
    std::shared_ptr<TransitionRelation> to_successor_tr;
    size_t plan_length;

public:
    ReconstructionNode() = delete;
    ReconstructionNode(int g, int h, int zero_layer, BDD states, BDD visited_staes, bool fwd_reconstruction, int plan_length);

    int get_g() const {return g;}
    int get_h() const {return h;}
    int get_f() const {return get_g() + get_h();}
    int get_zero_layer() const {return zero_layer;}
    BDD get_states() const {return states;}
    BDD get_visitied_states() const {return visited_states;}

    std::shared_ptr<ReconstructionNode> get_predecessor() const;
    std::shared_ptr<ReconstructionNode> get_successor() const;
    std::shared_ptr<TransitionRelation> get_to_predecessor_tr() const;
    std::shared_ptr<TransitionRelation> get_to_successor_tr() const;
    size_t get_plan_length() const {return plan_length;}

    std::shared_ptr<ReconstructionNode> get_origin_predecessor() const;
    std::shared_ptr<ReconstructionNode> get_origin_successor() const;

    void set_g(int g) {this->g = g;}
    void set_h(int h) {this->h = h;}
    void set_zero_layer(int zero_layer) {this->zero_layer = zero_layer;}
    void set_states(BDD states) {this->states = states;}
    void set_visited_states(BDD visited_states) {this->visited_states = visited_states;}
    void add_visited_states(BDD newly_visited_states) {this->visited_states += newly_visited_states;}

    void set_predecessor(const std::shared_ptr<ReconstructionNode> &predecessor,
                         const std::shared_ptr<TransitionRelation> &to_predecessor_tr);
    void set_successor(const std::shared_ptr<ReconstructionNode> &successor,
                       const std::shared_ptr<TransitionRelation> &to_successor_tr);
    void set_plan_length(size_t plan_length) {this->plan_length = plan_length;}

    bool is_fwd_phase() const;
    void set_fwd_phase(bool fwd_phase) {this->fwd_phase = fwd_phase;}

    void get_plan(Plan &plan) const;
    BDD get_middle_state(BDD initial_state) const;

    friend std::ostream &operator<<(std::ostream &os,
                                    const ReconstructionNode &node) {
        return os << "symcut{g=" << node.get_g() << ", h=" << node.get_h()
                  << ", f=" << node.get_f()
                  << ", zero_layer=" << node.get_zero_layer()
                  << ", fwd_phase=" << node.is_fwd_phase()
                  << ", |plan|=" << node.get_plan_length()
                  << ", nodes=" << node.get_states().nodeCount()
                  << ", visited_nodes=" << node.get_visitied_states().nodeCount()
               // << ", states=" << node.get_states().CountMinterm(15)
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
        if (node2.get_zero_layer() < node1.get_zero_layer())
            return true;
        if (node2.get_zero_layer() > node1.get_zero_layer())
            return false;
        return node2.get_plan_length() > node1.get_plan_length();
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
