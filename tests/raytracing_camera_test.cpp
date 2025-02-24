#include "format.h"
#include "raytracing/camera.h"
#include "output.h"
#include "raytracing/intersection.h"
#include "raytracing/material.h"
#include "timer.h"
#include "raytracing/geometry.h"
#include "raytracing/renderer.h"
#include "raytracing/tracer.h"
#include "raytracing/pipeline.h"
#include "registry.h"
#include "metrics.h"

#include <SDL_events.h>
#include <SDL_video.h>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <functional>
#include <optional>
#include <string>

int main([[maybe_unused]]int argc, [[maybe_unused]]char** argv)
{
    using namespace AiCo;
    using namespace RT;

    output::init();
    int width = std::stoi(argv[1]), height = std::stoi(argv[2]);

    output::window WND("samples", 0, 0, width, height);
    output::window WNDR("render", width, height, width, height);
    
    registry<material_t> mat_registry;
    auto METAL = mat_registry.add(new material_t{.scatter = metallic(), 
    .texture =[](const intersection_t&){return color3f{0.8f, 0.8f, 0.8f};}});
    
    auto DIFFUSE = mat_registry.add(new material_t{.scatter = lambertian_diffuse(), 
    .texture = [](const intersection_t&){return color3f{0.5f, 0.5f, 0.5f};}});

    auto EMIT = mat_registry.add(new material_t{.scatter=[](const intersection_t&){return std::nullopt;}, 
    .texture = [](const intersection_t&){return color3f{1.0f, 0.95f, 0.95f};}});

    sphere smallBall(0.5f, {0.0f, 0.5f, -2.5f}, mat_registry[METAL]);
    sphere bigBall(20.f, {0.0f, -20.5f, -2.f}, mat_registry[DIFFUSE]);
    sphere rightBall(1.f, {2.f, 0.0f, -4.5f}, mat_registry[DIFFUSE]);
    sphere leftBall(1.f, {0.f, 0.2f, -1.5f}, mat_registry[DIFFUSE]);

    std::vector<intersector_t> scene = {std::ref(smallBall), std::ref(bigBall), std::ref(rightBall), 
    std::ref(leftBall), sphere(0.5f, {0.5f, 0.5f, -3.f}, mat_registry[DIFFUSE]),
    sphere(0.5f, {-0.5f, 0.f, -5.f}, mat_registry[DIFFUSE]),
    sphere(0.1f, {1.5f, 0.3f, -1.5f}, mat_registry[METAL])};
    
    renderer R
    (
    5,
    simple_pipeline
        (
        [&scene](ray R, interval K)
            {return nearest_intersect(scene)(R, K);}, 
        unbiased_tracer(10, {0.001f, 10.f}),
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
       
        //std::printf("frame idx : %d\ttime : %0.2fms\n", count++, frameTimer.clock().count()/1000.f );
        
        auto inscts = INSCT_CNTR.load(std::memory_order_relaxed);
        auto engtime = float(globalTimer.time_since_start().count())/1000.f;
        
        std::printf("ENGINE %0.2fms\nFRAME %0.2fms\nINSCT %lu\nINSCT/sec %0.1fM/s\n", 
        engtime,
        float(frameTimer.clock().count())/1000.f, 
        inscts, 
        double(inscts/1e+3)/engtime);
        
        if(!quit)
            std::printf("\033[F\033[F\033[F\033[F");
        
        smallBall.radius = map(cos(0.7 * double(globalTimer.time_since_start().count())/1e+6), {-1.f, 1.f}, 
        {1.2f, 1.8f});
        smallBall.center.x = cos(0.2f * 0.7 * double(globalTimer.time_since_start().count())/1e+6 + PI/2.f);
        smallBall.center.y = cos(1.2f * 0.7 * double(globalTimer.time_since_start().count())/1e+6);
    }
    output::terminate();
    return 0;
}
