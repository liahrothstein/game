#include "game/Game.h"
#include "stb_image.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

static const char* MESH_VS = R"(#version 330 core
    layout(location=0) in vec3 aPos;
    layout(location=1) in vec3 aNormal;
    layout(location=2) in vec2 aUV;
    uniform mat4 uProj, uView, uModel;
    out vec3 vNormal;
    out vec2 vUV;
    void main() {
        vNormal = normalize(mat3(uModel) * aNormal);
        vUV = aUV;
        gl_Position = uProj * uView * uModel * vec4(aPos, 1.0);
    })";

static const char* MESH_FS = R"(#version 330 core
    in vec3 vNormal;
    in vec2 vUV;
    uniform sampler2D uTex;
    out vec4 FragColor;
    void main() {
        vec3 lightDir = normalize(vec3(0.5, 1.0, 0.6));
        float lambert = max(dot(normalize(vNormal), lightDir), 0.0);
        vec3 base = texture(uTex, vUV).rgb;          // цвет из текстуры!
        FragColor = vec4(base * (0.35 + 0.65 * lambert), 1.0);
    })";

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

static GLuint makeAshTexture() {
    const int S = 512;
    std::vector<unsigned char> px(S * S * 3);
    srand(42);
    for (int i = 0; i < S * S; ++i) {
        unsigned char g = 40 + rand() % 26;
        px[i*3+0] = g; px[i*3+1] = g; px[i*3+2] = g;
    }
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, S, S, 0, GL_RGB, GL_UNSIGNED_BYTE, px.data());
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    return tex;
}

static Mesh makeGround() {
    std::vector<Vertex> verts;
    glm::vec3 p[4] = { {-20,0,-20}, {20,0,-20}, {20,0,20}, {-20,0,20} };
    glm::vec2 u[4] = { {0,8}, {8,8}, {8,0}, {0,0} };
    for (int i = 0; i < 4; ++i)
        verts.push_back({ p[i], {0,1,0}, u[i] });
    std::vector<GLuint> idx = { 0,1,2, 0,2,3 };
    Mesh m;
    m.build(verts, idx);
    return m;
}

bool Game::init() {
    if (!window.init("Kosti i Pepel — dev", 1280, 720)) return false;

    glEnable(GL_DEPTH_TEST);                    // КРИТИЧНО для 3D

    if (!initQuad(R"(D:\Development\game\assets\test.png)")) return false;
    initGround();
    return loadIdol(R"(D:\Development\game\assets\idol_hooded_r1.glb)");
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

bool Game::loadIdol(const char* glbPath) {
    meshShader.build(MESH_VS, MESH_FS);
    if (!GltfLoader::loadFirstMesh(glbPath, idolMesh)) return false;

    std::vector<unsigned char> pixels;
    int w, h, n;
    if (!GltfLoader::loadFirstImage(glbPath, pixels)) {
        std::cerr << "gltf: embedded image not found\n";
        return false;
    }
    stbi_set_flip_vertically_on_load(true);
    unsigned char* decoded = stbi_load_from_memory(
        pixels.data(), (int)pixels.size(), &w, &h, &n, 0);
    if (!decoded) { std::cerr << "stbi decode failed\n"; return false; }

    glGenTextures(1, &idolTex);
    glBindTexture(GL_TEXTURE_2D, idolTex);
    GLenum fmt = (n == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, decoded);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(decoded);
    return true;
}

void Game::drawQuad(float aspect) {
    quadShader.use();
    quadShader.setMat4("uProj",  camera.proj(aspect));
    quadShader.setMat4("uView",  camera.view());
    quadShader.setMat4("uModel", glm::mat4(1.0f));
    glBindTexture(GL_TEXTURE_2D, quadTex);
    glBindVertexArray(quadVao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

void Game::drawIdol(float aspect) {
    meshShader.use();
    meshShader.setMat4("uProj",  camera.proj(aspect));
    meshShader.setMat4("uView",  camera.view());

    // Конвейерный фикс ориентации: меши asset-forge (Blender Z-up)
    // в glTF лежат «на спине» — вариант 2 ставит их вертикально
    glm::mat4 m = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1,0,0));
    // Подъём из пола при необходимости (подберите число по глазу):
    // m = glm::translate(m, glm::vec3(0.0f, 0.2f, 0.0f));

    meshShader.setMat4("uModel", m);
    glBindTexture(GL_TEXTURE_2D, idolTex);
    idolMesh.draw();
}

bool Game::initGround() {
    groundShader.build(
        R"(#version 330 core
           layout(location=0) in vec3 aPos;
           layout(location=1) in vec3 aNormal;
           layout(location=2) in vec2 aUV;
           uniform mat4 uProj, uView, uModel;
           out vec2 vUV;
           void main() { vUV = aUV;
               gl_Position = uProj * uView * uModel * vec4(aPos, 1.0); })",
        R"(#version 330 core
           in vec2 vUV;
           uniform sampler2D uTex;
           out vec4 FragColor;
           void main() { FragColor = texture(uTex, vUV); })");
    groundTex = makeAshTexture();
    groundMesh = makeGround();
    return true;
}

void Game::drawGround(float aspect) {
    groundShader.use();
    groundShader.setMat4("uProj",  camera.proj(aspect));
    groundShader.setMat4("uView",  camera.view());
    groundShader.setMat4("uModel", glm::mat4(1.0f));
    glBindTexture(GL_TEXTURE_2D, groundTex);
    groundMesh.draw();
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
        const bool* keys = SDL_GetKeyboardState(nullptr);
        static int orientMode = 4;
        if (keys[SDL_SCANCODE_1]) orientMode = 1;
        if (keys[SDL_SCANCODE_2]) orientMode = 2;
        if (keys[SDL_SCANCODE_3]) orientMode = 3;
        if (keys[SDL_SCANCODE_4]) orientMode = 4;

        window.pollSize();
        float aspect = (float)window.width / (float)window.height;

        glViewport(0, 0, window.width, window.height);
        glClearColor(0.05f, 0.05f, 0.06f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);   // +depth!
        drawGround(aspect);

        drawQuad(aspect);
        drawIdol(aspect);     // идол стоит в центре (0,0,0) — квад на полу под ним

        window.swap();
    }
}

void Game::shutdown() {
    idolMesh.shutdown();
}