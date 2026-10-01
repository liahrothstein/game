#include <glad/glad.h>
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "render/Shader.h"
#include "stb_image.h"

// ══════════════════════════════════════════════════════
//                    КОНСТАНТЫ
// ══════════════════════════════════════════════════════
constexpr int   WINDOW_W = 1280;
constexpr int   WINDOW_H = 720;
constexpr float HALF_W   = WINDOW_W / 2.0f;
constexpr float HALF_H   = WINDOW_H / 2.0f;

// ══════════════════════════════════════════════════════
//                    ПЛАТФОРМА
// ══════════════════════════════════════════════════════

static SDL_Window*   g_window = nullptr;
static SDL_GLContext g_gl     = nullptr;

static bool initPlatform() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init: " << SDL_GetError() << "\n";
        return false;
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    g_window = SDL_CreateWindow("Kosti i Pepel — dev",
                                WINDOW_W, WINDOW_H,
                                SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!g_window) { std::cerr << "Window: " << SDL_GetError() << "\n"; return false; }

    g_gl = SDL_GL_CreateContext(g_window);
    SDL_GL_MakeCurrent(g_window, g_gl);
    SDL_GL_SetSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        std::cerr << "glad init failed\n";
        return false;
    }
    std::cout << "OpenGL: " << glGetString(GL_VERSION) << "\n";
    return true;
}

static void shutdownPlatform() {
    SDL_GL_DestroyContext(g_gl);
    SDL_DestroyWindow(g_window);
    SDL_Quit();
}

// ══════════════════════════════════════════════════════
//                    ВВОД
// ══════════════════════════════════════════════════════

static bool pollEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_QUIT) return false;
        if (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE) return false;
    }
    return true;
}

static void applyMovementScreen(glm::vec2& camPos, float speed) {
    const bool* keys = SDL_GetKeyboardState(nullptr);
    glm::vec2 dir{0.0f, 0.0f};
    if (keys[SDL_SCANCODE_W]) dir += glm::vec2{0.0f, -1.0f};   // явный тип
    if (keys[SDL_SCANCODE_S]) dir += glm::vec2{0.0f,  1.0f};
    if (keys[SDL_SCANCODE_A]) dir += glm::vec2{-1.0f, 0.0f};
    if (keys[SDL_SCANCODE_D]) dir += glm::vec2{ 1.0f, 0.0f};
    // Поворот на -45° (обратный к повороту изометрической камеры)
    float s = 0.70710678f;
    glm::vec2 world;
    world.x = dir.x * s - dir.y * s;
    world.y = dir.x * s + dir.y * s;
    camPos += world * speed;
}

struct Quad {
    Shader shader;
    GLuint vao=0, vbo=0, ebo=0, tex=0;
};

static void initQuad(Quad& q, const char* pngPath) {
    q.shader.build(
        R"(#version 330 core
           layout(location=0) in vec2 aPos;   // квад в плоскости XZ: (x, z)
           layout(location=1) in vec2 aUV;
           uniform mat4 uProj, uView, uModel;
           out vec2 vUV;
           void main() {
               vUV = aUV;
               gl_Position = uProj * uView * uModel * vec4(aPos.x, 0.0, aPos.y, 1.0);
           })",
        R"(#version 330 core
           in vec2 vUV;
           uniform sampler2D uTex;
           out vec4 FragColor;
           void main() { FragColor = texture(uTex, vUV); })");

    // 4 вершины: pos(x,y), uv(u,v); индексы: 2 треугольника
    float v[] = {
        -0.3f,-0.3f, 0,1,   // левый низ
         0.3f,-0.3f, 1,1,   // правый низ
         0.3f, 0.3f, 1,0,   // правый верх
        -0.3f, 0.3f, 0,0 }; // левый верх
    GLuint idx[] = { 0,1,2, 0,2,3 };

    glGenVertexArrays(1, &q.vao);
    glGenBuffers(1, &q.vbo);
    glGenBuffers(1, &q.ebo);
    glBindVertexArray(q.vao);
    glBindBuffer(GL_ARRAY_BUFFER, q.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, q.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));
    glBindVertexArray(0);

    // Текстура
    stbi_set_flip_vertically_on_load(true);   // PNG снизу-вверх → GL
    int w,h,n;
    unsigned char* data = stbi_load(pngPath, &w, &h, &n, 0);
    if (!data) { std::cerr << "Texture load failed: " << pngPath << "\n"; return; }
    glGenTextures(1, &q.tex);
    glBindTexture(GL_TEXTURE_2D, q.tex);
    GLenum fmt = (n == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(data);
}

static void drawQuad(const Quad& q, const glm::vec2& screenPos) {
    glm::vec2 clip((screenPos.x - HALF_W)/HALF_W, (HALF_H - screenPos.y)/HALF_H);
    q.shader.use();
    q.shader.setVec2("uPos", clip);
    glBindTexture(GL_TEXTURE_2D, q.tex);
    glBindVertexArray(q.vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

static glm::mat4 makeProj() {
    int w = WINDOW_W, h = WINDOW_H;
    SDL_GetWindowSizeInPixels(g_window, &w, &h);   // SDL3
    float aspect = (float)w / (float)h;
    return glm::ortho(-4.0f * aspect, 4.0f * aspect, -4.0f, 4.0f, 0.1f, 100.0f);
}

static glm::mat4 makeView(const glm::vec2& camPos) {
    // Глаз: над точкой camPos, смещён назад-вверх (изометрический угол ~43°)
    glm::vec3 eye  (camPos.x + 6.0f, 8.0f, camPos.y + 6.0f);
    glm::vec3 center(camPos.x, 0.0f, camPos.y);   // смотрим на точку на земле
    glm::vec3 up   (0.0f, 1.0f, 0.0f);
    return glm::lookAt(eye, center, up);
}

// ══════════════════════════════════════════════════════
//                    РЕНДЕР
// ══════════════════════════════════════════════════════

struct Triangle {          // GPU-ресурсы треугольника — создаём один раз
    Shader shader;
    GLuint vao = 0, vbo = 0;
};

static void initTriangle(Triangle& t) {
    t.shader.build(
        R"(#version 330 core
           layout(location=0) in vec2 aPos;
           layout(location=1) in vec2 aUV;      // ← НОВОЕ: UV-координаты
           uniform vec2 uPos;
           out vec2 vUV;                        // ← НОВОЕ: передаём UV во фрагментный
           void main() { vUV = aUV; gl_Position = vec4(aPos + uPos, 0.0, 1.0); })",
        R"(#version 330 core
           in vec2 vUV;                         // ← НОВОЕ: принимаем UV
           uniform sampler2D uTex;              // ← НОВОЕ: текстура
           out vec4 FragColor;
           void main() { FragColor = texture(uTex, vUV); })");  // ← цвет = цвет текстуры в точке UV

    float verts[] = { -0.1f,-0.1f,  0.1f,-0.1f,  0.0f,0.15f };
    glGenVertexArrays(1, &t.vao);
    glGenBuffers(1, &t.vbo);
    glBindVertexArray(t.vao);
    glBindBuffer(GL_ARRAY_BUFFER, t.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glBindVertexArray(0);
}

static void beginFrame() {          // очистка — начало кадра
    glViewport(0, 0, WINDOW_W, WINDOW_H);
    glClearColor(0.05f, 0.05f, 0.06f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

static void drawTriangle(const Triangle& t, const glm::vec2& screenPos) {
    // SDL: y растёт вниз; GL clip-space: y растёт вверх → инвертируем и нормируем
    glm::vec2 clip((screenPos.x - HALF_W) / HALF_W,
                   (HALF_H - screenPos.y) / HALF_H);
    t.shader.use();
    t.shader.setVec2("uPos", clip);
    glBindVertexArray(t.vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

static void endFrame(SDL_Window* w) { SDL_GL_SwapWindow(w); }

// ══════════════════════════════════════════════════════
//                    MAIN — только orchestration
// ══════════════════════════════════════════════════════

int main(int, char**) {
    if (!initPlatform()) { shutdownPlatform(); return 1; }

    // ─── GPU-ресурсы: квад (создаётся ОДИН раз) ───
    Quad quad;
    initQuad(quad, R"(D:\Development\game\assets\test.png)");

    // ─── Камера ───
    glm::vec2 camPos{0.0f, 0.0f};

    bool running = true;
    while (running) {
        running = pollEvents();
        applyMovementScreen(camPos, 0.15f);     // WASD двигает КАМЕРУ (метры)

        beginFrame();

        glm::mat4 proj  = makeProj();
        glm::mat4 view  = makeView(camPos);
        glm::mat4 model = glm::mat4(1.0f);

        quad.shader.use();
        quad.shader.setMat4("uProj", proj);
        quad.shader.setMat4("uView", view);
        quad.shader.setMat4("uModel", model);
        glBindTexture(GL_TEXTURE_2D, quad.tex);
        glBindVertexArray(quad.vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        endFrame(g_window);
    }

    shutdownPlatform();
    return 0;
}