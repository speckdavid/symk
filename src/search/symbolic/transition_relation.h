#ifndef SYMBOLIC_TRANSITION_RELATION_H
#define SYMBOLIC_TRANSITION_RELATION_H

#include "sym_variables.h"

#include "../task_proxy.h"
#include <set>
#include <vector>

class GlobalOperator;

namespace symbolic {
class SymStateSpaceManager;
class OriginalStateSpace;

/*
 * Represents a symbolic transition.
 * It has two differentiated parts: label and abstract state transitions
 * Label refers to variables not considered in the merge-and-shrink
 * Each label has one or more abstract state transitions
 */
class TransitionRelation {
  SymVariables *sV; // To call basic BDD creation methods
  int cost;         // transition cost
  Bdd tBDD;         // bdd for making the relprod

  std::vector<int> effVars;     // FD Index of eff variables. Must be sorted!!
  Bdd existsVars, existsBwVars; // Cube with variables to existentialize
  std::vector<Bdd> swapVarsS,
      swapVarsSp; // Swap variables s to sp and viceversa

  std::set<OperatorID> ops_ids; // List of operators represented by the TR

public:
  // Constructor for transitions irrelevant for the abstraction
  TransitionRelation(SymVariables *sVars, OperatorID op_id, int cost_);
  void init();

  // Copy constructor
  TransitionRelation(const TransitionRelation &) = default;

  Bdd image(const Bdd &from) const;
  Bdd preimage(const Bdd &from) const;
  Bdd image(const Bdd &from, int maxNodes) const;
  Bdd preimage(const Bdd &from, int maxNodes) const;

  void edeletion(const std::vector<std::vector<Bdd>> &notMutexBDDsByFluentFw,
                 const std::vector<std::vector<Bdd>> &notMutexBDDsByFluentBw,
                 const std::vector<std::vector<Bdd>> &exactlyOneBDDsByFluent);

  void merge(const TransitionRelation &t2, int maxNodes);

  int getCost() const { return cost; }

  void set_cost(int cost_) { cost = cost_; }

  int nodeCount() const { return tBDD.nodeCount(); }

  const std::set<OperatorID> &getOpsIds() const { return ops_ids; }

  const Bdd &getBDD() const { return tBDD; }

  friend std::ostream &operator<<(std::ostream &os,
                                  const TransitionRelation &tr);
};
} // namespace symbolic
#endif