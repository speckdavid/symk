// Copyright 03.07.2017, University of Freiburg,
// Author: David Speck <speckd>.

#include <algorithm>
#include "randomTest.h"
#include "../operations_extension/userOperations.h"
#include <math.h>

float plusFloat(float a, float b)
{
  return a + b;
}

float minusFloat(float a, float b)
{
  return a - b;
}

float multiplyFloat(float a, float b)
{
  return a * b;
}

float divideFloat(float a, float b)
{
  return a / b;
}

float minFloat(float a, float b)
{
  return std::min(a, b);
}

float maxFloat(float a, float b)
{
  return std::max(a, b);
}

float greaterThanFloat(float a, float b)
{
  return isgreater(a, b) ? 1 : 0;
}

float greaterEqualsFloat(float a, float b)
{
  return isgreaterequal(a, b) ? 1 : 0;
}

float lessThanFloat(float a, float b)
{
  return isless(a, b) ? 1 : 0;
}

float lessEqualsFloat(float a, float b)
{
  return islessequal(a, b) ? 1 : 0;
}

float equalsFloat(float a, float b)
{
  bool equal = std::fabs(a - b) < std::numeric_limits<float>::epsilon();
  return equal ? 1 : 0;
}

float pow2Float(float a)
{
  return std::pow(a, 2);
}

float cmplFloat(float a)
{
  return a == std::numeric_limits<float>::infinity() ? 0 : std::numeric_limits<float>::infinity();
}

void make_rnd_test(float min_var, float max_var, float min_lim, float max_lim,
                   const MEDDLY::binary_opname *op, float (*eval)(float, float), 
                   bool add_inf, bool non_zero)
{
  float max = std::numeric_limits<float>::infinity();
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<int> dist(min_var, max_var);
  std::uniform_int_distribution<int> dist2(min_lim, max_lim);
  int nVars = dist(mt);
  int *limits = new int[nVars];
  int nAssigns = 1;
  for (int i = 0; i < nVars; i++)
  {
    limits[i] = dist2(mt);
    nAssigns *= limits[i];
  }
  int **assigns = assignments(nVars, limits);
  float *results1 = new float[nAssigns];
  float *results2 = new float[nAssigns];
  for (int i = 0; i < nAssigns; i++)
  {
    results1[i] = static_cast<float>(dist2(mt));
    results2[i] = static_cast<float>(dist2(mt));
    if (results1[i] == max_lim && add_inf)
      results1[i] = max;
    if (results2[i] == max_lim && add_inf)
      results2[i] = max;
    if (results2[i] == 0 && non_zero) {
      results2[i] = 1;
      std::cout << "non-zero" << std::endl;
    }
  }
  // Create EVMDD
  MEDDLY::domain *dom = MEDDLY::createDomainBottomUp(limits, nVars);
  MEDDLY::forest *forest = dom->createForest(false, MEDDLY::forest::REAL,
                                             MEDDLY::forest::EVPLUS);
  MEDDLY::dd_edge state1(forest);
  MEDDLY::dd_edge state2(forest);
  MEDDLY::dd_edge state_result(forest);
  forest->createEdge(assigns, results1, nAssigns, state1);
  forest->createEdge(assigns, results2, nAssigns, state2);
  MEDDLY::apply(op, state1, state2, state_result);
  for (int i = 0; i < nAssigns; i++)
  {
    // Creates the assignment vectors => we use our own evaluate function!
    int vec[nVars + 1];
    vec[0] = 0;
    for (int j = 1; j < nVars + 1; j++)
    {
      vec[j] = assigns[i][j];
    }
    // Evaluate
    float result = -1;
    forest->evaluate(state_result, vec, result);

    // Check if correct!
    if (std::fabs(eval(results1[i], results2[i]) - result) > 0.01)
    {
      MEDDLY::FILE_output meddlyout(stdout);
      state1.show(meddlyout, 3);
      state1.getEdgeValue(result);
      std::cout << "--------------------------" << std::endl;
      state2.show(meddlyout, 3);
      state2.getEdgeValue(result);
      std::cout << "--------------------------" << std::endl;
      state_result.show(meddlyout, 3);
      state_result.getEdgeValue(result);
      std::cout << "--------------------------" << std::endl;
      for (int j = 1; j < nVars + 1; j++)
      {
        std::cout << vec[j] << " ";
      } 
      std::cout << std::endl;
      std::cout << "--------------------------" << std::endl;
    }
    EXPECT_FLOAT_EQ(eval(results1[i], results2[i]), result);
  }
  delete[] limits;
  delete[] results1;
  delete[] results2;
  for (int i = 0; i < nAssigns; i++) {
    delete[] assigns[i];
  }
  delete[] assigns;
}

void make_rnd_test(float min_var, float max_var, float min_lim, float max_lim,
                   const MEDDLY::unary_opname *op, float (*eval)(float))
{
  float max = std::numeric_limits<float>::infinity();
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<int> dist(min_var, max_var);
  std::uniform_int_distribution<int> dist2(min_lim, max_lim);
  int nVars = dist(mt);
  int *limits = new int[nVars];
  int nAssigns = 1;
  for (int i = 0; i < nVars; i++)
  {
    limits[i] = dist2(mt);
    nAssigns *= limits[i];
  }
  int **assigns = assignments(nVars, limits);
  float *results = new float[nAssigns];
  for (int i = 0; i < nAssigns; i++)
  {
    results[i] = static_cast<float>(dist2(mt));
    if (results[i] == max_lim)
      results[i] = max;
  }
  // Create EVMDD
  MEDDLY::domain *dom = MEDDLY::createDomainBottomUp(limits, nVars);
  MEDDLY::forest *forest = dom->createForest(false, MEDDLY::forest::REAL,
                                             MEDDLY::forest::EVPLUS);
  MEDDLY::dd_edge state(forest);
  MEDDLY::dd_edge state_result(forest);
  forest->createEdge(assigns, results, nAssigns, state);
  MEDDLY::apply(op, state, state_result);
  for (int i = 0; i < nAssigns; i++)
  {
    // Creates the assignment vectors => we use our own evaluate function!
    int vec[nVars + 1];
    vec[0] = 0;
    for (int j = 1; j < nVars + 1; j++)
    {
      vec[j] = assigns[i][j];
    }
    // Evaluate
    float result = -1;
    forest->evaluate(state_result, vec, result);

    // Check if correct!
    if (std::fabs(eval(results[i]) - result) > 0.01)
    {
      MEDDLY::FILE_output meddlyout(stdout);
      state.show(meddlyout, 3);
      std::cout << "--------------------------" << std::endl;
      state_result.show(meddlyout, 3);
      std::cout << "--------------------------" << std::endl;
      return;
    }
    EXPECT_FLOAT_EQ(eval(results[i]), result);
  }
  delete[] limits;
  delete[] results;
  for (int i = 0; i < nAssigns; i++) {
    delete[] assigns[i];
  }
  delete[] assigns;
}