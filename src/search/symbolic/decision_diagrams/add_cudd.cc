#ifdef DDLIBCUDDADD

#include "add_cudd.h"
#include "bdd_tmpl.h"
#include <iostream>

namespace symbolic {

std::unique_ptr<cudd::Cudd> Bdd::manager = nullptr;

Bdd Bdd::BddZero() { return Bdd(manager->addZero()); }

Bdd Bdd::BddOne() { return Bdd(manager->addOne()); }

Bdd Bdd::BddVar(int i) { return Bdd(manager->addVar(i)); }

void Bdd::initalize_manager(unsigned int numVars, unsigned int numVarsZ,
                            unsigned int numSlots, unsigned int cacheSize,
                            unsigned long /*maxMemory*/) {
  std::cout << "DD-lib: Cudd-Add" << std::endl;
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

Bdd::Bdd(const Bdd &bdd) { this->add = bdd.get_lib_add(); }

Bdd::Bdd(cudd::ADD add) { this->add = add; }

cudd::ADD Bdd::get_lib_add() const { return add; }

Bdd Bdd::operator=(const Bdd &right) { return Bdd(add = right.get_lib_add()); }

bool Bdd::operator==(const Bdd &other) const {
  return add == other.get_lib_add();
}

bool Bdd::operator!=(const Bdd &other) const {
  return add != other.get_lib_add();
}

bool Bdd::operator<=(const Bdd &other) const {
  return add <= other.get_lib_add();
}

bool Bdd::operator>=(const Bdd &other) const {
  return add >= other.get_lib_add();
}

bool Bdd::operator<(const Bdd &other) const {
  return add < other.get_lib_add();
}

bool Bdd::operator>(const Bdd &other) const {
  return add > other.get_lib_add();
}

Bdd Bdd::operator!() const { return Bdd(add.Cmpl()); }

Bdd Bdd::operator~() const { return Bdd(add.Cmpl()); }

Bdd Bdd::operator*(const Bdd &other) const {
  return Bdd(add * other.get_lib_add());
}

Bdd Bdd::operator*=(const Bdd &other) {
  add *= other.get_lib_add();
  return *this;
}

Bdd Bdd::operator&(const Bdd &other) const {
  return Bdd(add & other.get_lib_add());
}

Bdd Bdd::operator&=(const Bdd &other) {
  add &= other.get_lib_add();
  return *this;
}

Bdd Bdd::operator+(const Bdd &other) const {
  return Bdd(add | other.get_lib_add());
}

Bdd Bdd::operator+=(const Bdd &other) {
  add |= other.get_lib_add();
  return *this;
}

Bdd Bdd::operator|(const Bdd &other) const {
  return Bdd(add | other.get_lib_add());
}

Bdd Bdd::operator|=(const Bdd &other) {
  add |= other.get_lib_add();
  return *this;
}

/*Bdd Bdd::operator^(const Bdd &other) const {
  return Bdd(add ^ other.get_lib_add());
}

Bdd Bdd::operator^=(const Bdd &other)
{
  add ^= other.get_lib_add();
  return *this;
}*/

Bdd Bdd::operator-(const Bdd &other) const {
  return Bdd(add & other.get_lib_add().Cmpl());
}

Bdd Bdd::operator-=(const Bdd &other) {
  add = add & other.get_lib_add().Cmpl();
  return *this;
}

Bdd Bdd::Xnor(const Bdd &other) const {
  return Bdd(add.Xnor(other.get_lib_add()));
}

Bdd Bdd::Or(const Bdd &other, int maxNodes) const {
  if (maxNodes > 0 && (nodeCount() > maxNodes || other.nodeCount() > maxNodes)) {
    exceptionError();
  }
  Bdd result = Bdd(add.Or(other.get_lib_add()));
  if (maxNodes > 0 && result.nodeCount() > maxNodes) {
    throw BDDError();
  }
  return result;
}

Bdd Bdd::And(const Bdd &other, int maxNodes) const {
  if (maxNodes > 0 && (nodeCount() > maxNodes || other.nodeCount() > maxNodes)) {
    exceptionError();
  }
  Bdd result = Bdd(add & other.get_lib_add());
  if (maxNodes > 0 && result.nodeCount() > maxNodes) {
    throw BDDError();
  }
  return result;
}

Bdd Bdd::SwapVariables(const std::vector<Bdd>& x, const std::vector<Bdd>& y) const {
  std::vector<cudd::ADD> cudd_x;
  std::vector<cudd::ADD> cudd_y;
  for (auto b : x) {
    cudd_x.push_back(b.get_lib_add());
  }
  for (auto b : y) {
    cudd_y.push_back(b.get_lib_add());
  }
  return Bdd(add.SwapVariables(cudd_x, cudd_y));
}

int Bdd::nodeCount() const { return add.nodeCount(); }

bool Bdd::IsZero() const { return add.IsZero(); }

bool Bdd::IsOne() const { return add.IsOne(); }

Bdd Bdd::ExistAbstract(const Bdd &cube, int maxNodes) const {
  if (maxNodes > 0 && nodeCount() > maxNodes) {
        exceptionError();
  }
  Bdd result = Bdd(add.ExistAbstract(cube.get_lib_add()));
  if (maxNodes > 0 && result.nodeCount() > maxNodes) {
    throw BDDError();
  }
  return result;
}

Bdd Bdd::RelationProductNext(const Bdd &relation, const Bdd &cube,
                             const std::vector<Bdd> &pre_vars,
                             const std::vector<Bdd> &succ_vars,
                             int max_nodes) const {
  Bdd tmp = this->And(relation, max_nodes);
  tmp = tmp.ExistAbstract(cube, max_nodes);
  return tmp.SwapVariables(pre_vars, succ_vars);
}

Bdd Bdd::RelationProductPrev(const Bdd &relation, const Bdd &cube,
                             const std::vector<Bdd> &pre_vars,
                             const std::vector<Bdd> &succ_vars,
                             int max_nodes) const {
  Bdd tmp = this->SwapVariables(pre_vars, succ_vars);
  tmp = tmp.And(relation, max_nodes);
  return tmp.ExistAbstract(cube, max_nodes);
}

void Bdd::toDot(const std::string &file_name) const {
  FILE *outfile = fopen(file_name.c_str(), "w");
  DdNode **ddnodearray = (DdNode **)malloc(sizeof(add.getNode()));
  ddnodearray[0] = add.getNode(); // bdd.Add().getNode();
  Cudd_DumpDot(manager->getManager(), 1, ddnodearray, NULL, NULL,
               outfile); // dump the function to .dot file
  free(ddnodearray);
  fclose(outfile);
}

} // namespace symbolic

#endif
