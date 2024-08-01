#pragma once
#include <string>
#include <cctype>
#include <algorithm>

namespace string_util {
    inline std::string to_lower(std::string const& s) {
        std::string out(s);
        std::transform(s.begin(), s.end(), out.begin(), [](char c) {return std::tolower(c); });
        return out;
    }
}