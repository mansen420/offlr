#include "format.h"
#include "raytracing/camera.h"
#include "output.h"
#include "raytracing/ray.h"
#include "timer.h"
#include "raster.h"
#include "utils.h"
#include "raytracing/geometry.h"
#include "raytracing/renderer.h"

#include <cstddef>
#include <iostream>
#include <memory>
#include <string>

AiCo::color3f rayGradient(const AiCo::ray& sample);

AiCo::nearest_hit_structure balls;
int main([[maybe_unused]]int argc, [[maybe_unused]]char** argv)
{
    using namespace AiCo;

    output::init();
    int width = std::stoi(argv[1]), height = std::stoi(argv[2]);

    camera cam(2.f, width, height);
    output::window WND("title", 0, 0, width, height);
    
    raster framebuffer(WND.framebuffer, WND.width, WND.height);
        
    sphere ball(0.5, {0.2f, 0.5f, -2.f});
    balls.list.push_back(std::make_shared<sphere>(sphere(0.5f, {0.2f, 0.5f, -2.f})));
    balls.list.push_back(std::make_shared<sphere>(sphere(20.f, {0.0f, -20.5f, -2.f})));
    
    color3f (* trace) (const ray& sampler) = [](const ray& sampler) -> color3f
                {
                    if(balls.intersects(sampler, 0.f, 10.f))
                        return 0.5f * balls.lastIntersect.N + glm::vec3(0.5);
                    else
                        return rayGradient(sampler);
                };
    renderer R(cam, 1, WND.width, WND.height, trace, WND.framebuffer);
    bool quit = false;
    while(!quit)
    {
        SDL_Event e;
        while(SDL_PollEvent(&e))
            if(e.type == SDL_QUIT)
                quit = true;
        
        ms_timer frameTimer;

        R.render();

        WND.write_frame();

        std::cout << frameTimer.clock().count() << " ms" << std::endl;
    }
    output::terminate();
    return 0;
}

AiCo::color3f rayGradient(const AiCo::ray& sample)
{
    AiCo::color3f blue = {0.25, 0.4, 0.8};
    AiCo::color3f white = {1, 1, 1};
    return AiCo::lerp(0.5 * sample.dir.y + 0.5, blue, white);
}
