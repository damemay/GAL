#include "fonts.hh"
#include "utils.hh"

Font::Font(const std::string& path, uint8_t size_) : size{size_} {
    TTF_Font* sdl_font = TTF_OpenFont(path.c_str(), size);
    if(!sdl_font) {
        glp_logv("SDL_ttf could not load: %s\n SDL:%s", path.c_str(), TTF_GetError());
        return;
    }
    
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    SDL_Surface* sdl_surface = SDL_CreateRGBSurface(0, font_atlas_size, font_atlas_size,
            32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
#else
    SDL_Surface* sdl_surface = SDL_CreateRGBSurface(0, font_atlas_size, font_atlas_size,
            32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
#endif

    if(!sdl_surface) {
        glp_logv("could not create surface for font atlas: %s", SDL_GetError());
        return;
    }

    SDL_Rect dest = {0, 0, 0, 0};
    for(uint32_t i=' '; i<='~'; ++i) {
        auto g = TTF_RenderGlyph32_Blended(sdl_font, i, {255, 255, 255, 255});
        if(!g) {
            glp_logv("could not render glyph: %s", TTF_GetError());
            return;
        }

        SDL_SetSurfaceBlendMode(g, SDL_BLENDMODE_NONE);

        int minx{0}, maxx{0}, miny{0}, maxy{0}, advance{0};
        TTF_GlyphMetrics32(sdl_font, i, &minx, &maxx, &miny, &maxy, &advance);
        dest.w = maxx-minx;
        dest.h = maxy-miny;

        line_skip = TTF_FontLineSkip(sdl_font);

        if(dest.x+dest.w >= font_atlas_size) {
            dest.x = 0;
            dest.y = dest.h + line_skip;
            if(dest.y+dest.h >= font_atlas_size) {
                glp_log("out of space in font atlas");
                return;
            }
        }

        SDL_BlitSurface(g, nullptr, sdl_surface, &dest);
        SDL_FreeSurface(g);

        Glyph glyph = {{dest.x, dest.y}, {dest.w, dest.h}, (float)advance};
        glyphs.insert(std::pair<char, Glyph>(i, glyph));

        dest.x += dest.w;
    }

    texture = new Texture{sdl_surface->w, sdl_surface->h, 4, sdl_surface->pixels};

#ifndef __vita__
    shader = new Shader{"../res/shaders/text.vert", "../res/shaders/text.frag"};
#else

#endif

    SDL_FreeSurface(sdl_surface);
    TTF_CloseFont(sdl_font);
}

Font::~Font() {
    delete texture;
}

Text::Text(Font* font_) : font{font_} {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
}

void Text::update(const std::string& text, int x, int y) {
    content = text;
    std::vector<glm::vec2> verts;
    for(size_t i=0; i<content.size(); i++) {
        char character = content.data()[i];
        auto glyph = font->glyphs[character];

        glm::vec2 v_up_left     {-1.0f, 1.0f};
        glm::vec2 v_up_right    {1.0f, 1.0f};
        glm::vec2 v_down_right  {1.0f, -1.0f};
        glm::vec2 v_down_left   {-1.0f, -1.0f};

        glm::vec2 uv_up_left    {glyph.uv.x/              (float)font->size, 1.0f-(glyph.uv.y/               (float)font->size)};
        glm::vec2 uv_up_right   {glyph.uv.x+glyph.advance/(float)font->size, 1.0f-(glyph.uv.y/               (float)font->size)};
        glm::vec2 uv_down_right {glyph.uv.x+glyph.advance/(float)font->size, 1.0f-((glyph.uv.y-glyph.size.y)/(float)font->size)};
        glm::vec2 uv_down_left  {glyph.uv.x/              (float)font->size, 1.0f-((glyph.uv.y-glyph.size.y)/(float)font->size)};

        verts.push_back(v_up_left);
        verts.push_back(uv_up_left);
        verts.push_back(v_down_left);
        verts.push_back(uv_down_left);
        verts.push_back(v_up_right);
        verts.push_back(uv_up_right);
        verts.push_back(v_down_right);
        verts.push_back(uv_down_right);
        verts.push_back(v_up_right);
        verts.push_back(uv_up_right);
        verts.push_back(v_down_left);
        verts.push_back(uv_down_left);
    }

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(glm::vec2), verts.data(), GL_STATIC_DRAW);
#ifndef __vita__
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2*sizeof(glm::vec2), (void*)(sizeof(glm::vec2)));
    glEnableVertexAttribArray(1);
#else

#endif
}

void Text::render() {
    glDisable(GL_DEPTH_TEST);
    glBindTexture(GL_TEXTURE_2D, font->texture->id);
    font->shader->bind();
    glBindVertexArray(VAO);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDrawArrays(GL_TRIANGLES, 0, content.size()*6);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

Text::~Text() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}
