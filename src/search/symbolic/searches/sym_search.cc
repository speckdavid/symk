#include "sym_search.h"

using namespace std;

namespace symbolic {
SymSearch::SymSearch(SymbolicSearch *eng, const SymParameters &params)
    : mgr(nullptr), sym_params(params), engine(eng) {}
}
