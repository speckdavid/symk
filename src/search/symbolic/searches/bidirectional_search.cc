#include "bidirectional_search.h"

#include "../search_engines/symbolic_search.h"

#include <memory>

using namespace std;

namespace symbolic {
BidirectionalSearch::BidirectionalSearch(SymbolicSearch *eng,
                                         const SymParameters &params,
                                         shared_ptr<UniformCostSearch> _fw,
                                         shared_ptr<UniformCostSearch> _bw,
                                         bool alternating)
    : SymSearch(eng, params), fw(_fw), bw(_bw), cur_dir(nullptr), alternating(alternating) {
    assert(fw->getStateSpace() == bw->getStateSpace());
    mgr = fw->getStateSpaceShared();
}

string BidirectionalSearch::get_last_dir() const {
    return cur_dir ? cur_dir->get_last_dir() : "";
}

UniformCostSearch *BidirectionalSearch::selectBestDirection() {
    if (alternating) {
        if (!cur_dir) {
            cur_dir = fw;
        } else {
            if (cur_dir == fw) {
                cur_dir = bw;
            } else {
                cur_dir = fw;
            }
        }
    } else {
        Estimation &fw_est = *fw->get_step_estimator();
        Estimation &bw_est = *bw->get_step_estimator();
        if (fw_est.get_failed() && bw_est.get_failed()) {
            sym_params.increase_bound();
            bw_est.set_data(bw_est.get_time(), bw_est.get_nodes(), false);
            return fw.get();
        }
        /*utils::g_log << "FWD: " << fw_est << endl;
        utils::g_log << "BWD: " << bw_est << endl;
        utils::g_log << ((bw_est < fw_est) ? "bw" : "fw") << endl;*/
        cur_dir = (bw_est < fw_est) ? bw : fw;
    }

    return cur_dir.get();
}

bool BidirectionalSearch::finished() const {
    return fw->finished() || bw->finished();
}

void BidirectionalSearch::stepImage(int maxTime, int maxNodes) {
    selectBestDirection()->stepImage(maxTime, maxNodes);
    engine->setLowerBound(getF());
    engine->setMinG(fw->getG() + bw->getG());
}
}
