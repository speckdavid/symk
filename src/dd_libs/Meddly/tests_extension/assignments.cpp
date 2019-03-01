#include <iostream>
#include <limits>
#include "assignments.h"

int** assignments(int nVars, int* limits) {
  int nAssigns = 1;
  for (int i = 0; i < nVars; i++) {
    nAssigns *= limits[i];
  }
  int** assigns = new int*[nAssigns];
  for (int i = 0; i < nAssigns; i++) {
    assigns[i] = new int[nVars + 1];
    assigns[i][0] = 0;
  }
  for (int v = 0; v < nVars; v++) {
    int rythm = 1;
    int cur_val = 0;
    for (int i = 0; i < v; i++) {
      rythm *= limits[i];
    }
    for (int i = 1; i <= nAssigns; i++) {
      assigns[i - 1][v + 1] = cur_val % limits[v];
      if (i % rythm == 0)
        cur_val++;
    }
  }
  return assigns;
}
