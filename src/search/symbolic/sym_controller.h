#ifndef SYMBOLIC_SYM_CONTROLLER_H
#define SYMBOLIC_SYM_CONTROLLER_H

//Shared class for SymEngine and smas_heuristic

#include "sym_state_space_manager.h"
#include "sym_enums.h"
#include "sym_params_search.h"
#include "sym_solution.h"

#include <vector>
#include <memory>
#include <limits>

namespace options {
class OptionParser;
class Options;
}

namespace symbolic {
class SymSolution;
class SymVariables;
class SymPH;


class SymController {
protected:
    std::shared_ptr<SymVariables> vars; //The symbolic variables are declared here

    SymParamsMgr mgrParams; //Parameters for SymStateSpaceManager configuration.
    SymParamsSearch searchParams; //Parameters to search the original state space

    int lower_bound;
    SymSolution solution; 
public:
    SymController(const options::Options &opts);
    virtual ~SymController() {}

    virtual void new_solution(const SymSolution & sol);
    void setLowerBound(int lower);

    int getUpperBound() const {
	if(solution.solved()) return solution.getCost();
	else return std::numeric_limits<int>::max();
    }
    int getLowerBound() const {
	return lower_bound; 
    }
    
    bool solved() const {
	return getLowerBound() >= getUpperBound(); 
    }
    
    const SymSolution * get_solution () const {
	return &solution;
    }

    inline SymVariables *getVars() {
        return vars.get();
    }

    inline const SymParamsMgr &getMgrParams() const {
        return mgrParams;
    }

    inline const SymParamsSearch &getSearchParams() const {
        return searchParams;
    }

    static void add_options_to_parser(options::OptionParser &parser,
                                      int maxStepTime, int maxStepNodes);
};
}
#endif
