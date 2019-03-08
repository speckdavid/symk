#include "sym_solution.h"

#include "../state_registry.h"
#include "debug_macros.h"
#include <vector> // std::vector

#include "unidirectional_search.h"

using namespace std;

namespace symbolic {
void SymSolution::getPlan(vector<OperatorID> &path) const {
  assert(path.empty()); // This code should be modified to allow appending
                        // things to paths
  DEBUG_MSG(cout << "Extract path forward: " << g << endl;);
  if (exp_fw) {
    exp_fw->getPlan(cut, g, path);
  }
  DEBUG_MSG(cout << "Extract path backward: " << h << endl;);
  if (exp_bw) {
    Bdd newCut;
    if (!path.empty()) {
      TaskProxy task_proxy(*tasks::g_root_task);
      State s = task_proxy.get_initial_state();
      // Get state
      for (auto op_id : path) {
        OperatorProxy op = task_proxy.get_operators()[op_id];
        s = s.get_successor(op);
      }
      newCut = exp_bw->getStateSpace()->getVars()->getStateBDD(s.get_values());
    } else {
      newCut = cut;
    }

    exp_bw->getPlan(newCut, h, path);
  }
}
} // namespace symbolic