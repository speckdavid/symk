#ifndef SYMBOLIC_DECISION_DIAGRAMS_EVMDD_MEDDLY_H
#define SYMBOLIC_DECISION_DIAGRAMS_EVMDD_MEDDLY_H

#ifdef DDLIBMEDDLY

#include <meddly.h>
#include <memory>
#include <vector>
#include "../../utils/timer.h"
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
  MEDDLY::dd_edge dd_edge;

  static MEDDLY::domain *domain;
  static MEDDLY::forest *forest;
  static MEDDLY::expert_forest *exp_forest;
  static int num_vars;
  static utils::Timer timer;
  static double time_limit;

public:
  static void add_options_to_parser(options::OptionParser &/*parser*/) {}
  static void parse_options(const options::Options &/*opts*/) {}
  Bdd();
  Bdd(const Bdd &other);
  Bdd(const MEDDLY::dd_edge &dd_edge);

  static Bdd BddZero();
  static Bdd BddOne();
  static Bdd BddVar(int i);
  static void initalize_manager(unsigned int numVars, unsigned int numVarsZ,
                                unsigned int numSlots, unsigned int cacheSize,
                                unsigned long maxMemory);
  static void unset_time_limit();
  static void reset_start_time();
  static void set_time_limit(unsigned long tl);

  MEDDLY::dd_edge get_lib_bdd() const;
  bool operator==(const Bdd &other) const;
  bool operator!=(const Bdd &other) const;
  // bool operator<=(const Bdd &other) const;
  // bool operator>=(const Bdd &other) const;
  // bool operator<(const Bdd &other) const;
  // bool operator>(const Bdd &other) const;
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
  /*Bdd operator^(const Bdd &other) const;
  Bdd operator^=(const Bdd &other);*/
  Bdd operator-(const Bdd &other) const;
  Bdd operator-=(const Bdd &other);

  Bdd Xnor(const Bdd &other) const;
  Bdd Or(const Bdd &other, int maxNodes) const;
  Bdd And(const Bdd &other, int maxNodes) const;
  Bdd SwapVariables(const std::vector<Bdd> &x, const std::vector<Bdd> &y) const;

  int nodeCount() const;
  bool IsZero() const;
  bool IsOne() const;
  Bdd ExistAbstract(const std::vector<Bdd> &cube, int limit = 0) const;

  Bdd RelationProductNext(const Bdd &relation, const Bdd &cube,
                          const std::vector<Bdd> &pre_vars,
                          const std::vector<Bdd> &succ_vars,
                          int max_nodes = 0) const;

  Bdd RelationProductPrev(const Bdd &relation, const Bdd &cube,
                          const std::vector<Bdd> &pre_vars,
                          const std::vector<Bdd> &succ_vars,
                          int max_nodes = 0) const;

  void check_time_limit() const;
  void toDot(const std::string &file_name) const;
};
} // namespace symbolic

#endif

#endif
