#ifndef TASK_UTILS_PLAN_GRAPH_H
#define TASK_UTILS_PLAN_GRAPH_H

#include "../state_registry.h"
#include "../task_proxy.h"
#include <map>
#include <set>
#include <vector>

class OperatorID;

using Plan = std::vector<OperatorID>;
using Edge = std::pair<StateID, StateID>;

namespace plan_graph
{
class PlanGraph
{
  protected:
    std::vector<Plan> plans;
    TaskProxy task_proxy;
    StateRegistry state_registry;
    std::vector<StateID> nodes; // We need to enumerate them
    std::map<StateID, std::vector<StateID>> adj;
    std::map<Edge, std::set<OperatorID>> labels;
    std::set<OperatorID> contained_operators;

    // Helper structure
    std::map<OperatorID, std::vector<Edge>> ops_to_edges;

    void build_graph();

    int get_state_position(StateID state_id) const
    {
        auto it = std::find(nodes.begin(), nodes.end(), state_id);
        return it != nodes.end() ? std::distance(nodes.begin(), it) : -1;
    }

  public:
    PlanGraph(const std::vector<Plan> &plans, const TaskProxy &task_proxy);

    size_t get_num_nodes() const { return nodes.size(); }

    size_t get_num_edges() const
    {
        int result = 0;
        for (auto &op : contained_operators)
        {
            result += ops_to_edges.at(op).size();
        }
        return result;
    }

    bool is_contained(OperatorID op_id) const
    {
        return contained_operators.count(op_id) > 0;
    }

    int get_num_of_edges(OperatorID op_id) const
    {
        return ops_to_edges.at(op_id).size();
    }

    int get_source_state(OperatorID op_id, int edge_id) const
    {
        return get_state_position(ops_to_edges.at(op_id).at(edge_id).first);
    }

    int get_target_state(OperatorID op_id, int edge_id) const
    {
        return get_state_position(ops_to_edges.at(op_id).at(edge_id).second);
    }

    // TODO: we should not iterate in such a naive way
    int get_op(int edge_id) const
    {
        int cur = 0;
        for (auto &op_edges : ops_to_edges)
        {
            if (edge_id < (int)(cur + op_edges.second.size()))
            {
                return op_edges.first.get_index();
            }
            cur += op_edges.second.size();
        }
        return -1;
    }

    int get_source_state(int edge_id) const
    {
        int cur = 0;
        for (auto &op_edges : ops_to_edges)
        {
            for (auto &e : op_edges.second)
            {
                if (cur == edge_id)
                {
                    return get_state_position(e.first);
                }
                cur++;
            }
        }
        return -1;
    }

    int get_target_state(int edge_id) const
    {
        int cur = 0;
        for (auto &op_edges : ops_to_edges)
        {
            for (auto &e : op_edges.second)
            {
                if (cur == edge_id)
                {
                    return get_state_position(e.second);
                }
                cur++;
            }
        }
        return -1;
    }
};
} // namespace plan_graph
#endif