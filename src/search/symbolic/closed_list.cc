#include "closed_list.h"

#include "sym_state_space_manager.h"
#include "sym_solution.h"
#include "sym_utils.h"

#include "../utils/timer.h"
#include "debug_macros.h"

#include <sstream>
#include <iostream>
#include <fstream>
#include <string>

#include <cassert>

using namespace std;

namespace symbolic {

    ClosedList::ClosedList() : mgr(nullptr) {
    }

    void ClosedList::init(SymStateSpaceManager *manager, UnidirectionalSearch * search) {
	mgr = manager;
	my_search = search;
	set<int>().swap(h_values);
	map<int, Bdd>().swap(closedUpTo);
	map <int, vector<Bdd>>().swap(zeroCostClosed);
	map<int, Bdd>().swap(closed);
	closedTotal = mgr->zeroBDD();
	hNotClosed = 0;
	fNotClosed = 0;
    }


    void ClosedList::init(SymStateSpaceManager *manager, UnidirectionalSearch * search, const ClosedList &other) {
	mgr = manager;
	my_search = search;
	set<int>().swap(h_values);
	map<int, Bdd>().swap(closedUpTo);
	map <int, vector<Bdd>>().swap(zeroCostClosed);
	map<int, Bdd>().swap(closed);
	closedTotal = mgr->zeroBDD();
	hNotClosed = 0;
	fNotClosed = 0;

	closedTotal = other.closedTotal;
	closed[0] = closedTotal;
	newHValue(0);	
    }

    void ClosedList::newHValue(int h_value) {
	h_values.insert(h_value);
    }

    void ClosedList::insert(int h, const Bdd &S) {
	DEBUG_MSG(cout << "Inserting on closed "  << "g=" << h << ": " << S.nodeCount() << " nodes and "
		 << endl;
	    );

#ifdef DEBUG_GST
	gst_plan.checkClose(S, h, exploration);
#endif

	if (closed.count(h)) {
	    assert(h_values.count(h));
	    closed[h] += S;
	} else {
	    closed[h] = S;
	    newHValue(h);
	}

	if (mgr->hasTransitions0()) {
	    zeroCostClosed[h].push_back(S);
	}
	closedTotal += S;

	//Introduce in closedUpTo
	auto c = closedUpTo.lower_bound(h);
	while (c != std::end(closedUpTo)) {
	    c->second += S;
	    c++;
	}
    }


    void ClosedList::setHNotClosed(int newHNotClosed) {
	if (newHNotClosed > hNotClosed) {
	    hNotClosed = newHNotClosed;
	    newHValue(newHNotClosed); //Add newHNotClosed to list of h values (and those of parents)
	}
    }

    void ClosedList::setFNotClosed(int f) {
	if (f > fNotClosed) {
	    fNotClosed = f;
	}
    }

    void ClosedList::extract_path(const Bdd &c, int h, bool fw,
                                  vector<OperatorID> &path) const
    {
        if (!mgr)
            return;
        DEBUG_MSG(cout << "Sym closed extract path h=" << h << " notClosed: " << hNotClosed << endl;
                  cout << "Closed: ";
                  for (auto &c
                       : closed)
                      cout
                  << c.first << " ";
                  cout << endl;);
        const map<int, vector<TransitionRelation>> &trs = mgr->getIndividualTRs();
        Bdd cut = c;
        size_t steps0 = 0;
        if (zeroCostClosed.count(h))
        {
            assert(trs.count(0));
            //DEBUG_MSG(cout << "Check " << steps0 << " of " << zeroCostClosed.at(h).size() << endl;);
            while (steps0 < zeroCostClosed.at(h).size() && (cut * zeroCostClosed.at(h)[steps0]).IsZero())
            {
                //DEBUG_MSG(cout << "Steps0 is not " << steps0<< " of " << zeroCostClosed.at(h).size() << endl;);
                steps0++;
            }
            //DEBUG_MSG(cout << "Steps0 of h=" << h << " is " << steps0 << endl;);
            if (steps0 < zeroCostClosed.at(h).size())
            {
                cut *= zeroCostClosed.at(h)[steps0];
            }
            else
            {
                DEBUG_MSG(cout << "cut not found with steps0. Try to find with preimage: " << trs.count(0) << endl;);
                bool foundZeroCost = false;
                for (const TransitionRelation &tr : trs.at(0))
                {
                    if (foundZeroCost)
                        break;
                    Bdd succ;
                    if (fw) {
			succ = tr.preimage(cut);
		    } else {
			succ = tr.image(cut);
		    }
		    if (succ.IsZero()) {
			continue;
		    }

		    for (size_t newSteps0 = 0; newSteps0 < steps0; newSteps0++) {
			Bdd intersection = succ * zeroCostClosed.at(h)[newSteps0];
			if (!intersection.IsZero()) {
			    steps0 = newSteps0;
			    cut = succ;
			    //DEBUG_MSG(cout << "Adding " << (*(tr.getOps().begin()))->get_name() << endl;);
			    path.push_back(*(tr.getOpsIds().begin()));
			    foundZeroCost = true;
			    break;
			}
		    }
		}
		if (!foundZeroCost) {
		    DEBUG_MSG(cout << "cut not found with steps0. steps0=0:" << endl;
			);

		    steps0 = 0;
		}
	    }
	}

	while (h > 0 || steps0 > 0) {
	    DEBUG_MSG(
		cout << "h=" << h << " and steps0=" << steps0 << endl;
		//cout << "CUT: "; cut.print(0, 1);
		);
	    if (steps0 > 0) {
		bool foundZeroCost = false;
		//Apply 0-cost operators
		if (trs.count(0)) {
		    for (const TransitionRelation &tr : trs.at(0)) {
			if (foundZeroCost)
			    break;
			Bdd succ;
			if (fw) {
			    succ = tr.preimage(cut);
			} else {
			    succ = tr.image(cut);
			}
			if (succ.IsZero()) {
			    continue;
			}

			for (size_t newSteps0 = 0; newSteps0 < steps0; newSteps0++) {
			    Bdd intersection = succ * zeroCostClosed.at(h)[newSteps0];
			    if (!intersection.IsZero()) {
				steps0 = newSteps0;
				cut = succ;
				//DEBUG_MSG(cout << "Adding " << (*(tr.getOps().begin()))->get_name() << endl;);
				path.push_back(*(tr.getOpsIds().begin()));
				foundZeroCost = true;
				break;
			    }
			}
		    }
		}

		if (!foundZeroCost) {
		    /*    DEBUG_MSG(
			  cout << "Force steps0 = 0" << endl;
			  for (int newSteps0 = 0; newSteps0 <= steps0; newSteps0++){
			  cout << "Steps0: " << newSteps0 << ": "; zeroCostClosed.at(h)[newSteps0].print(0,2);
			  cout << "CUT: "; cut.print(0, 1);
			  }
			  );*/
		    steps0 = 0;
		}
	    }

	    if (h > 0 && steps0 == 0) {
		bool found = false;
		for (auto key : trs) { //TODO: maybe is best to use the inverse order
		    if (found)
			break;
		    int newH = h - key.first;
		    if (key.first == 0 || closed.count(newH) == 0)
			continue;
		    for (TransitionRelation &tr : key.second) {
			//DEBUG_MSG(cout << "Check " << tr.getOps().size() << " " << (*(tr.getOps().begin()))->get_name() << " of cost " << key.first << " in h=" << newH << endl;);
			Bdd succ;
			if (fw) {
			    succ = tr.preimage(cut);
			} else {
			    succ = tr.image(cut);
			}
			Bdd intersection = succ * closed.at(newH);
			/*DEBUG_MSG(cout << "Image computed: "; succ.print(0,1);
			  cout << "closed at newh: "; closed.at(newH).print(0,1);
			  cout << "Intersection: "; intersection.print(0,1););*/
			if (!intersection.IsZero()) {
			    h = newH;
			    cut = succ;
			    steps0 = 0;
			    if (zeroCostClosed.count(h)) {
				while ((cut * zeroCostClosed.at(h)[steps0]).IsZero()) {
				    //DEBUG_MSG(cout << "r Steps0 is not " << steps0<< " of " << zeroCostClosed.at(h).size() << endl;);
				    steps0++;
				    assert(steps0 < zeroCostClosed.at(newH).size());
				}

				//DEBUG_MSG(cout << "r Steps0 of h=" << h << " is " << steps0 << endl;);

				cut *= zeroCostClosed.at(h)[steps0];
			    }
			    path.push_back(*(tr.getOpsIds().begin()));

			    //DEBUG_MSG(cout << "Selected " << path.back()->get_name() << endl;);

			    found = true;
			    break;
			}
		    }
		}
		if (!found) {
		    cerr << "Error: Solution reconstruction failed: " << endl;
		    utils::exit_with(utils::ExitCode::SEARCH_CRITICAL_ERROR);
		}
	    }
	}

	DEBUG_MSG(cout << "Sym closed extracted path" << endl;
	    );
    }

    SymSolution ClosedList::checkCut(UnidirectionalSearch * search, const Bdd &states, int g, bool fw) const {
	Bdd cut_candidate = states * closedTotal;
	if (cut_candidate.IsZero()) {
	    return SymSolution(); //No solution yet :(
	}

	for (const auto &closedH : closed) {
	    int h = closedH.first;

	    // cout << "Check cut of g=" << g << " with h=" << h << endl;
	    Bdd cut = closedH.second * cut_candidate;
	    if (!cut.IsZero()) {
		if (fw) //Solution reconstruction will fail
		    return SymSolution(search, my_search, g, h, cut);
		else
		    return SymSolution(my_search, search, h, g, cut);
	    }
	}

	cerr << "Error: Cut with closedTotal but not found on closed" << endl;
	utils::exit_with(utils::ExitCode::SEARCH_CRITICAL_ERROR);
    }

	std::vector<SymSolution> ClosedList::getAllCuts(UnidirectionalSearch *search, const Bdd &states, int g, bool fw, int lower_bound) const
	{
		std::vector<SymSolution> result;
		Bdd cut_candidate = states * closedTotal;
		if (cut_candidate.IsZero()) {
	    	result.emplace_back();
		} else {
			for (const auto &closedH : closed) {
				int h = closedH.first;

				if (g + h < lower_bound)
				{
					continue;
				}

				// cout << "Check cut of g=" << g << " with h=" << h << endl;
				Bdd cut = closedH.second * cut_candidate;
				if (!cut.IsZero()) {
					if (fw) {
						result.emplace_back(search, my_search, g, h, cut);
					} else {
						result.emplace_back(my_search, search, h, g, cut);
					}
				}	
			}
		}
		if (result.size() == 0)
		{
			result.emplace_back();
		}
		return result;
	}

    void ClosedList::statistics() const {
	// cout << "h (eval " << num_calls_eval << ", not_closed" << time_eval_states << "s, closed " << time_closed_states
	//   << "s, pruned " << time_pruned_states << "s, some " << time_prune_some
	//   << "s, all " << time_prune_all  << ", children " << time_prune_some_children << "s)";
    }

    const std::set<int> &ClosedList::getHValues() {
	assert(h_values.count(hNotClosed));
    
	return h_values;
    }
}