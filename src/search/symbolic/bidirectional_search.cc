#include "bidirectional_search.h"

#include "debug_macros.h"
#include <algorithm> // std::reverse
#include <memory>
#include "sym_controller.h"

using namespace std;
using utils::g_timer;

namespace symbolic
{

BidirectionalSearch::BidirectionalSearch(SymController *eng,
                                         const SymParamsSearch &params,
                                         std::unique_ptr<UnidirectionalSearch> _fw,
                                         unique_ptr<UnidirectionalSearch> _bw) : SymSearch(eng, params),
                                                                                 fw(std::move(_fw)),
                                                                                 bw(std::move(_bw))
{
    assert(fw->getStateSpace() == bw->getStateSpace());
    mgr = fw->getStateSpaceShared();
}

UnidirectionalSearch *BidirectionalSearch::selectBestDirection() const
{

    bool fwSearchable = fw->isSearchable();
    bool bwSearchable = bw->isSearchable();
    if (fwSearchable && !bwSearchable)
    {
        return fw.get();
    }
    else if (!fwSearchable && bwSearchable)
    {
        return bw.get();
    }

    return fw->nextStepNodes() <= bw->nextStepNodes() ? fw.get() : bw.get();
}

bool BidirectionalSearch::finished() const
{
    return fw->finished() || bw->finished();
}

void BidirectionalSearch::statistics() const
{
    if (fw)
        fw->statistics();
    if (bw)
        bw->statistics();
    cout << endl;
}

bool BidirectionalSearch::stepImage(int maxTime, int maxNodes)
{
    bool res = selectBestDirection()->stepImage(maxTime, maxNodes);

    if (isOriginal())
    {
        engine->setLowerBound(getF());
    }

    return res;
}
} // namespace symbolic