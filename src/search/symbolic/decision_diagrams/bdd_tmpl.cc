#include "bdd_tmpl.h"
#include <iostream>

namespace symbolic
{
void exceptionError(std::string /*message*/)
{
    throw BDDError();
}
} // namespace symbolic
