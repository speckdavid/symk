#ifndef SYMBOLIC_DECISION_DIAGRAMS_ADD_CUDD_H
#define SYMBOLIC_DECISION_DIAGRAMS_ADD_CUDD_H

#ifdef DDLIBCUDDADD

#include "cuddObj.hh"
#include <memory>
#include "../../options/options.h"

namespace symbolic
{
/*
 * BDD-Variables for a symbolic exploration.
 * This information is global for every class using symbolic search.
 * The only decision fixed here is the variable ordering, which is assumed to be
 * always fixed.
 */

class Bdd
{
protected:
  cudd::ADD add;
  static std::unique_ptr<cudd::Cudd> manager;

public:
  static void add_options_to_parser(options::OptionParser &/*parser*/) {}
  static void parse_options(const options::Options &/*opts*/) {}

  Bdd();
  Bdd(const Bdd &add);
  Bdd(cudd::ADD add);

  static Bdd BddZero();
  static Bdd BddOne();
  static Bdd BddVar(int i);
  static void initalize_manager(unsigned int numVars, unsigned int numVarsZ,
                                unsigned int numSlots, unsigned int cacheSize,
                                unsigned long maxMemory);
  static void unset_time_limit();
  static void reset_start_time();
  static void set_time_limit(unsigned long tl);

  cudd::ADD get_lib_add() const;
  Bdd operator=(const Bdd &right);
  bool operator==(const Bdd &other) const;
  bool operator!=(const Bdd &other) const;
  bool operator<=(const Bdd &other) const;
  bool operator>=(const Bdd &other) const;
  bool operator<(const Bdd &other) const;
  bool operator>(const Bdd &other) const;
  Bdd operator!() const;
  Bdd operator~() const;
  Bdd operator*(const Bdd &other) const;
  Bdd operator*=(const Bdd &other);
  Bdd operator&(const Bdd &other) const;
  Bdd operator&=(const Bdd &other);
  Bdd operator+(const Bdd &other) const;
  Bdd operator+=(const Bdd &other);
  Bdd operator|(const Bdd &other) const;
  Bdd operator|=(const Bdd &other);
  // Bdd operator^(const Bdd &other) const;
  // Bdd operator^=(const Bdd &other);
  Bdd operator-(const Bdd &other) const;
  Bdd operator-=(const Bdd &other);

  Bdd Xnor(const Bdd &other) const;
  Bdd Or(const Bdd &other, int maxNodes) const;
  Bdd And(const Bdd &other, int maxNodes) const;
  Bdd SwapVariables(const std::vector<Bdd>& x, const std::vector<Bdd>& y) const;

  int nodeCount() const;
  bool IsZero() const;
  bool IsOne() const;
  Bdd ExistAbstract(const Bdd &cube, int maxNodes = 0) const;

  Bdd RelationProductNext(const Bdd &relation, const Bdd &cube,
                          const std::vector<Bdd> &pre_vars,
                          const std::vector<Bdd> &succ_vars,
                          int max_nodes = 0) const;

  Bdd RelationProductPrev(const Bdd &relation, const Bdd &cube,
                          const std::vector<Bdd> &pre_vars,
                          const std::vector<Bdd> &succ_vars,
                          int max_nodes = 0) const;

  void toDot(const std::string &file_name) const;
};
} // namespace symbolic

#endif

#endif