#ifdef DDLIBCUDD

#include "bdd_cudd.h"
#include "bdd_tmpl.h"
#include <iostream>

namespace symbolic
{

std::unique_ptr<cudd::Cudd> Bdd::manager = nullptr;

Bdd Bdd::BddZero() { return Bdd(manager->bddZero()); }

Bdd Bdd::BddOne() { return Bdd(manager->bddOne()); }

Bdd Bdd::BddVar(int i) { return Bdd(manager->bddVar(i)); }

void Bdd::initalize_manager(unsigned int numVars, unsigned int numVarsZ,
                            unsigned int numSlots, unsigned int cacheSize,
                            unsigned long /*maxMemory*/)
{
  std::cout << "DD-lib: Cudd" << std::endl;
  manager = std::unique_ptr<cudd::Cudd>(
      new cudd::Cudd(numVars, numVarsZ, numSlots, cacheSize, 3.8e+9));
  manager->SetMaxMemory(3.8e+9); //
  manager->setHandler(exceptionError);
  manager->setTimeoutHandler(exceptionError);
  manager->setNodesExceededHandler(exceptionError);
}

void Bdd::unset_time_limit() { manager->UnsetTimeLimit(); }

void Bdd::reset_start_time() { manager->ResetStartTime(); }

void Bdd::set_time_limit(unsigned long tl) { manager->SetTimeLimit(tl); }

Bdd::Bdd() {}

Bdd::Bdd(const Bdd &bdd) { this->bdd = bdd.get_lib_bdd(); }

Bdd::Bdd(cudd::BDD bdd) { this->bdd = bdd; }

cudd::BDD Bdd::get_lib_bdd() const { return bdd; }

void Bdd::set_lib_bdd(cudd::BDD bdd)
{
  this->bdd = bdd;
}

Bdd Bdd::operator=(const Bdd &right) { return Bdd(bdd = right.get_lib_bdd()); }

bool Bdd::operator==(const Bdd &other) const
{
  return bdd == other.get_lib_bdd();
}

bool Bdd::operator!=(const Bdd &other) const
{
  return bdd != other.get_lib_bdd();
}

bool Bdd::operator<=(const Bdd &other) const
{
  return bdd <= other.get_lib_bdd();
}

bool Bdd::operator>=(const Bdd &other) const
{
  return bdd >= other.get_lib_bdd();
}

bool Bdd::operator<(const Bdd &other) const
{
  return bdd < other.get_lib_bdd();
}

bool Bdd::operator>(const Bdd &other) const
{
  return bdd > other.get_lib_bdd();
}

Bdd Bdd::operator!() const { return Bdd(!bdd); }

Bdd Bdd::operator~() const { return Bdd(~bdd); }

Bdd Bdd::operator*(const Bdd &other) const
{
  return Bdd(bdd * other.get_lib_bdd());
}

Bdd Bdd::operator*=(const Bdd &other)
{
  bdd *= other.get_lib_bdd();
  return *this;
}

Bdd Bdd::operator&(const Bdd &other) const
{
  return Bdd(bdd & other.get_lib_bdd());
}

Bdd Bdd::operator&=(const Bdd &other)
{
  bdd &= other.get_lib_bdd();
  return *this;
}

Bdd Bdd::operator+(const Bdd &other) const
{
  return Bdd(bdd + other.get_lib_bdd());
}

Bdd Bdd::operator+=(const Bdd &other)
{
  bdd += other.get_lib_bdd();
  return *this;
}

Bdd Bdd::operator|(const Bdd &other) const
{
  return Bdd(bdd | other.get_lib_bdd());
}

Bdd Bdd::operator|=(const Bdd &other)
{
  bdd |= other.get_lib_bdd();
  return *this;
}

Bdd Bdd::operator^(const Bdd &other) const
{
  return Bdd(bdd ^ other.get_lib_bdd());
}

Bdd Bdd::operator^=(const Bdd &other)
{
  bdd ^= other.get_lib_bdd();
  return *this;
}

Bdd Bdd::operator-(const Bdd &other) const
{
  return Bdd(bdd - other.get_lib_bdd());
}

Bdd Bdd::operator-=(const Bdd &other)
{
  bdd -= other.get_lib_bdd();
  return *this;
}

Bdd Bdd::Xnor(const Bdd &other) const
{
  return Bdd(bdd.Xnor(other.get_lib_bdd()));
}

Bdd Bdd::Or(const Bdd &other, int maxNodes) const
{
  return Bdd(bdd.Or(other.get_lib_bdd(), maxNodes));
}

Bdd Bdd::And(const Bdd &other, int maxNodes) const
{
  return Bdd(bdd.And(other.get_lib_bdd(), maxNodes));
}

Bdd Bdd::SwapVariables(const std::vector<Bdd> &x, const std::vector<Bdd> &y) const
{
  std::vector<cudd::BDD> cudd_x(x.size());
  std::vector<cudd::BDD> cudd_y(y.size());
  for (size_t i = 0; i < x.size(); i++)
  {
    cudd_x[i] = x.at(i).get_lib_bdd();
    cudd_y[i] = y.at(i).get_lib_bdd();
  }
  return Bdd(bdd.SwapVariables(cudd_x, cudd_y));
}

int Bdd::nodeCount() const { return bdd.nodeCount(); }

bool Bdd::IsZero() const { return bdd.IsZero(); }

bool Bdd::IsOne() const { return bdd.IsOne(); }

Bdd Bdd::AndAbstract(const Bdd &g, const Bdd &cube, unsigned int limit) const
{
  return Bdd(bdd.AndAbstract(g.get_lib_bdd(), cube.get_lib_bdd(), limit));
}

Bdd Bdd::RelationProductNext(const Bdd &relation, const Bdd &cube,
                             const std::vector<Bdd> &pre_vars,
                             const std::vector<Bdd> &succ_vars,
                             int max_nodes) const
{
  Bdd tmp = AndAbstract(relation, cube, max_nodes);
  return tmp.SwapVariables(pre_vars, succ_vars);
}

Bdd Bdd::RelationProductPrev(const Bdd &relation, const Bdd &cube,
                             const std::vector<Bdd> &pre_vars,
                             const std::vector<Bdd> &succ_vars,
                             int max_nodes) const
{
  Bdd tmp = SwapVariables(pre_vars, succ_vars);
  return tmp.AndAbstract(relation, cube, max_nodes);
}

void Bdd::toDot(const std::string &file_name, std::vector<std::string>& var_names) const
{
  std::vector<char *> names(var_names.size());
  for (size_t i = 0; i < var_names.size(); ++i) {
    names[i] = &var_names[i].front();
  }

  FILE *outfile = fopen(file_name.c_str(), "w");
  DdNode **ddnodearray = (DdNode **)malloc(sizeof(bdd.Add().getNode()));
  ddnodearray[0] = bdd.Add().getNode();
  Cudd_DumpDot(manager->getManager(), 1, ddnodearray, names.data(), NULL,
               outfile); // dump the function to .dot file
  free(ddnodearray);
  fclose(outfile);
}

} // namespace symbolic

#endif
