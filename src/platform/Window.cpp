#include "platform/Window.h"
#include <glad/glad.h>
#include <iostream>

bool Window::init(const char* title, int w, int h) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init: " << SDL_GetError() << "\n";
        return false;
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    width = w; height = h;
    handle = SDL_CreateWindow(title, w, h,
                              SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!handle) { std::cerr << "Window: " << SDL_GetError() << "\n"; return false; }

    gl = SDL_GL_CreateContext(handle);
    SDL_GL_MakeCurrent(handle, gl);
    SDL_GL_SetSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        std::cerr << "glad init failed\n";
        return false;
    }
    std::cout << "OpenGL: " << glGetString(GL_VERSION) << "\n";
    return true;
}

void Window::shutdown() {
    SDL_GL_DestroyContext(gl);
    SDL_DestroyWindow(handle);
    SDL_Quit();
}

void Window::swap() { SDL_GL_SwapWindow(handle); }

void Window::pollSize() {
    SDL_GetWindowSizeInPixels(handle, &width, &height);
}