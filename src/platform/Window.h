#pragma once
#include <SDL3/SDL.h>

struct Window {
    SDL_Window*   handle = nullptr;
    SDL_GLContext gl     = nullptr;
    int width  = 1280;
    int height = 720;

    bool init(const char* title, int w, int h);
    void shutdown();
    void swap();                       // endFrame
    void pollSize();                   // обновляет width/height при ресайзе
};