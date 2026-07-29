#ifndef TEXT_H
#define TEXT_H

#include <glad/gl.h>
#include <string>

class TextRenderer {
public:
    TextRenderer()  = default;
    ~TextRenderer() { cleanup(); }

    bool init(int winW, int winH);

    void renderText(float x, float y, const std::string& text,
                    float scale, const float color[4]);

    void renderHUD(int winW, int winH,
                   bool gravOn, bool springOn, bool dampOn, bool windOn,
                   bool trailOn, bool arrowsOn, bool paused,
                   float stiffness, float damping, float mass);

    void cleanup();

private:
    GLuint m_tex    = 0;
    GLuint m_vao    = 0;
    GLuint m_vbo    = 0;
    GLuint m_shader = 0;

    void buildTexture();
    void buildGlyph(GLubyte* buf, int stride, int ch);
};

#endif // TEXT_H
