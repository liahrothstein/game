#pragma once
#include "platform/Window.h"
#include "render/Camera.h"
#include "render/Shader.h"
#include "render/Mesh.h"
#include "assets/GltfLoader.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

class Game {
public:
    bool init();
    void run();
    void shutdown();

private:
    Window window;
    Camera camera;

    // квад-подложка (этап 1)
    Shader quadShader;
    GLuint quadVao = 0, quadVbo = 0, quadEbo = 0, quadTex = 0;
    bool initQuad(const char* pngPath);
    void drawQuad(float aspect);

    // 3D-объект (этап 2): идол из GLB
    Shader meshShader;
    Mesh idolMesh;
    GLuint idolTex = 0;   // рядом с Mesh idolMesh;
    bool loadIdol(const char* glbPath);
    void drawIdol(float aspect);

    bool running = true;
};