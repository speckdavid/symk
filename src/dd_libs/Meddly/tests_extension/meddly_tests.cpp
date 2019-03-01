#include "../operations_extension/userOperations.h"
#include "assignments.h"
#include "randomTest.h"
#include <gtest/gtest.h>
#include <iostream>
#include <limits>
#include <meddly.h>
#include <vector>
#include <math.h>

int nRandomTest;

TEST(EVMDD_PLUSREAL, Paper_Reduction_EVMDDs)
{
  // Create results
  float max = std::numeric_limits<float>::infinity();
  int nVars = 3;
  int limits[] = {2, 2, 3};
  int **assigns = assignments(nVars, limits);
  float results1[] = {0, max, 2, max, 2, max, max, 1, 3, max, max, 2};
  float results2[] = {0, 2, max, max, 2, 4, max, max, 1, 3, max, 3};
  float results3[] = {0, 2, 2, max, 2, 4, max, 1, 1, 3, max, 2};
  // Build EVMDDs
  MEDDLY::domain *dom = MEDDLY::createDomainBottomUp(limits, nVars);
  MEDDLY::forest *forest =
      dom->createForest(false, MEDDLY::forest::REAL, MEDDLY::forest::EVPLUS);
  MEDDLY::dd_edge state1(forest);
  MEDDLY::dd_edge state2(forest);
  MEDDLY::dd_edge state3(forest);
  forest->createEdge(assigns, results1, 2 * 2 * 3, state1);
  forest->createEdge(assigns, results2, 2 * 2 * 3, state2);
  forest->createEdge(assigns, results3, 2 * 2 * 3, state3);
  // Print for debug
  // MEDDLY::FILE_output out(stdout);
  // state1.show(out, 3);
  // state2.show(out, 3);
  // state3.show(out, 3);
  // Check results
  for (int i = 0; i < 12; i++)
  {
    // Creates the assignment vectors => we use our own evaluate function!
    int vec[nVars + 1];
    for (int j = 1; j < nVars + 1; j++)
    {
      vec[j] = assigns[i][j];
    }
    // Evaluate
    float result1 = -1;
    float result2 = -1;
    float result3 = -1;
    forest->evaluate(state1, vec, result1);
    forest->evaluate(state2, vec, result2);
    forest->evaluate(state3, vec, result3);
    ASSERT_EQ(results1[i], result1);
    ASSERT_EQ(results2[i], result2);
    ASSERT_EQ(results3[i], result3);
  }

  // clear memory
  for (int i = 0; i < 2 * 2 * 3; i++)
  {
    delete assigns[i];
    assigns[i] = nullptr;
  }
  delete assigns;
  assigns = nullptr;
}

TEST(EVMDD_PLUSREAL, Paper_Reduction2_EVMDDs)
{
  // Create results
  float max = std::numeric_limits<float>::infinity();
  int nVars = 3;
  int limits[] = {2, 2, 3};
  int **assigns = assignments(nVars, limits);
  float results1[] = {0, max, 2, max, 2, max, max, 1, 3, max, max, 2};
  float results2[] = {0, 2, max, max, 2, 4, max, max, 1, 3, max, 3};
  float results3[] = {0, 2, 2, max, 2, 4, max, 1, 1, 3, max, 2};
  // Build EVMDDs
  MEDDLY::domain *dom = MEDDLY::createDomainBottomUp(limits, nVars);
  MEDDLY::forest *forest =
      dom->createForest(false, MEDDLY::forest::REAL, MEDDLY::forest::EVPLUS);
  MEDDLY::dd_edge state1(forest);
  MEDDLY::dd_edge state2(forest);
  MEDDLY::dd_edge state3(forest);
  forest->createEdge(assigns, results1, 2 * 2 * 3, state1);
  forest->createEdge(assigns, results2, 2 * 2 * 3, state2);
  forest->createEdge(assigns, results3, 2 * 2 * 3, state3);
  // Print for debug
  // MEDDLY::FILE_output out(stdout);
  // state1.show(out, 3);
  //assert(false);
  // state2.show(out, 3);
  // state3.show(out, 3);
  // Check results
  for (int i = 0; i < 12; i++)
  {
    // Creates the assignment vectors => we use our own evaluate function!
    int vec[nVars + 1];
    for (int j = 1; j < nVars + 1; j++)
    {
      vec[j] = assigns[i][j];
    }
    // Evaluate
    float result1 = -1;
    float result2 = -1;
    float result3 = -1;
    forest->evaluate(state1, vec, result1);
    forest->evaluate(state2, vec, result2);
    forest->evaluate(state3, vec, result3);
    ASSERT_EQ(results1[i], result1);
    ASSERT_EQ(results2[i], result2);
    ASSERT_EQ(results3[i], result3);
  }

  // clear memory
  for (int i = 0; i < 2 * 2 * 3; i++)
  {
    delete assigns[i];
    assigns[i] = nullptr;
  }
  delete assigns;
  assigns = nullptr;
}

TEST(EVMDD_PLUSREAL, Paper_PLUS_EVMDDs)
{
  // Create results
  float max = std::numeric_limits<float>::infinity();
  int nVars = 3;
  int limits[] = {2, 2, 3};
  int **assigns = assignments(nVars, limits);
  float results1[] = {0, max, 2, max, 2, max, max, 1, 3, max, max, 2};
  float results2[] = {0, 2, max, max, 2, 4, max, max, 1, 3, max, 3};
  // Build EVMDDs
  MEDDLY::domain *dom = MEDDLY::createDomainBottomUp(limits, nVars);
  MEDDLY::forest *forest = dom->createForest(false, MEDDLY::forest::REAL,
                                             MEDDLY::forest::EVPLUS);
  MEDDLY::dd_edge state1(forest);
  MEDDLY::dd_edge state2(forest);
  MEDDLY::dd_edge state3(forest);
  forest->createEdge(assigns, results1, 2 * 2 * 3, state1);
  forest->createEdge(assigns, results2, 2 * 2 * 3, state2);
  MEDDLY::apply(USER_OPS::EVPLUS, state1, state2, state3);
  // Print for debug
  /*MEDDLY::FILE_output out(stdout);
  std::cout << std::endl;
  state1.show(out, 3);
  std::cout << std::endl;
  state2.show(out, 3);
  std::cout << std::endl;
  state3.show(out, 3);
  std::cout << std::endl;*/
  // Check results
  for (int i = 0; i < 2 * 2 * 3; i++)
  {
    // Creates the assignment vectors => we use our own evaluate function!
    int vec[nVars + 1];
    for (int j = 1; j < nVars + 1; j++)
    {
      vec[j] = assigns[i][j];
    }
    // Evaluate
    float result = -1;
    forest->evaluate(state3, vec, result);
    ASSERT_EQ(results1[i] + results2[i], result);
  }

  // clear memory
  for (int i = 0; i < 2 * 2 * 3; i++)
  {
    delete assigns[i];
    assigns[i] = nullptr;
  }
  delete assigns;
  assigns = nullptr;
}

TEST(EVMDD_PLUSREAL, Cach_PLUS_EVMDDs)
{
  // Create results
  float max = std::numeric_limits<float>::infinity();
  int nVars = 4;
  int limits[] = {2, 2, 2, 2};
  int **assigns = assignments(nVars, limits);
  // Build EVMDDs
  MEDDLY::domain *dom = MEDDLY::createDomainBottomUp(limits, nVars);
  MEDDLY::forest *forest =
      dom->createForest(false, MEDDLY::forest::REAL, MEDDLY::forest::EVPLUS);
  MEDDLY::dd_edge state1(forest);
  MEDDLY::dd_edge state2(forest);
  MEDDLY::dd_edge state3(forest);
  float values2[] = {0, 2};
  float values3[] = {0, 4};
  float values4[] = {0, 8};
  forest->createEdgeForVar(1, false, state1);
  forest->createEdgeForVar(2, false, values2, state2);
  MEDDLY::apply(USER_OPS::EVPLUS, state1, state2, state1);
  forest->createEdgeForVar(3, false, values3, state2);
  forest->createEdgeForVar(4, false, values4, state3);
  MEDDLY::apply(USER_OPS::EVPLUS, state2, state3, state2);
  // forest->createEdge(assigns, results2, 2 * 2 * 2 * 2, state2);
  MEDDLY::apply(USER_OPS::EVPLUS, state1, state2, state3);
  // Print for debug
  /*MEDDLY::FILE_output out(stdout);
  std::cout << std::endl;
  state1.show(out, 3);
  std::cout << std::endl;
  state2.show(out, 3);
  std::cout << std::endl;
  state3.show(out, 3);
  std::cout << std::endl;*/
  // Check results
  for (int i = 0; i < 2 * 2 * 2 * 2; i++)
  {
    // Creates the assignment vectors => we use our own evaluate function!
    int vec[nVars + 1];
    for (int j = 1; j < nVars + 1; j++)
    {
      vec[j] = assigns[i][j];
    }
    // Evaluate
    float result = -1;
    forest->evaluate(state3, vec, result);
    ASSERT_EQ(i, result);
  }

  // clear memory
  for (int i = 0; i < 2 * 2 * 3; i++)
  {
    delete assigns[i];
    assigns[i] = nullptr;
  }
  delete assigns;
  assigns = nullptr;
}

TEST(EVMDD_PLUSREAL, Random_PLUS_EVMDDs)
{
  for (int i = 0; i < nRandomTest; i++)
  {
    make_rnd_test(0, 6, 2, 8, USER_OPS::EVPLUS, &plusFloat);
  }
}

TEST(EVMDD_PLUSREAL, PaperMod_MINUS_EVMDDs)
{
  // Create results
  float max = std::numeric_limits<float>::infinity();
  int nVars = 3;
  int limits[] = {2, 2, 3};
  int **assigns = assignments(nVars, limits);
  float results1[] = {0, max, 2, max, 2, max, max, 1, 3, max, max, 2};
  float results2[] = {0, 2, 3, 4, 2, 4, 1, 2, 1, 3, 3, 3};
  // Build EVMDDs
  MEDDLY::domain *dom = MEDDLY::createDomainBottomUp(limits, nVars);
  MEDDLY::forest *forest = dom->createForest(false, MEDDLY::forest::REAL,
                                             MEDDLY::forest::EVPLUS);
  MEDDLY::dd_edge state1(forest);
  MEDDLY::dd_edge state2(forest);
  MEDDLY::dd_edge state3(forest);
  forest->createEdge(assigns, results1, 2 * 2 * 3, state1);
  forest->createEdge(assigns, results2, 2 * 2 * 3, state2);
  MEDDLY::apply(USER_OPS::MINUS, state1, state2, state3);
  // Print for debug
  /*MEDDLY::FILE_output out(stdout);
  std::cout << std::endl;
  state1.show(out, 3);
  std::cout << std::endl;
  state2.show(out, 3);
  std::cout << std::endl;
  state3.show(out, 3);
  std::cout << std::endl;*/
  // Check results
  for (int i = 0; i < 2 * 2 * 3; i++)
  {
    // Creates the assignment vectors => we use our own evaluate function!
    int vec[nVars + 1];
    for (int j = 1; j < nVars + 1; j++)
    {
      vec[j] = assigns[i][j];
    }
    // Evaluate
    float result = -1;
    forest->evaluate(state3, vec, result);
    ASSERT_EQ(minusFloat(results1[i], results2[i]), result);
  }

  // clear memory
  for (int i = 0; i < 2 * 2 * 3; i++)
  {
    delete assigns[i];
    assigns[i] = nullptr;
  }
  delete assigns;
  assigns = nullptr;
}

TEST(EVMDD_PLUSREAL, Random_MINUS_EVMDDs)
{
  for (int i = 0; i < nRandomTest; i++)
  {
    make_rnd_test(0, 6, 2, 8, USER_OPS::MINUS, &minusFloat, false);
  }
}

TEST(EVMDD_PLUSREAL, PaperMod_MULTIPLY_EVMDDs)
{
  // Create results
  float max = std::numeric_limits<float>::infinity();
  int nVars = 3;
  int limits[] = {2, 2, 3};
  int **assigns = assignments(nVars, limits);
  float results1[] = {0, max, 2, max, 2, max, max, 1, 3, max, max, 2};
  float results2[] = {0, 2, 3, 4, 2, 4, 1, 2, 1, 3, 3, 3};
  // Build EVMDDs
  MEDDLY::domain *dom = MEDDLY::createDomainBottomUp(limits, nVars);
  MEDDLY::forest *forest = dom->createForest(false, MEDDLY::forest::REAL,
                                             MEDDLY::forest::EVPLUS);
  MEDDLY::dd_edge state1(forest);
  MEDDLY::dd_edge state2(forest);
  MEDDLY::dd_edge state3(forest);
  forest->createEdge(assigns, results1, 2 * 2 * 3, state1);
  forest->createEdge(assigns, results2, 2 * 2 * 3, state2);
  MEDDLY::apply(USER_OPS::MULTIPLY, state1, state2, state3);
  // Print for debug
  /*MEDDLY::FILE_output out(stdout);
  std::cout << std::endl;
  state1.show(out, 3);
  std::cout << std::endl;
  state2.show(out, 3);
  std::cout << std::endl;
  state3.show(out, 3);
  std::cout << std::endl;*/
  // Check results
  for (int i = 0; i < 2 * 2 * 3; i++)
  {
    // Creates the assignment vectors => we use our own evaluate function!
    int vec[nVars + 1];
    for (int j = 1; j < nVars + 1; j++)
    {
      vec[j] = assigns[i][j];
    }
    // Evaluate
    float result = -1;
    forest->evaluate(state3, vec, result);
    ASSERT_EQ(multiplyFloat(results1[i], results2[i]), result);
  }

  // clear memory
  for (int i = 0; i < 2 * 2 * 3; i++)
  {
    delete assigns[i];
    assigns[i] = nullptr;
  }
  delete assigns;
  assigns = nullptr;
}

TEST(EVMDD_PLUSREAL, Random_MULTIPLY_EVMDDs)
{
  for (int i = 0; i < nRandomTest; i++)
  {
    make_rnd_test(0, 6, 2, 8, USER_OPS::MULTIPLY, &multiplyFloat);
  }
}

TEST(EVMDD_PLUSREAL, DIVIDE_EVMDDs)
{
    // Create results
  int nVars = 2;
  int limits[] = {2, 3};
  int **assigns = assignments(nVars, limits);
  float results1[] = {6, 5, 6, 6, 2, 2};
  float results2[] = {3, 3, 6, 6, 3, 6};
  // Build EVMDDs
  MEDDLY::domain *dom = MEDDLY::createDomainBottomUp(limits, nVars);
  MEDDLY::forest *forest =
      dom->createForest(false, MEDDLY::forest::REAL, MEDDLY::forest::EVPLUS);
  MEDDLY::dd_edge state1(forest);
  MEDDLY::dd_edge state2(forest);
  MEDDLY::dd_edge state3(forest);
  forest->createEdge(assigns, results1, 2 * 3, state1);
  forest->createEdge(assigns, results2, 2 * 3, state2);
  MEDDLY::apply(USER_OPS::DIVIDE, state1, state2, state3);
  // Print for debug
  /*MEDDLY::FILE_output out(stdout);
  std::cout << std::endl;
  state1.show(out, 3);
  std::cout << std::endl;
  state2.show(out, 3);
  std::cout << std::endl;
  state3.show(out, 3);
  std::cout << std::endl;*/
  for (int i = 0; i < 2 * 3; i++)
  {
    // Creates the assignment vectors => we use our own evaluate function!
    int vec[nVars + 1];
    for (int j = 1; j < nVars + 1; j++)
    {
      vec[j] = assigns[i][j];
    }
    // Evaluate
    float result = -1;
    forest->evaluate(state3, vec, result);
    ASSERT_FLOAT_EQ(divideFloat(results1[i], results2[i]), result);
  }
}

TEST(EVMDD_PLUSREAL, Random_DIVIDE_EVMDDs)
{
  for (int i = 0; i < nRandomTest; i++)
  {
    make_rnd_test(0, 8, 2, 6, USER_OPS::DIVIDE, &divideFloat, false, true);
  }
}

TEST(EVMDD_PLUSREAL, Paper_UNIONMIN_EVMDDs)
{
  // Create results
  float max = std::numeric_limits<float>::infinity();
  int nVars = 3;
  int limits[] = {2, 2, 3};
  int **assigns = assignments(nVars, limits);
  float results1[] = {0, max, 2, max, 2, max, max, 1, 3, max, max, 2};
  float results2[] = {0, 2, max, max, 2, 4, max, max, 1, 3, max, 3};
  // Build EVMDDs
  MEDDLY::domain *dom = MEDDLY::createDomainBottomUp(limits, nVars);
  MEDDLY::forest *forest =
      dom->createForest(false, MEDDLY::forest::REAL, MEDDLY::forest::EVPLUS);
  MEDDLY::dd_edge state1(forest);
  MEDDLY::dd_edge state2(forest);
  MEDDLY::dd_edge state3(forest);
  forest->createEdge(assigns, results1, 2 * 2 * 3, state1);
  forest->createEdge(assigns, results2, 2 * 2 * 3, state2);
  MEDDLY::apply(USER_OPS::UNIONMIN, state1, state2, state3);
  // Print for debug
  /*MEDDLY::FILE_output out(stdout);
  std::cout << std::endl;
  state1.show(out, 3);
  std::cout << std::endl;
  state2.show(out, 3);
  std::cout << std::endl;
  state3.show(out, 3);
  std::cout << std::endl;*/
  // Check results
  for (int i = 0; i < 2 * 2 * 3; i++)
  {
    // Creates the assignment vectors => we use our own evaluate function!
    int vec[nVars + 1];
    for (int j = 1; j < nVars + 1; j++)
    {
      vec[j] = assigns[i][j];
    }
    // Evaluate
    float result = -1;
    forest->evaluate(state3, vec, result);
    ASSERT_EQ(std::min(results1[i], results2[i]), result);
  }

  // clear memory
  for (int i = 0; i < 2 * 2 * 3; i++)
  {
    delete assigns[i];
    assigns[i] = nullptr;
  }
  delete assigns;
  assigns = nullptr;
}

TEST(EVMDD_PLUSREAL, Random_UNIONMIN_EVMDDs)
{
  for (int i = 0; i < nRandomTest; i++)
  {
    make_rnd_test(0, 6, 2, 8, USER_OPS::UNIONMIN, &minFloat);
  }
}

TEST(EVMDD_PLUSREAL, Paper_INTESECTIONMAX_EVMDDs)
{
  // Create results
  float max = std::numeric_limits<float>::infinity();
  int nVars = 3;
  int limits[] = {2, 2, 3};
  int **assigns = assignments(nVars, limits);
  float results1[] = {0, max, 2, max, 2, max, max, 1, 3, max, max, 2};
  float results2[] = {0, 2, max, max, 2, 4, max, max, 1, 3, max, 3};
  // Build EVMDDs
  MEDDLY::domain *dom = MEDDLY::createDomainBottomUp(limits, nVars);
  MEDDLY::forest *forest =
      dom->createForest(false, MEDDLY::forest::REAL, MEDDLY::forest::EVPLUS);
  MEDDLY::dd_edge state1(forest);
  MEDDLY::dd_edge state2(forest);
  MEDDLY::dd_edge state3(forest);
  forest->createEdge(assigns, results1, 2 * 2 * 3, state1);
  forest->createEdge(assigns, results2, 2 * 2 * 3, state2);
  MEDDLY::apply(USER_OPS::INTERSECTIONMAX, state1, state2, state3);
  // Print for debug
  /*MEDDLY::FILE_output out(stdout);
  std::cout << std::endl;
  state1.show(out, 3);
  std::cout << std::endl;
  state2.show(out, 3);
  std::cout << std::endl;
  state3.show(out, 3);
  std::cout << std::endl;*/
  // Check results
  for (int i = 0; i < 2 * 2 * 3; i++)
  {
    // Creates the assignment vectors => we use our own evaluate function!
    int vec[nVars + 1];
    for (int j = 1; j < nVars + 1; j++)
    {
      vec[j] = assigns[i][j];
    }
    // Evaluate
    float result = -1;
    forest->evaluate(state3, vec, result);
    ASSERT_EQ(std::max(results1[i], results2[i]), result);
  }

  // clear memory
  for (int i = 0; i < 2 * 2 * 3; i++)
  {
    delete assigns[i];
    assigns[i] = nullptr;
  }
  delete assigns;
  assigns = nullptr;
}

TEST(EVMDD_PLUSREAL, Random_INTERSECTIONMAX_EVMDDs)
{
  for (int i = 0; i < nRandomTest; i++)
  {
    make_rnd_test(0, 6, 2, 8, USER_OPS::INTERSECTIONMAX, &maxFloat);
  }
}

TEST(EVMDD_PLUSREAL, Random_GREATERTHAN_EVMDDs)
{
  for (int i = 0; i < nRandomTest; i++)
  {
    make_rnd_test(0, 6, 2, 8, USER_OPS::GREATERTHAN, &greaterThanFloat);
  }
}

TEST(EVMDD_PLUSREAL, Random_GREATEREQUALS_EVMDDs)
{
  for (int i = 0; i < nRandomTest; i++)
  {
    make_rnd_test(0, 6, 2, 8, USER_OPS::GREATEREQUALS, &greaterEqualsFloat);
  }
}

TEST(EVMDD_PLUSREAL, Random_LESSTHAN_EVMDDs)
{
  for (int i = 0; i < nRandomTest; i++)
  {
    make_rnd_test(0, 6, 2, 8, USER_OPS::LESSTHAN, &lessThanFloat);
  }
}

TEST(EVMDD_PLUSREAL, Random_LESSEQUALS_EVMDDs)
{
  for (int i = 0; i < nRandomTest; i++)
  {
    make_rnd_test(0, 6, 2, 8, USER_OPS::LESSEQUALS, &lessEqualsFloat);
  }
}

TEST(EVMDD_PLUSREAL, Random_EQUALS_EVMDDs)
{
  for (int i = 0; i < nRandomTest; i++)
  {
    make_rnd_test(0, 6, 2, 8, USER_OPS::EQUALS, &equalsFloat);
  }
}

TEST(EVMDD_PLUSREAL, Paper_POW_EVMDDs)
{
  float max = std::numeric_limits<float>::infinity();
  int nVars = 3;
  int limits[] = {2, 2, 3};
  int **assigns = assignments(nVars, limits);
  float results1[] = {0, max, 2, max, 2, max, max, 1, 3, max, max, 2};
  float results2[] = {0, 2, max, max, 2, 4, max, max, 1, 3, max, 3};
  MEDDLY::domain *dom = MEDDLY::createDomainBottomUp(limits, nVars);
  MEDDLY::forest *forest =
      dom->createForest(false, MEDDLY::forest::REAL, MEDDLY::forest::EVPLUS);
  MEDDLY::dd_edge state1(forest);
  MEDDLY::dd_edge state2(forest);
  MEDDLY::dd_edge pow1(forest);
  MEDDLY::dd_edge pow2(forest);
  forest->createEdge(assigns, results1, 12, state1);
  forest->createEdge(assigns, results2, 12, state2);


  MEDDLY::apply(USER_OPS::POW, state1, pow1);
  MEDDLY::apply(USER_OPS::POW, state2, pow2);
  for (int i = 0; i < 12; i++)
  {
    int vec[nVars + 1];
    for (int j = 1; j < nVars + 1; j++)
    {
      vec[j] = assigns[i][j];
    }
    // Evaluate
    float result1 = -1;
    float result2 = -2;
    forest->evaluate(pow1, vec, result1);
    forest->evaluate(pow2, vec, result2);
    ASSERT_EQ(std::pow(results1[i],2), result1);
    ASSERT_EQ(std::pow(results2[i],2), result2);
  }

  // clear memory
  for (int i = 0; i < 2 * 2 * 3; i++)
  {
    delete assigns[i];
    assigns[i] = nullptr;
  }
  delete assigns;
  assigns = nullptr;
}

TEST(EVMDD_PLUSREAL, Mini_POW_EVMDDs)
{
  float max = std::numeric_limits<float>::infinity();
  int nVars = 1;
  int limits[] = {2};
  int **assigns = assignments(nVars, limits);
  float results1[] = {0, max};
  float results2[] = {1, 5};
  MEDDLY::domain *dom = MEDDLY::createDomainBottomUp(limits, nVars);
  MEDDLY::forest *forest =
      dom->createForest(false, MEDDLY::forest::REAL, MEDDLY::forest::EVPLUS);
  MEDDLY::dd_edge state1(forest);
  MEDDLY::dd_edge state2(forest);
  MEDDLY::dd_edge pow1(forest);
  MEDDLY::dd_edge pow2(forest);
  forest->createEdge(assigns, results1, 2, state1);
  forest->createEdge(assigns, results2, 2, state2);

  MEDDLY::apply(USER_OPS::POW, state1, pow1);
  MEDDLY::apply(USER_OPS::POW, state2, pow2);
  for (int i = 0; i < 2; i++)
  {
    int vec[nVars + 1];
    for (int j = 1; j < nVars + 1; j++)
    {
      vec[j] = assigns[i][j];
    }
    // Evaluate
    float result1 = -1;
    float result2 = -2;
    forest->evaluate(pow1, vec, result1);
    forest->evaluate(pow2, vec, result2);
    ASSERT_EQ(std::pow(results1[i],2), result1);
    ASSERT_EQ(std::pow(results2[i],2), result2);
  }

  // clear memory
  for (int i = 0; i < 2; i++)
  {
    delete assigns[i];
    assigns[i] = nullptr;
  }
  delete assigns;
  assigns = nullptr;
}

TEST(EVMDD_PLUSREAL, Random_POW_EVMDDs)
{
  for (int i = 0; i < nRandomTest; i++)
  {
    make_rnd_test(0, 6, 2, 8, USER_OPS::POW, &pow2Float);
  }
}

TEST(EVMDD_PLUSREAL, Paper_PARTIALCOMPLEMENT_EVMDDs)
{
  float max = std::numeric_limits<float>::infinity();
  int nVars = 3;
  int limits[] = {2, 2, 3};
  int **assigns = assignments(nVars, limits);
  float results1[] = {0, max, 2, max, 2, max, max, 1, 3, max, max, 2};
  float results2[] = {0, 2, max, max, 2, 4, max, max, 1, 3, max, 3};
  MEDDLY::domain *dom = MEDDLY::createDomainBottomUp(limits, nVars);
  MEDDLY::forest *forest =
      dom->createForest(false, MEDDLY::forest::REAL, MEDDLY::forest::EVPLUS);
  MEDDLY::dd_edge state1(forest);
  MEDDLY::dd_edge state2(forest);
  MEDDLY::dd_edge comp1(forest);
  MEDDLY::dd_edge comp2(forest);
  forest->createEdge(assigns, results1, 12, state1);
  forest->createEdge(assigns, results2, 12, state2);

  // TEST the results
  // float results1_after[] = { max, 0, max, 0, max, 0, 0, max, max, 0, 0, max
  // }; float results2_after[] = { max, max, 0, 0, max, max, 0, 0, max, max, 0,
  // max };
  MEDDLY::apply(USER_OPS::PARTIALCOMPLEMENT, state1, comp1);
  MEDDLY::apply(USER_OPS::PARTIALCOMPLEMENT, state2, comp2);
  /*MEDDLY::FILE_output out(stdout);
   state2.show(out, 3);
   std::cout << "\n\n" << std::endl;
   comp2.show(out, 3);*/
  for (int i = 0; i < 12; i++)
  {
    int vec[nVars + 1];
    for (int j = 1; j < nVars + 1; j++)
    {
      vec[j] = assigns[i][j];
    }
    // Evaluate
    float result1 = -1;
    float result2 = -2;
    forest->evaluate(comp1, vec, result1);
    forest->evaluate(comp2, vec, result2);
    ASSERT_EQ(cmplFloat(results1[i]), result1);
    ASSERT_EQ(cmplFloat(results2[i]), result2);
  }

  // clear memory
  for (int i = 0; i < 2 * 2 * 3; i++)
  {
    delete assigns[i];
    assigns[i] = nullptr;
  }
  delete assigns;
  assigns = nullptr;
}

TEST(EVMDD_PLUSREAL, Random_PARTIALCOMPLEMENT_EVMDDs)
{
  for (int i = 0; i < nRandomTest; i++)
  {
    make_rnd_test(0, 6, 2, 8, USER_OPS::PARTIALCOMPLEMENT, &cmplFloat);
  }
}

TEST(EVMDD_PLUSREAL, Paper_RESTRICT_EVMDDs) {
  int nVars = 3;
  int limits[] = { 2, 2, 3 };
  int** assigns = assignments(nVars, limits);
  float results1[] = { 0, INFTY, 2, INFTY, 2, INFTY, INFTY, 1, 3, INFTY, INFTY, 2 };
  float results2[] = { 0, 2, INFTY, INFTY, 2, 4, INFTY, INFTY, 1, 3, INFTY, 3 };
  MEDDLY::domain* dom = MEDDLY::createDomainBottomUp(limits, nVars);
  MEDDLY::forest* forest = dom->createForest(false, MEDDLY::forest::REAL,
  MEDDLY::forest::EVPLUS);
  MEDDLY::dd_edge state1(forest);
  MEDDLY::dd_edge state2(forest);
  MEDDLY::dd_edge state_results1(forest);
  MEDDLY::dd_edge state_results2(forest);
  forest->createEdge(assigns, results1, 12, state1);
  forest->createEdge(assigns, results2, 12, state2);

  // create results stat 1
  float s1ev00[] = { 0, 0, 2, 2, 2, 2, INFTY, INFTY, 3, 3, INFTY, INFTY };
  float s1ev01[] = { INFTY, INFTY, INFTY, INFTY, INFTY, INFTY, 1, 1, INFTY, INFTY, 2, 2 };
  float s1ev10[] = { 0, INFTY, 0, INFTY, 2, INFTY, 2, INFTY, 3, INFTY, 3, INFTY };
  float s1ev11[] = { 2, INFTY, 2, INFTY, INFTY, 1, INFTY, 1, INFTY, 2, INFTY, 2 };
  float s1ev20[] = { 0, INFTY, 2, INFTY, 0, INFTY, 2, INFTY, 0, INFTY, 2, INFTY };
  float s1ev21[] = { 2, INFTY, INFTY, 1, 2, INFTY, INFTY, 1, 2, INFTY, INFTY, 1 };
  float s1ev22[] = { 3, INFTY, INFTY, 2, 3, INFTY, INFTY, 2, 3, INFTY, INFTY, 2 };
  float* s1res[] = { s1ev00, s1ev01, s1ev10, s1ev11, s1ev20, s1ev21, s1ev22 };

  // create results stat 2
  float s2ev00[] = { 0, 0, INFTY, INFTY, 2, 2, INFTY, INFTY, 1, 1, INFTY, INFTY };
  float s2ev01[] = { 2, 2, INFTY, INFTY, 4, 4, INFTY, INFTY, 3, 3, 3, 3 };
  float s2ev10[] = { 0, 2, 0, 2, 2, 4, 2, 4, 1, 3, 1, 3 };
  float s2ev11[] = { INFTY, INFTY, INFTY, INFTY, INFTY, INFTY, INFTY, INFTY, INFTY, 3, INFTY, 3 };
  float s2ev20[] = { 0, 2, INFTY, INFTY, 0, 2, INFTY, INFTY, 0, 2, INFTY, INFTY };
  float s2ev21[] = { 2, 4, INFTY, INFTY, 2, 4, INFTY, INFTY, 2, 4, INFTY, INFTY };
  float s2ev22[] = { 1, 3, INFTY, 3, 1, 3, INFTY, 3, 1, 3, INFTY, 3 };
  float* s2res[] = { s2ev00, s2ev01, s2ev10, s2ev11, s2ev20, s2ev21, s2ev22 };

  int cur_res = 0;
  float result;
  for (int var = 1; var <= 3; var++) {
    for (int val = 0; val < limits[var - 1]; val++) {
      USER_OPS::setRestrictVarVal(state1, state_results1, var, val);
      USER_OPS::setRestrictVarVal(state2, state_results2, var, val);
      MEDDLY::apply(USER_OPS::RESTRICT, state1, state_results1);
      MEDDLY::apply(USER_OPS::RESTRICT, state2, state_results2);
      for (int i = 0; i < 12; i++) {
        // Creates the assignment vectors => we use our own evaluate function!
        int vec[nVars + 1];
        for (int j = 1; j < nVars + 1; j++) {
          vec[j] = assigns[i][j];
        }

        forest->evaluate(state_results1, vec, result);
        ASSERT_EQ(s1res[cur_res][i], result);
        forest->evaluate(state_results2, vec, result);
        ASSERT_EQ(s2res[cur_res][i], result);
      }
      cur_res++;
    }
  }
  // clear memory
  for (int i = 0; i < 2 * 2 * 3; i++) {
    delete assigns[i];
    assigns[i] = nullptr;
  }
  delete assigns;
  assigns = nullptr;
}

TEST(EVMDD_PLUSREAL, SWAPVAR_EVMDDs)
{
  // Create results
  float max = std::numeric_limits<float>::infinity();
  int nVars = 4;
  int limits[] = {6, 2, 2, 6};
  int **assigns = assignments(nVars, limits);
  // Build EVMDDs
  MEDDLY::domain *dom = MEDDLY::createDomainBottomUp(limits, nVars);
  MEDDLY::forest *forest =
      dom->createForest(false, MEDDLY::forest::REAL, MEDDLY::forest::EVPLUS);
  MEDDLY::dd_edge state1(forest);
  MEDDLY::dd_edge state2(forest);
  MEDDLY::dd_edge state3(forest);
  MEDDLY::dd_edge state4(forest);
  float values3[] = {0, 2, 4, 8, 16, 32};
  forest->createEdgeForVar(1, false, state1);
  forest->createEdgeForVar(2, false, state2);
  forest->createEdgeForVar(3, false, state3);
  forest->createEdgeForVar(4, false, values3, state4);

  MEDDLY::dd_edge dummy(forest);
  MEDDLY::apply(USER_OPS::SWAPVAR, state1, dummy);
  MEDDLY::dd_edge swap_1_4(forest); // 0 is terminal 
  USER_OPS::setSwapVar(state1, swap_1_4, 1, 4);
  MEDDLY::apply(USER_OPS::SWAPVAR, state1, swap_1_4);
  MEDDLY::dd_edge swap_4_1(forest);
  USER_OPS::setSwapVar(state4, swap_4_1, 4, 1);
  MEDDLY::apply(USER_OPS::SWAPVAR, state4, swap_4_1);
  // Print for debug
  /*MEDDLY::FILE_output out(stdout);
  std::cout << std::endl;
  state3.show(out, 3);
  std::cout << std::endl;
  swap_4_1.show(out, 3);
  std::cout << std::endl;*/
  // Check results
  for (int i = 0; i < 6 * 2 * 2 * 6; i++)
  {
    // Creates the assignment vectors => we use our own evaluate function!
    int vec[nVars + 1];
    int rev_vec[nVars + 1];
    for (int j = 1; j < nVars + 1; j++)
    {
      vec[j] = assigns[i][j];
      rev_vec[nVars + 1 - j] = assigns[i][j];
    }

    // Evaluate
    float prev_result = -1;
    float result = -1;
    forest->evaluate(state1, vec, prev_result);
    forest->evaluate(dummy, vec, result);
    ASSERT_EQ(prev_result, result);
    forest->evaluate(state1, vec, prev_result);
    forest->evaluate(swap_1_4, rev_vec, result);
    ASSERT_EQ(prev_result, result);
    forest->evaluate(state4, vec, prev_result);  
    forest->evaluate(swap_4_1, rev_vec, result);
    ASSERT_EQ(prev_result, result);
  }

  // clear memory
  for (int i = 0; i < 2 * 2 * 3; i++)
  {
    delete assigns[i];
    assigns[i] = nullptr;
  }
  delete assigns;
  assigns = nullptr;
}

int main(int argc, char **argv)
{
  // Some input parsing...
  std::string usage = "Usage: ./runTests <#RandomTest>";
  if (argc < 1 || argc > 2)
  {
    std::cout << usage << std::endl;
    exit(0);
  }
  if (argc == 2 &&
      (std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help"))
  {
    std::cout << usage << std::endl;
    exit(0);
  }
  if (argc == 2)
  {
    nRandomTest = std::stoi(argv[1]);
  }
  else
  {
    nRandomTest = 100;
  }
  // Init Meddly + User Ops
  USER_OPS::initializeUserOperations();
  MEDDLY::initialize();

  // Google Test Framework
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
