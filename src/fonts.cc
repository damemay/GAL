#include "fonts.hh"
#include "utils.hh"

namespace glp {

#ifndef __vita__
constexpr auto text_vert = "#version 330 core\n"
"layout(location = 0) in vec2 position;\n"
"layout(location = 1) in vec2 uv_in;\n"
"out vec2 uv;\n"
"void main() {\n"
"    gl_Position = vec4(position, 0.0f, 1.0f);\n"
"    uv = uv_in;\n"
"}\n";

constexpr auto text_frag = "#version 330 core\n"
"in vec2 uv;\n"
"out vec4 out_color;\n"
"uniform sampler2D tex;\n"
"void main() {\n"
"    out_color = texture(tex, vec2(uv.x, uv.y));\n"
"}\n";
#else
constexpr auto text_vert = "void main(\n"
"    float2 position,\n"
"    float2 uv_in,\n"
"    float4 out gl_Position : POSITION,\n"
"    float2 out uv : TEXCOORD0)\n"
"{\n"
"    gl_Position = float4(position, 0.0, 1.0);\n"
"    uv = uv_in;\n"
"}\n";

constexpr auto text_frag = "float4 main(\n"
"    float4 gl_FragColor : COLOR,\n"
"    float2 uv : TEXCOORD0,\n"
"    uniform sampler2D tex\n"
") {\n"
"    gl_FragColor = tex2D(tex, float2(uv.x, uv.y));\n"
"    return gl_FragColor;\n"
"}\n";
#endif

Font::Font(const size_t& screen_w, const size_t& screen_h, const std::string& path)
    : screen_width{screen_w}, screen_height{screen_h} {
        if(path.empty()) {
            extern const unsigned char karla_png[];
            extern const unsigned int karla_png_len;
            texture = new Texture{karla_png, karla_png_len};
        } else texture = new Texture{path};

        shader = new Shader{text_vert, text_frag, false};
}

Font::~Font() {
    delete texture;
    delete shader;
}

Text::Text(Font* font_) : font{font_} {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
}

void Text::update(const std::string& text, uint8_t size, uint16_t x, uint16_t y) {
    content = text;
    std::vector<glm::vec2> verts;
    for(size_t i=0; i<content.size(); i++) {
        char character = content.data()[i];

        glm::vec2 v_up_left     {x+i*(size/2), y+size};
        v_up_left -= glm::vec2(font->screen_width/2, font->screen_height/2);
        v_up_left /= glm::vec2(font->screen_width/2, font->screen_height/2);
        glm::vec2 v_up_right    {x+i*(size/2)+size, y+size};
        v_up_right -= glm::vec2(font->screen_width/2, font->screen_height/2);
        v_up_right /= glm::vec2(font->screen_width/2, font->screen_height/2);
        glm::vec2 v_down_right  {x+i*(size/2)+size, y};
        v_down_right -= glm::vec2(font->screen_width/2, font->screen_height/2);
        v_down_right /= glm::vec2(font->screen_width/2, font->screen_height/2);
        glm::vec2 v_down_left   {x+i*(size/2), y};
        v_down_left -= glm::vec2(font->screen_width/2, font->screen_height/2);
        v_down_left /= glm::vec2(font->screen_width/2, font->screen_height/2);

        float x = (character%16)/16.0f;
        float y = (character/16)/16.0f;
        glm::vec2 uv_up_left    {x, y};
        glm::vec2 uv_up_right   {x+1.0f/16.0f, y};
        glm::vec2 uv_down_right {x+1.0f/16.0f, y+1.0f/16.0f};
        glm::vec2 uv_down_left  {x, y+1.0f/16.0f};

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
    font->shader->bind();
    GLuint id = glGetAttribLocation(shader->get(), "position");
    glVertexAttribPointer(id, 2, GL_FLOAT, GL_FALSE, 2*sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(id);
    id = glGetAttribLocation(shader->get(), "uv_in");
    glVertexAttribPointer(id, 2, GL_FLOAT, GL_FALSE, 2*sizeof(glm::vec2), (void*)(sizeof(glm::vec2)));
    glEnableVertexAttribArray(id);

#endif
}

void Text::render() {
    glDisable(GL_DEPTH_TEST);
    font->shader->bind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, font->texture->id);
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

}
