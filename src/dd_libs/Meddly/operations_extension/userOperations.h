// Copyright 03.07.2017, University of Freiburg,
// Author: David Speck <speckd>.

#ifndef USEROPERATIONS_H_
#define USEROPERATIONS_H_

#include <meddly.h>
#include <meddly_expert.h>
#include "evplus.h"
#include "minus.h"
#include "multiply.h"
#include "divide.h"
#include "intersectionmax.h"
#include "unionmin.h"
#include "greaterThan.h"
#include "greaterEquals.h"
#include "lessThan.h"
#include "lessEquals.h"
#include "equals.h"
#include "pow.h"
#include "partialcomplement.h"
#include "restrict.h"
#include "swapVar.h"

extern const float INFTY;

namespace USER_OPS
{
void initializeUserOperations();

void to_dot(const MEDDLY::dd_edge &edge, std::string file_name);
}

#endif // USEROPERATIONS_H_
