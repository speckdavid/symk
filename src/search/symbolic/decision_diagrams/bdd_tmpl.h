#ifndef SYMBOLIC_DECISION_DIAGRAMS_BDD_TMPL_H
#define SYMBOLIC_DECISION_DIAGRAMS_BDD_TMPL_H

#include "add_cudd.h"
#include "bdd_buddy.h"
#include "bdd_cacbdd.h"
#include "bdd_cudd.h"
#include "bdd_sylvan.h"
#include "evmdd_meddly.h"

namespace symbolic {
struct BDDError {};
extern void exceptionError(std::string message = "");
} // namespace symbolic

#endif
