#ifndef SDAC_PARSER_GLOBALS_H
#define SDAC_PARSER_NUMERIC_GLOBALS_H

#include <vector>
#include <string>
#include <map>

namespace sdac_parser {
// Global using directives
using Ordering = std::vector<std::string>;
using Domains = std::map<std::string, int>;
using ConcreteState = std::vector<int>;
}


#endif
