#include "sym_search.h"

using namespace std;

namespace symbolic {
SymSearch::SymSearch(SymbolicSearch *eng, const SymParamsSearch &params)
    : mgr(nullptr), p(params), engine(eng) {}
} // namespace symbolic
