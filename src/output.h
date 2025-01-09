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
            SDL_Window* handle;
            SDL_Renderer* renderer;
            SDL_Texture* frame;
    public:
            raster framebuffer;
            window(const char* title, uint x, uint y, uint width, uint height);
            void write_frame();
            ~window();
        };
    };
};
