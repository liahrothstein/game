#pragma once
#include "platform/Window.h"
#include "render/Camera.h"
#include "render/Shader.h"
#include <glad/glad.h>
#include <glm/glm.hpp>

class Game {
public:
    bool init();
    void run();          // цикл целиком
    void shutdown();

private:
    Window window;
    Camera camera;

    // квад (этап 1) — на этапе 2 заменится на Mesh+текстуры
    Shader quadShader;
    GLuint quadVao = 0, quadVbo = 0, quadEbo = 0, quadTex = 0;

    bool running = true;
    bool initQuad(const char* pngPath);
    void drawQuad();
};