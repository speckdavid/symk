// $Id$

#ifndef REORDERING_FACTORY_H
#define REORDERING_FACTORY_H

#include <memory>

#include "meddly.h"

#include "reordering_base.h"

#include "lowest_inversion_reordering.h"
#include "highest_inversion_reordering.h"
#include "sink_down_reordering.h"
#include "bring_up_reordering.h"
#include "lowest_cost_reordering.h"
#include "lowest_memory_reordering.h"
#include "random_reordering.h"
#include "larc_reordering.h"

namespace MEDDLY {

class reordering_factory
{
private:
  reordering_factory();

public:
  static std::unique_ptr<reordering_base> create(forest::policies::reordering_type r);
};

inline std::unique_ptr<reordering_base> reordering_factory::create(forest::policies::reordering_type r)
{
  switch(r) {
  case forest::policies::reordering_type::LOWEST_INVERSION:
    return std::unique_ptr<reordering_base>(new lowest_inversion_reordering());
  case forest::policies::reordering_type::HIGHEST_INVERSION:
    return std::unique_ptr<reordering_base>(new highest_inversion_reordering());
  case forest::policies::reordering_type::SINK_DOWN:
    return std::unique_ptr<reordering_base>(new sink_down_reordering());
  case forest::policies::reordering_type::BRING_UP:
    return std::unique_ptr<reordering_base>(new bring_up_reordering());
  case forest::policies::reordering_type::LOWEST_COST:
    return std::unique_ptr<reordering_base>(new lowest_cost_reordering());
  case forest::policies::reordering_type::LOWEST_MEMORY:
    return std::unique_ptr<reordering_base>(new lowest_memory_reordering());
  case forest::policies::reordering_type::RANDOM:
    return std::unique_ptr<reordering_base>(new random_reordering());
  case forest::policies::reordering_type::LARC:
    return std::unique_ptr<reordering_base>(new larc_reordering());
  default:
    throw error(error::INVALID_ARGUMENT);
  }
}

}

#endif
