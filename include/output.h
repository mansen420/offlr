#pragma once

#include "SDL.h"
#include <cstddef>
#include <stdexcept>

namespace output
{
    struct RGBA32
    {
        uint8_t R;
        uint8_t G;
        uint8_t B;
        uint8_t A;
    };
    void init();
    void terminate();
    class window
    {
        SDL_Window* handle;
        SDL_Renderer* renderer;
        SDL_Texture* frame;
public:
        uint height, width;
        RGBA32* framebuffer;
        window(const char* title, uint x, uint y, uint width, uint height);
        void write_frame();
        ~window();
    };
};
