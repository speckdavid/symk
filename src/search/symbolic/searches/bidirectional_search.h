#ifndef SYMBOLIC_BIDIRECTIONAL_SEARCH_H
#define SYMBOLIC_BIDIRECTIONAL_SEARCH_H

#include "sym_search.h"
#include "uniform_cost_search.h"

namespace symbolic {
class BidirectionalSearch : public SymSearch {
private:
    std::shared_ptr<UniformCostSearch> fw, bw;
    std::shared_ptr<UniformCostSearch> cur_dir;
    bool alternating;

    // Returns the best direction to search the bd exp
    UniformCostSearch *selectBestDirection();

public:
    BidirectionalSearch(SymbolicSearch *eng, const SymParameters &params,
                        std::shared_ptr<UniformCostSearch> fw,
                        std::shared_ptr<UniformCostSearch> bw,
                        bool alternating = false);

    virtual bool finished() const override;

    virtual void step() override {
        stepImage(sym_params.max_alloted_time, sym_params.max_alloted_nodes);
    }

    virtual std::string get_last_dir() const override;

    virtual void stepImage(int maxTime, int maxNodes) override;

    virtual int getF() const override {
        return std::max<int>(std::max<int>(fw->getF(), bw->getF()),
                             fw->getG() + bw->getG() + std::min(1, mgr->get_min_transition_cost()));
    }

    bool isExpFor(BidirectionalSearch *bdExp) const;

    inline UniformCostSearch *getFw() const {return fw.get();}

    inline UniformCostSearch *getBw() const {return bw.get();}

    friend std::ostream &operator<<(std::ostream &os,
                                    const BidirectionalSearch &other);
};
}
#endif
