#ifndef SDAC_PARSER_STRING_UTILS_H
#define SDAC_PARSER_STRING_UTILS_H

#include <algorithm>
#include <cctype>
#include <iterator>
#include <locale>
#include <sstream>
#include <string>
#include <vector>

namespace sdac_parser {
struct StringUtils {
    // trim from start (in place)
    static void ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                        [](int ch) {return !std::isspace(ch);}));
    }

    // trim from end (in place)
    static void rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(),
                             [](int ch) {return !std::isspace(ch);})
                .base(),
                s.end());
    }

    // trim from both ends (in place)
    static void trim(std::string &s) {
        ltrim(s);
        rtrim(s);
    }

    // Split string by delimiter into result
    template<typename Out>
    static void split(const std::string &s, char delim, Out result) {
        std::stringstream ss(s);
        std::string item;
        while (std::getline(ss, item, delim)) {
            *(result++) = item;
        }
    }

    // Split string by delimiter
    static std::vector<std::string> split(const std::string &s, char delim) {
        std::vector<std::string> elems;
        split(s, delim, std::back_inserter(elems));
        return elems;
    }

    // Split by parantheses
    std::vector<std::string> static tokenize(std::string const &s,
                                             char open_paren, char close_paren) {
        std::vector<std::string> result;
        int openParens = 0;
        std::stringstream tmp;
        for (size_t pos = 0; pos < s.length(); ++pos) {
            tmp << s[pos];
            if (s[pos] == open_paren) {
                openParens++;
            } else if (s[pos] == close_paren) {
                openParens--;
                if (openParens == 0) {
                    std::string token = tmp.str();
                    trim(token);
                    result.push_back(token);
                    tmp.str("");
                }
            }
        }
        return result;
    }
};
}

#endif
