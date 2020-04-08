#include "bidirectional_search.h"

#include "../search_engines/symbolic_search.h"
#include <memory>

using namespace std;
using utils::g_timer;

namespace symbolic {

BidirectionalSearch::BidirectionalSearch(SymbolicSearch *eng,
                                         const SymParamsSearch &params,
                                         std::unique_ptr<UniformCostSearch> _fw,
                                         unique_ptr<UniformCostSearch> _bw)
    : SymSearch(eng, params), fw(std::move(_fw)), bw(std::move(_bw)) {

  assert(fw->getStateSpace() == bw->getStateSpace());
  mgr = fw->getStateSpaceShared();
}

UniformCostSearch *BidirectionalSearch::selectBestDirection() const {

  bool fwSearchable = fw->isSearchable();
  bool bwSearchable = bw->isSearchable();
  if (fwSearchable && !bwSearchable) {
    return fw.get();
  } else if (!fwSearchable && bwSearchable) {
    return bw.get();
  }
  // std::cout << (fw->nextStepNodes() <= bw->nextStepNodes() ? "fwd " : "bwd ")
  // << std::flush;
  return fw->nextStepNodes() <= bw->nextStepNodes() ? fw.get() : bw.get();
}

bool BidirectionalSearch::finished() const {
  return fw->finished() || bw->finished();
}

bool BidirectionalSearch::stepImage(int maxTime, int maxNodes) {
  bool res = selectBestDirection()->stepImage(maxTime, maxNodes);
  engine->setLowerBound(getF());
  engine->setMinG(fw->getG() + bw->getG());

  return res;
}
} // namespace symbolic
