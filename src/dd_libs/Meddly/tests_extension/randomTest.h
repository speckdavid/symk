// Copyright 03.07.2017, University of Freiburg,
// Author: David Speck <speckd>.

#ifndef RANDOMTEST_H_
#define RANDOMTEST_H_

#include <random>
#include <algorithm>
#include <limits>
#include <gtest/gtest.h>
#include <meddly.h>
#include "assignments.h"

float plusFloat(float a, float b);
float minusFloat(float a, float b);
float multiplyFloat(float a, float b);
float divideFloat(float a, float b);
float minFloat(float a, float b);
float maxFloat(float a, float b);
float greaterThanFloat(float a, float b);
float greaterEqualsFloat(float a, float b);
float lessThanFloat(float a, float b);
float lessEqualsFloat(float a, float b);
float equalsFloat(float a, float b);
float cmplFloat(float a);
float pow2Float(float a);
float pow3Float(float a);
float powPiFloat(float a);

void make_rnd_test(float min_var, float max_var, float min_lim, float max_lim,
const MEDDLY::binary_opname* op, float (*eval)(float, float), bool add_inf = true , bool non_zero = false);

void make_rnd_test(float min_var, float max_var, float min_lim, float max_lim,
const MEDDLY::unary_opname* op, float (*eval)(float) );

#endif // RANDOMTEST_H_