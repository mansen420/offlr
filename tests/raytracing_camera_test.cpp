#include "raytracing/camera.h"
#include "output.h"
#include "raytracing/material.h"
#include "timer.h"
#include "raytracing/geometry.h"
#include "raytracing/renderer.h"
#include "raytracing/tracer.h"
#include "raytracing/pipeline.h"


#include <SDL_events.h>
#include <SDL_video.h>
#include <cmath>
#include <cstdio>
#include <string>

int main([[maybe_unused]]int argc, [[maybe_unused]]char** argv)
{
    using namespace AiCo;
    using namespace RT;

    output::init();
    int width = std::stoi(argv[1]), height = std::stoi(argv[2]);

    output::window WND("samples", 0, 0, width, height);
    output::window WNDR("render", width, height, width, height);
    

    sphere smallBall(0.5f, {0.0f, 0.5f, -2.5f});
    sphere bigBall(20.f, {0.0f, -20.5f, -2.f});
    
    std::vector<intersector_t> scene = {std::ref(smallBall), std::ref(bigBall)};
    
    renderer R
    (
    10,
    simple_pipeline
        (
        [&scene](ray R, interval K)
            {return nearest_intersect(scene)(R, K);}, 
        simple_tracer(lambertian_diffuse(), 10, {0.001f, 10.f}),
        vFOV_camera(40.f, width, height, {-2.f, -2.f , -2.5f}, 0.2f,
            {3.f, 2.f, -1.f})
        )
    );

    micro_timer globalTimer;

    R(WNDR.framebuffer);
    WNDR.write_frame();

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
        smallBall.center.x = cos(2.f * 0.7 * double(globalTimer.time_since_start().count())/1e+6 + PI/2.f);
    }
    output::terminate();
    return 0;
}
