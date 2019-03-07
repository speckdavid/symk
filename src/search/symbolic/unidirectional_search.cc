#include "unidirectional_search.h"
#include "closed_list.h"
#include "sym_solution.h"

using namespace std;

namespace symbolic {

    OppositeFrontierFixed::OppositeFrontierFixed(Bdd bdd, 
						 const SymStateSpaceManager & mgr) : goal (bdd), 
										     hNotGoal(mgr.getAbsoluteMinTransitionCost()) {
    }

    SymSolution OppositeFrontierFixed::checkCut(UnidirectionalSearch * search, const Bdd &states, int g, bool fw) const {
	Bdd cut = states * goal;
	if (cut.IsZero()) {
	    return SymSolution(); //No solution yet :(
	}

	if (fw) //Solution reconstruction will fail
	    return SymSolution(search, nullptr,  g, 0, cut);
	else
	    return SymSolution(nullptr, search, 0, g, cut);

    }

	std::vector<SymSolution> OppositeFrontierFixed::getAllCuts(UnidirectionalSearch *search, const Bdd &states, int g, bool fw, int /*lower_bound*/) const {
		std::vector<SymSolution> result;
		Bdd cut = states * goal;
		if (cut.IsZero()) {
			result.emplace_back(); //No solution yet :(
		} else {
			if (fw) //Solution reconstruction will fail
				result.emplace_back(search, nullptr,  g, 0, cut);
			else
				result.emplace_back(nullptr, search, 0, g, cut);
		}	
		return result;
	}

    UnidirectionalSearch::UnidirectionalSearch(SymController * eng, const SymParamsSearch &params) : 
	SymSearch(eng, params), fw(true), closed(std::make_shared<ClosedList>()) {}


    void UnidirectionalSearch::statistics() const {
	cout << "Exp " << (fw ? "fw" : "bw") << " time: " << stats.step_time << "s (img:" <<
	    stats.image_time << "s, heur: " << stats.time_heuristic_evaluation <<
	    "s) in " << stats.num_steps_succeeded << " steps ";
    }
}
