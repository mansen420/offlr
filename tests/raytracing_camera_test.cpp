#include "format.h"
#include "raytracing/camera.h"
#include "output.h"
#include "raytracing/material.h"
#include "raytracing/ray.h"
#include "timer.h"
#include "utils.h"
#include "raytracing/geometry.h"
#include "raytracing/renderer.h"
#include "raytracing/tracer.h"

#include <SDL_events.h>
#include <SDL_video.h>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>

int main([[maybe_unused]]int argc, [[maybe_unused]]char** argv)
{
    using namespace AiCo;
    using namespace RT;

    output::init();
    int width = std::stoi(argv[1]), height = std::stoi(argv[2]);

    camera cam(2.f, width, height);
    output::window WND("samples", 0, 0, width, height);
    output::window WNDR("render", width, height, width, height);
    

    sphere smallBall(0.5f, {0.2f, 0.5f, -2.f});
    sphere bigBall(20.f, {0.0f, -20.5f, -2.f});
    
    auto balls_intersector = [&smallBall, &bigBall] (const ray& R, interval K)
    {
        return nearest_hit_structure({smallBall, bigBall})(R, K);
    };


    lambertian_diffuse mat({0.5f, 0.5f, 0.55f});
    renderer R(cam, 2, simple_tracer(&mat, balls_intersector, 100));

    micro_timer globalTimer;

    R(WNDR.framebuffer);
    WNDR.write_frame();
    std::printf("Fin. %fms", globalTimer.clock().count()/1000.f );
    R.samplesPerPixel = 10;

    bool quit = false;
    unsigned int count = 0;
    while(!quit)
    {
        SDL_Event e;
        while(SDL_PollEvent(&e))
            if(e.type == SDL_QUIT || e.window.event == SDL_WINDOWEVENT_CLOSE)
                quit = true;

        micro_timer frameTimer;

        R(WND.framebuffer);

        WNDR.write_frame();
        WND.write_frame();
       
        std::printf("frame idx : %d\ttime : %0.2fms\n", count++, frameTimer.clock().count()/1000.f );

        smallBall.radius = 0.5 * cos(0.7 * double(globalTimer.time_since_start().count())/1e+6) + 0.5;
    }
    output::terminate();
    return 0;
}
