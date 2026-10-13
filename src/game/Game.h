#pragma once
#include "platform/Window.h"
#include "render/Camera.h"
#include "render/Shader.h"
#include "render/Mesh.h"
#include "assets/GltfLoader.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

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
    Shader groundShader;
    GLuint groundTex = 0;
    Mesh groundMesh;
    bool initGround();
    void drawGround(float aspect);
    bool loadIdol(const char* glbPath);
    void drawIdol(float aspect);

    // --- сцена ---
    struct SceneObject {
        Mesh mesh;
        GLuint tex = 0;
        glm::vec3 pos;
        float yawDeg = 0.0f;
    };
    std::vector<SceneObject> objects;

    // загружает объект и добавляет в сцену
    bool loadObject(const char* glbPath, glm::vec3 pos, float yawDeg);
    // отрисовывает всю сцену
    void drawScene(float aspect);

    bool running = true;
};