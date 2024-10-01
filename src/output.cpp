#include <SDL_pixels.h>
#include <SDL_render.h>
#include <SDL_video.h>
#include <stdexcept>
#include "SDL.h"
#include "output.h"

void output::init()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        throw std::runtime_error("Failed to init SDL");
}
void output::terminate()
{
    SDL_Quit();
}
output::window::window(const char* title, uint x, uint y, uint width, uint height) : height(height), width(width)
{
    auto res = SDL_CreateWindowAndRenderer(width, height, 0, &handle, &renderer);
    if(res != 0)
        throw std::runtime_error("WINDOW ERROR");
    frame = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, width, height);
    framebuffer = new output::RGBA32[height * width];
}
void output::window::write_frame()
{
    void* texPtr;
    int texPitch;
    SDL_LockTexture(frame, nullptr, &texPtr, &texPitch);
    memcpy(texPtr, framebuffer, texPitch * height);
    SDL_UnlockTexture(frame);

    SDL_RenderClear(renderer);
    //SDL_Rect destination{0, 0, (int)nrCols, (int)nrRows};
    SDL_RenderCopy(renderer, frame, nullptr, nullptr);
    SDL_RenderPresent(renderer);
}
output::window::~window()
{
    SDL_DestroyTexture(frame);
    SDL_DestroyWindow(handle);
    delete [] framebuffer;
}
