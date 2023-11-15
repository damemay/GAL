#include <fstream>
#include <sstream>
#include <cstring>

#include "utils.hh"

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
        glp_logv("could not read file %s -- %s", path.c_str(), e.what());
    }
    return content.str();
}

std::string compress(const std::string& data, int compress_level) {
    size_t est_size = ZSTD_compressBound(data.size());
    std::string compressed{};

    compressed.resize(est_size);
    auto size = ZSTD_compress((void*)compressed.data(),
            est_size, data.data(), data.size(), compress_level);
    compressed.resize(size);
    compressed.shrink_to_fit();

    return compressed;
}

std::string decompress(const std::string& data) {
    auto const est_size = ZSTD_getFrameContentSize(data.data(), data.size());
    std::string decompressed{};

    decompressed.resize(est_size);
    size_t const size = ZSTD_decompress((void*)decompressed.data(),
            est_size, data.data(), data.size());
    decompressed.resize(size);
    decompressed.shrink_to_fit();
    
    return decompressed;
}

};

}
