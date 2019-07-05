#ifndef SYMBOLIC_SYM_CONTROLLER_H
#define SYMBOLIC_SYM_CONTROLLER_H

// Shared class for SymEngine and smas_heuristic

#include "sym_enums.h"
#include "sym_params_search.h"
#include "sym_solution_cut.h"
#include "sym_solution_registry.h"
#include "sym_state_space_manager.h"
#include "unidirectional_search.h"

#include <limits>
#include <memory>
#include <vector>

namespace options {
    class OptionParser;
    class Options;
} // namespace options

namespace symbolic {
    class SymVariables;

    class SymController {
    protected:
        std::shared_ptr<SymVariables> vars; // The symbolic variables are declared

        SymParamsMgr mgrParams; // Parameters for SymStateSpaceManager configuration.
        SymParamsSearch searchParams; // Parameters to search the original state space

        int lower_bound; // Lower bound of search (incl. min-action costs)
        int min_g; // min g costs of open lists

        SymSolutionRegistry solution_registry; // Solution registry

    public:
        SymController(const options::Options &opts);

        virtual ~SymController() {
        }
        
        void init(UnidirectionalSearch* fwd_search, UnidirectionalSearch* bwd_search);

        virtual void new_solution(const SymSolutionCut &sol);
        void setLowerBound(int lower);

        void setMinG(int g) {
            min_g = std::max(g, min_g);
        }

        int getUpperBound() const {
            return std::numeric_limits<int>::max();
        }

        int getLowerBound() const {
            return lower_bound;
        }

        int getMinG() const {
            return min_g;
        }

        bool solved() const {
            return getLowerBound() >= getUpperBound();
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

        BDD get_states_on_goal_paths() const;

        static void add_options_to_parser(options::OptionParser &parser,
                int maxStepTime, int maxStepNodes);
    };
} // namespace symbolic
#endif
