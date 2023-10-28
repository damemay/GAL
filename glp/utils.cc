#include <fstream>
#include <sstream>
#include <cstring>

#include "utils.hh"

namespace util {

std::string read_file(const std::string& path) {
    std::ifstream file;
    std::stringstream content;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        file.open(path);
        content << file.rdbuf();
        file.close();
    } catch(std::ifstream::failure& e) {
        glp_logv("could not read file %s -- %s", path.c_str(), e.what());
    }
    return content.str();
}

};
