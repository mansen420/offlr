#pragma once

#include "glm/glm.hpp"

#include "SDL.h"
#include <cstddef>
#include <stdexcept>

namespace output
{
    typedef glm::vec<4, glm::uint8> RGBA32;

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
