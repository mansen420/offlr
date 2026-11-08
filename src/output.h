#pragma once

#include "SDL.h"
#include "raster.h"

namespace AiCo
{
    namespace output
    {
        void init();
        void terminate();
        class window
        {
            SDL_Renderer* renderer;
            SDL_Texture* frame;
    public:
            SDL_Window* handle;
            raster framebuffer;
            window(const char* title, uint x, uint y, uint width, uint height);
            void write_frame();
            ~window();
        };
    };
};
