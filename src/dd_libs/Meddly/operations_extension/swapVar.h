// Copyright 05.07.2018, University of Freiburg,
// Author: David Speck <speckd>.

#ifndef SWAPVAR_H_
#define SWAPVAR_H_

namespace USER_OPS
{
extern const MEDDLY::unary_opname *SWAPVAR;
void initializeSwapVar();
void setSwapVar(const MEDDLY::dd_edge &arg, const MEDDLY::dd_edge &res,
                       int from, int to);
} // namespace USER_OPS

#endif // SWAPVAR_H_