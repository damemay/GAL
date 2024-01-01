#pragma once

#include <string>
#include <map>
#include "shader.hh"
#include "material.hh"
#include "../res/fonts/karla.h"

namespace glp {

struct Font {
        Texture* texture {nullptr};
        Shader* shader {nullptr};
        size_t screen_width, screen_height;

        Font(const size_t& screen_w, const size_t& screen_h, const std::string& path="");
        ~Font();

        Font(const Font&) = delete;
        Font& operator=(const Font&) = delete;
};

class Text {
    private:
        Font* font;
        std::string content;
        GLuint VAO, VBO;

    public:
        Text(Font* font);
        ~Text();

        void update(const std::string& text, uint8_t size, uint16_t x, uint16_t y);
        void render();
    
        Text(const Text&) = delete;
        Text& operator=(const Text&) = delete;
};

}
