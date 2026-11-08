#include <SDL2/SDL.h>
#include <ctime>
#include <string>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

bool SaveWindowToPNG(SDL_Window* window, const char* directory = ".")
{
    if (!window) {
        SDL_Log("SaveWindowToPNG: window is null");
        return false;
    }

    SDL_Renderer* renderer = SDL_GetRenderer(window);
    if (!renderer) {
        SDL_Log("SaveWindowToPNG: no renderer found");
        return false;
    }

    int width, height;
    SDL_GetRendererOutputSize(renderer, &width, &height);

    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_ABGR8888);
    if (!surface) {
        SDL_Log("SaveWindowToPNG: could not create surface: %s", SDL_GetError());
        return false;
    }

    if (SDL_RenderReadPixels(renderer, nullptr, SDL_PIXELFORMAT_ABGR8888, surface->pixels, surface->pitch) != 0) {
        SDL_Log("SaveWindowToPNG: failed to read pixels: %s", SDL_GetError());
        SDL_FreeSurface(surface);
        return false;
    }

    // Timestamp-based filename
    std::time_t now = std::time(nullptr);
    std::tm* tm_info = std::localtime(&now);
    char buffer[64];
    std::strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", tm_info);
    std::string path = std::string(directory) + "/frame_" + buffer + ".png";

    // Write PNG
    int success = stbi_write_png(path.c_str(), width, height, 4, surface->pixels, surface->pitch);
    SDL_FreeSurface(surface);

    if (!success) {
        SDL_Log("SaveWindowToPNG: failed to write PNG file");
        return false;
    }

    SDL_Log("Saved screenshot: %s", path.c_str());
    return true;
}
