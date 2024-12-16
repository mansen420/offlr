#include "format.h"
#include "raytracing/camera.h"
#include "output.h"
#include "timer.h"
#include "raster.h"
#include "utils.h"
#include "raytracing/geometry.h"

#include <cstddef>
#include <iostream>
#include <string>

AiCo::color3f rayGradient(AiCo::ray sample);

int main([[maybe_unused]]int argc, [[maybe_unused]]char** argv)
{
    using namespace AiCo;

    output::init();
    int width = std::stoi(argv[1]), height = std::stoi(argv[2]);

    camera cam(1.f, width, height);
    output::window WND("title", 0, 0, width, height);
    
    raster framebuffer(WND.framebuffer, WND.width, WND.height);
        
    sphere ball(1.0, {0.0f, 0.0f, 2.f});

    bool quit = false;
    while(!quit)
    {
        SDL_Event e;
        while(SDL_PollEvent(&e))
            if(e.type == SDL_QUIT)
                quit = true;
        
        ms_timer frameTimer;
        
        for(size_t i = 0; i < width; ++i)
            for(size_t j = 0; j < height; ++j)
            {
                ray sampler = cam.samplePixel(i, j);
                if(ball.intersects(sampler, 0.f, 10.f))
                {
                    auto normal = 0.5f * ball.lastIntersect.N + glm::vec3(0.5);
                    framebuffer.at(i, j) = colorftoRGBA32(normal);
                }
                else
                    framebuffer.at(i, j) = colorftoRGBA32(rayGradient(sampler));
            }
        WND.write_frame();

        std::cout << frameTimer.clock().count() << " ms" << std::endl;
    }
    output::terminate();
    return 0;
}

AiCo::color3f rayGradient(AiCo::ray sample)
{
    AiCo::color3f blue = {0.25, 0.4, 0.8};
    AiCo::color3f white = {1, 1, 1};
    return AiCo::lerp(0.5 * sample.dir.y + 0.5, blue, white);
}
