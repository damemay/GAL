#pragma once

#include <SDL2/SDL_ttf.h>
#include <string>
#include <map>
#include "shader.hh"

constexpr uint8_t ascii_count {128};
constexpr uint16_t font_atlas_size {1024};

struct Glyph {
    glm::vec2 uv{0, 0};
    glm::vec2 size{0, 0};
    float advance;
};

struct Font {
        std::map<char, Glyph> glyphs;
        Texture* texture {nullptr};
        Shader* shader {nullptr};

        uint8_t size {0};
        int line_skip {0};

        Font(const std::string& path, uint8_t size);
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

        void update(const std::string& text, int x, int y);
        void render();
    
        Text(const Text&) = delete;
        Text& operator=(const Text&) = delete;
};
