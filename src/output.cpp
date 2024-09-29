#include <stdexcept>
#include "SDL.h"
#include "output.h"

void output::init()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        throw std::runtime_error("Failed to init SDL");
}
