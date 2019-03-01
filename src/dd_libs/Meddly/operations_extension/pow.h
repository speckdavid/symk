// Copyright 05.07.2018, University of Freiburg,
// Author: David Speck <speckd>.

#ifndef POW_H_
#define POW_H_

namespace USER_OPS
{
extern const MEDDLY::unary_opname *POW;
void initializePow();
void setPowExp(const MEDDLY::dd_edge &arg, const MEDDLY::dd_edge &res,
                       float exp);
} // namespace USER_OPS

#endif // POW_H_