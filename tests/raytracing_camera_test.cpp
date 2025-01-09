#include "format.h"
#include "glm/common.hpp"
#include "raytracing/camera.h"
#include "output.h"
#include "raytracing/ray.h"
#include "timer.h"
#include "raster.h"
#include "utils.h"
#include "raytracing/geometry.h"
#include "raytracing/renderer.h"

#include <SDL_events.h>
#include <SDL_video.h>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>

AiCo::color3f rayGradient(const AiCo::ray& sample);

int main([[maybe_unused]]int argc, [[maybe_unused]]char** argv)
{
    using namespace AiCo;

    output::init();
    int width = std::stoi(argv[1]), height = std::stoi(argv[2]);

    camera cam(2.f, width, height);
    output::window WND("samples", 0, 0, width, height);
    output::window WNDR("render", width, height, width, height);

    nearest_hit_structure balls;
    std::shared_ptr<sphere> smallBall = std::make_shared<sphere>(0.5f, glm::vec3{0.2f, 0.5f, -2.f});
    balls.list.push_back(smallBall);
    balls.list.push_back(std::make_shared<sphere>(sphere(20.f, {0.0f, -20.5f, -2.f})));
    
    auto trace = [&](const ray& sampler) -> color3f
                {
                    if(balls.intersects(sampler, 0.f, 10.f))
                        return 0.5f * balls.lastIntersect.N + glm::vec3(0.5);
                    else
                        return rayGradient(sampler);
                };
    int recursionDepth = 0, maxDepth = 10;
    std::function<color3f(const ray&)> trace_diffuse = [&](const ray& sampler) -> color3f
                {
                    recursionDepth++;
                    if(recursionDepth >= maxDepth)
                    {
                        recursionDepth = 0;
                        return {0.f, 0.f, 0.f};
                    }
                    if(balls.intersects(sampler, 0.0001f, 10.f))
                        return 0.5f * trace_diffuse(randvec_on_hemisphere(balls.lastIntersect.N));
                    else
                        return rayGradient(sampler);
                };
    renderer R(cam, 50, trace_diffuse);
    
    R(WNDR.framebuffer);
    
    micro_timer globalTimer;

    bool quit = false;
    while(!quit)
    {
        SDL_Event e;
        while(SDL_PollEvent(&e))
            if(e.type == SDL_QUIT || e.window.event == SDL_WINDOWEVENT_CLOSE)
                quit = true;

        micro_timer frameTimer;

        R.sample(WND.framebuffer);


        WNDR.write_frame();
        WND.write_frame();

        std::cout << frameTimer.clock().count()/1000.f << " ms" << std::endl;

        smallBall->radius = 0.5 * cos(0.7 * double(globalTimer.time_since_start().count())/1e+6) + 0.5;
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
