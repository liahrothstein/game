#include "game/Game.h"
#include "stb_image.h"
#include <iostream>

static void applyMovementScreen(glm::vec2& camPos, float speed) {
    const bool* keys = SDL_GetKeyboardState(nullptr);
    glm::vec2 dir{0.0f, 0.0f};
    if (keys[SDL_SCANCODE_W]) dir += glm::vec2{0.0f, -1.0f};
    if (keys[SDL_SCANCODE_S]) dir += glm::vec2{0.0f,  1.0f};
    if (keys[SDL_SCANCODE_A]) dir += glm::vec2{-1.0f, 0.0f};
    if (keys[SDL_SCANCODE_D]) dir += glm::vec2{ 1.0f, 0.0f};
    float s = 0.70710678f;
    camPos.x += (dir.x * s - dir.y * s) * speed;
    camPos.y += (dir.x * s + dir.y * s) * speed;
}

bool Game::init() {
    if (!window.init("Kosti i Pepel — dev", 1280, 720)) return false;
    return initQuad(R"(D:\Development\game\assets\test.png)");
}

bool Game::initQuad(const char* pngPath) {
    quadShader.build(
        R"(#version 330 core
           layout(location=0) in vec2 aPos;
           layout(location=1) in vec2 aUV;
           uniform mat4 uProj, uView, uModel;
           out vec2 vUV;
           void main() { vUV = aUV;
               gl_Position = uProj * uView * uModel * vec4(aPos.x, 0.0, aPos.y, 1.0); })",
        R"(#version 330 core
           in vec2 vUV;
           uniform sampler2D uTex;
           out vec4 FragColor;
           void main() { FragColor = texture(uTex, vUV); })");

    float v[] = { -1.0f,-1.0f, 0,1,   1.0f,-1.0f, 1,1,
                   1.0f, 1.0f, 1,0,  -1.0f, 1.0f, 0,0 };
    GLuint idx[] = { 0,1,2, 0,2,3 };
    glGenVertexArrays(1, &quadVao);
    glGenBuffers(1, &quadVbo);
    glGenBuffers(1, &quadEbo);
    glBindVertexArray(quadVao);
    glBindBuffer(GL_ARRAY_BUFFER, quadVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));
    glBindVertexArray(0);

    stbi_set_flip_vertically_on_load(true);
    int w, h, n;
    unsigned char* data = stbi_load(pngPath, &w, &h, &n, 0);
    if (!data) { std::cerr << "Texture load failed: " << pngPath << "\n"; return false; }
    glGenTextures(1, &quadTex);
    glBindTexture(GL_TEXTURE_2D, quadTex);
    GLenum fmt = (n == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(data);
    return true;
}

void Game::drawQuad() {
    float aspect = (float)window.width / (float)window.height;   // ← новая строка
    quadShader.use();
    quadShader.setMat4("uProj",  camera.proj(aspect));           // ← аспект передаётся
    quadShader.setMat4("uView",  camera.view());
    quadShader.setMat4("uModel", glm::mat4(1.0f));
    glBindTexture(GL_TEXTURE_2D, quadTex);
    glBindVertexArray(quadVao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

void Game::run() {
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = false;
            if (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE)
                running = false;
        }
        applyMovementScreen(camera.pos, 0.15f);

        window.pollSize();
        glViewport(0, 0, window.width, window.height);
        glClearColor(0.05f, 0.05f, 0.06f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        drawQuad();
        window.swap();
    }
}

void Game::shutdown() { /* GL-ресурсы освобождаются с контекстом; окно ниже */ }