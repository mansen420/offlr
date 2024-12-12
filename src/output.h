#pragma once

#include "format.h"
#include "SDL.h"

namespace AiCo
{
    namespace output
    {

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
};
