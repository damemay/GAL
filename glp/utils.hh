#pragma once

#include <string>
#include <format>

namespace glp {
    namespace util {
        std::string read_file(const std::string& path);
        void print(const std::string& formatted_string);
    }
}
