#include "sym_search.h"

using namespace std;

namespace symbolic
{
SymSearch::SymSearch(SymController *eng, const SymParamsSearch &params) : mgr(nullptr), p(params), engine(eng) {}
} // namespace symbolic