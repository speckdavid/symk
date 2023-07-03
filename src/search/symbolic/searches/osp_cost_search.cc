#include "osp_cost_search.h"
#include "../closed_list.h"

namespace symbolic {
void OspCostSearch::filterFrontier() {
    frontier.filter(closed->getClosed());
    mgr->filterMutex(frontier.bucket(), fw, initialization());
    removeZero(frontier.bucket());
}
}
