#include "plan_graph.h"
#include "../tasks/root_task.h"

namespace plan_graph
{
PlanGraph::PlanGraph(const std::vector<Plan> &plans,
                     const TaskProxy &task_proxy)
    : plans(plans), task_proxy(task_proxy), state_registry(task_proxy)
{
  build_graph();
}

void PlanGraph::build_graph()
{
  std::cout << "Build plan graph..." << std::flush;
  // Simulate plans and build the graph
  std::cout << plans.size() << std::endl;
  for (auto &plan : plans)
  {
    GlobalState cur = state_registry.get_initial_state();
    if (get_state_position(cur.get_id()) == -1)
    {
      nodes.push_back(cur.get_id());
    }
    for (auto &op : plan)
    {
      contained_operators.insert(op);
      GlobalState next = state_registry.get_successor_state(
          cur, task_proxy.get_operators()[op]);
      if (get_state_position(next.get_id()) == -1)
      {
        nodes.push_back(next.get_id());
      }
      Edge cur_edge(cur.get_id(), next.get_id());
      if (labels.count(cur_edge) == 0)
      {
        labels[cur_edge] = std::set<OperatorID>();
      }
      labels[cur_edge].insert(op);
      cur = next;
    }
  }

  std::cout << std::endl
            << "Registered: " << state_registry.size() << std::endl;

  for (auto edge_label : labels)
  {
    for (auto op : edge_label.second)
    {
      if (ops_to_edges.count(op) == 0)
      {
        ops_to_edges[op] = std::vector<Edge>();
      }
      ops_to_edges[op].push_back(edge_label.first);
    }
  }

  std::cout << "done!" << std::endl;
  std::cout << "#Verticies: " << get_num_nodes() << std::endl;
  std::cout << "#Edges: " << get_num_edges() << std::endl;
}

} // namespace plan_graph