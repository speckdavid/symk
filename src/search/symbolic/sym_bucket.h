#ifndef SYMBOLIC_SYM_BUCKET_H
#define SYMBOLIC_SYM_BUCKET_H

#include "cuddObj.hh"

#include <vector>

namespace symbolic {
typedef std::vector<BDD> Bucket;

void removeZero(Bucket &bucket);
void copyBucket(const Bucket &bucket, Bucket &res);
void moveBucket(Bucket &bucket, Bucket &res);
int nodeCount(const Bucket &bucket);
bool extract_states(Bucket &list, const Bucket &pruned, Bucket &res);
bool bucket_contains_any_state(const Bucket &bucket, const BDD &bdd);
}

#endif
