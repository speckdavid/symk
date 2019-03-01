// Copyright 03.07.2018, University of Freiburg,
// Author: David Speck <speckd>

#include "userOperations.h"
#include <fstream>
#include <iomanip>
#include <limits>
#include <set>
#include <map>
#include <sstream>
#include <stack>


const float INFTY = std::numeric_limits<float>::infinity();

void USER_OPS::initializeUserOperations()
{
  USER_OPS::initializePlus();
  USER_OPS::initializeMinus();
  USER_OPS::initializeMultiply();
  USER_OPS::initializeDivide();
  USER_OPS::initializeUnionmin();
  USER_OPS::initializeIntersectionmax();
  USER_OPS::initializePartialComplement();
  USER_OPS::initializeGreaterThan();
  USER_OPS::initializeGreaterEquals();
  USER_OPS::initializeLessThan();
  USER_OPS::initializeLessEquals();
  USER_OPS::initializeEquals();
  USER_OPS::initializePow();
  USER_OPS::initializeRestrict();
  USER_OPS::initializeSwapVar();
}

template <typename T>
std::string to_string_with_precision(const T a_value, const int n = 20) {
  std::ostringstream out;
  out << std::setprecision(n) << a_value;
  // std::string res = out.str();
  // res.erase(res.find_last_not_of('0') + 1, std::string::npos);
  return out.str();
}

void USER_OPS::to_dot(const MEDDLY::dd_edge &edge, std::string file_name) {
  MEDDLY::forest *f = edge.getForest();
  MEDDLY::expert_forest *exp_forest = static_cast<MEDDLY::expert_forest *>(f);
  // Collect edge data
  std::set<int> closed_nodes;
  std::map<int, std::string> levels_to_output;
  std::string edges;

  std::stack<MEDDLY::node_handle> node_stack;
  node_stack.push(edge.getNode());
  float incoming_weight;
  edge.getEdgeValue(incoming_weight);
  edges += "\"W0\" [label = \"" + to_string_with_precision(incoming_weight) +
           "\", shape= \"box\"];\n";
  edges += "\"F0\" -> \"W0\" [dir = none];\n";
  edges += "\"W0\" -> \"" + std::to_string(edge.getNode()) + "\";\n";
  while (!node_stack.empty()) {
    // Get level information
    MEDDLY::node_handle node = node_stack.top();
    node_stack.pop();
    int level = exp_forest->getNodeLevel(node);
    if (level == 0 || closed_nodes.find(node) != closed_nodes.end()) {
      continue;
    }
    if (levels_to_output.find(level) == levels_to_output.end()) {
      levels_to_output[level] =
          "{ rank = same; \" " +
          std::string(f->getDomain()->getVar(level)->getName()) + " \";\n";
    }
    levels_to_output[level] += "\"" + std::to_string(node) + "\";";

    const int domain = exp_forest->getLevelSize(level);
    MEDDLY::unpacked_node *A =
        MEDDLY::unpacked_node::newFromNode(exp_forest, node, true);

    for (int i = 0; i < domain; i++) {
      std::string connection_name = std::to_string(node) + "-" +
                                    std::to_string(i) + "-" +
                                    std::to_string(A->d(i));
      edges += "\"" + connection_name + "\" [label = \"" +
               to_string_with_precision(A->ef(i)) + "\", shape= \"box\"];\n";
      edges +=
          "\"" + std::to_string(node) + "\" -> \"" + connection_name + "\"";
      // std::to_string(A->ef(i))
      edges += " [ label=\"" + std::to_string(i) + "\", dir = none]";
      edges += ";\n";
      edges +=
          "\"" + connection_name + "\" -> \"" + std::to_string(A->d(i)) + "\"";
      edges += ";\n";
      node_stack.push(A->d(i));
    }
    closed_nodes.insert(node);
    MEDDLY::unpacked_node::recycle(A);
  }

  std::ofstream out_file;
  out_file.open(file_name);
  out_file << "digraph \"DD\" {" << std::endl;
  out_file << "size = \"7.5,10\"" << std::endl;
  out_file << "center = true;" << std::endl;
  out_file << "{ node [shape = plaintext];" << std::endl;
  out_file << "  edge [style = invis];" << std::endl;
  out_file << "  \"CONST NODES\" [style = invis];" << std::endl;
  std::string level_out = "";
  for (auto &entry : levels_to_output) {
    level_out = " \" " +
                std::string(f->getDomain()->getVar(entry.first)->getName()) +
                " \" ->" + level_out;
  }
  out_file << level_out;
  out_file << "\"CONST NODES\";" << std::endl;
  out_file << "}" << std::endl;
  out_file << "{ rank = same; node [shape = box]; edge [style = invis];"
           << std::endl;
  out_file << "\"F0\";" << std::endl;
  out_file << "}\n" << std::endl;
  for (auto &pair : levels_to_output) {
    pair.second += "}\n";
    out_file << pair.second << std::endl;
  }
  out_file << "{ rank = same; \"CONST NODES\";" << std::endl;
  out_file << "\"-1\"" << std::endl;
  out_file << "}\n" << std::endl;

  out_file << edges << std::endl;
  out_file << "\"-1\" [label = \"T\"];\n" << std::endl;
  out_file << "\"F0\" [label = \"\", shape = none];\n" << std::endl;
  out_file << "}" << std::endl;
  out_file.close();
}





