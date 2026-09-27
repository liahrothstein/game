#include <glad/glad.h>   // ← ПЕРВЫМ: объявления GL-функций
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <iostream>

int main(int, char**) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init: " << SDL_GetError() << "\n";
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window* window = SDL_CreateWindow("Kosti i Pepel — dev",
        1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window) {
        std::cerr << "Window: " << SDL_GetError() << "\n";
        return 1;
    }
    SDL_GLContext gl = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl);
    SDL_GL_SetSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        std::cerr << "glad init failed\n";
        return 1;
    }
    std::cout << "OpenGL " << glGetString(GL_VERSION) << "\n";

    glm::vec2 pos{640, 360};
    bool running = true;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = false;
            if (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE)
                running = false;
        }

        const bool* keys = SDL_GetKeyboardState(nullptr);
        if (keys[SDL_SCANCODE_W]) pos.y -= 2;
        if (keys[SDL_SCANCODE_S]) pos.y += 2;
        if (keys[SDL_SCANCODE_A]) pos.x -= 2;
        if (keys[SDL_SCANCODE_D]) pos.x += 2;
        std::cout << "pos: " << pos.x << ", " << pos.y << "\n";

        glViewport(0, 0, 1280, 720);
        glClearColor(0.05f, 0.05f, 0.06f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DestroyContext(gl);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}