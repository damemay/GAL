#include <stdexcept>
#include <utils.hh>
#include <fstream>
#include <sstream>
#include <format>

namespace glp {
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
                auto exception = std::format("could not read file {}: {}", path, e.what());
                throw std::runtime_error(exception);
            }
            return content.str();
        }

        void print(const std::string& formatted_string) {
            fprintf(stderr, "glp: %s\n", formatted_string.c_str());
        }
    }
}
