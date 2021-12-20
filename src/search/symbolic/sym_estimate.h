#ifndef SYMBOLIC_SYM_ESTIMATE_H
#define SYMBOLIC_SYM_ESTIMATE_H

#include <fstream>
#include <iostream>
#include <map>
#include <utility>

namespace symbolic {
class SymParamsSearch;
/*
 * NOTE: Though the aim of this class is to get an accurate estimation
 * of the times/nodes needed for an step, probably the overhead is not
 * worthy. Therefore, sym_estimate_linear.h is recommended.
 *
 * Class to estimate the cost of a given BDD operation depending on the size of
 * the BDD. It mantains a table that records previous experiences nodes => (cost
 * in time, cost in nodes). When it is asked about a new estimation, it
 *
 * We are assuming:
 * 1) estimations for larger BDDs are greater (we have to keep the consistency)
 * 2) If a new estimation that is not currently on our database is asked then:
 *    a) if the new value is larger than any in our data: linear interpolation
 * with the largest one. b) if the new value is smaller than any in our data:
 *            just interpolate the estimations of the previous and next value.
 *
 * 3) When we relax a search we will only keep the current estimation
 * (the image is performed with other TRs, so all the other data is not relevant
 * for us),
 */
class Estimation {
public:
    double time;
    double nodes;
    Estimation(double t = 1, double n = 1) : time(t), nodes(n) {}

    friend std::ostream &operator<<(std::ostream &os, const Estimation &est);
};

class SymStepCostEstimation {
    // Parameters for the estimation
    double param_min_estimation_time;
    double param_penalty_time_estimation_sum, param_penalty_time_estimation_mult;
    double param_penalty_nodes_estimation_sum,
           param_penalty_nodes_estimation_mult;

    long nextStepNodes;            // Nodes of the step to be estimated
    Estimation estimation;         // Current estimation of next step
    std::map<long, Estimation> data; // Data about time estimations (time, nodes)

    void update_data(long key, Estimation value);

public:
    SymStepCostEstimation(const SymParamsSearch &p);
    ~SymStepCostEstimation() {}

    void stepTaken(
        double time,
        double nodes); // Called after any step, telling how much time was spent
    void nextStep(double nodes); // Called before any step, telling number of
                                 // nodes to expand

    // Recompute the estimation if it has been exceeded
    void violated(double time_ellapsed, double time_limit, double node_limit);

    void recalculate(const SymStepCostEstimation &o, long nodes);

    inline long time() const {return estimation.time;}

    inline long nodes() const {return estimation.nodes;}

    inline long nextNodes() const {return nextStepNodes;}

    inline void violated_nodes(long nodes) {violated(0, 1, nodes);}

    friend std::ostream &operator<<(std::ostream &os,
                                    const SymStepCostEstimation &est);
    void write(std::ofstream &file) const;
    void read(std::ifstream &file);
};
}
#endif
