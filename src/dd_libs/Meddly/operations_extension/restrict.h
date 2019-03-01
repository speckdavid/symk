// Copyright 05.07.2018, University of Freiburg,
// Author: David Speck <speckd>.

#ifndef RESTRICT_H_
#define RESTRICT_H_

namespace USER_OPS
{
extern const MEDDLY::unary_opname *RESTRICT;
void initializeRestrict();
void setRestrictVarVal(const MEDDLY::dd_edge &arg, const MEDDLY::dd_edge &res,
                       int var, int val);
} // namespace USER_OPS

#endif // RESTRICT_H_