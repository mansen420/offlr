#include "format.h"
#include "glm/fwd.hpp"
#include "interval.h"
#include "raytracing/camera.h"
#include "output.h"
#include "raytracing/intersection.h"
#include "raytracing/material.h"
#include "timer2.h"
#include "raytracing/geometry.h"
#include "raytracing/renderer.h"
#include "raytracing/tracer.h"
#include "raytracing/pipeline.h"
#include "registry.h"
#include "metrics.h"
#include "materials.h"
#include "scenes.h"
#include "save_bmp.h"

#include <SDL_events.h>
#include <SDL_video.h>
#include <asm-generic/errno.h>
#include <atomic>
#include <cstdio>
#include <functional>
#include <string>
#include <vector>

int main([[maybe_unused]]int argc, [[maybe_unused]]char** argv)
{
    using namespace AiCo;
    using namespace RT;

    output::init();
    int width = std::stoi(argv[1]), height = std::stoi(argv[2]), spp = std::stoi(argv[3]);

    output::window WNDR("render", width, height, width, height);
    
    init_scenes();
    
    renderer R
    (
    /*samplesPerPixel=*/spp,
    /*pipeline=*/simple_pipeline
        (
        /*sceneInsctr=*/[](ray R, interval K)
            {return nearest_intersect(spaceDuneScene)(R, K);}, 
        /*tracer=*/direct_recursive_tracer(/*maxDepth=*/100, /*rayBounds=*/{0.001f, INF}, spaceDuneLights, 
                        color3f(0.0f, 0.0f, 0.0f), color3f(0.f)),
        /*view=*/vFOV_camera(5.f, width, height, {-0.5f, 0.f, -1.f}, 0.2f,
            {0.f, 2.0f, 3.5f})
        )
    );

    micro_timer globalTimer;

    R(WNDR.framebuffer);

    auto inscts = INSCT_CNTR.load(std::memory_order_relaxed);
    auto engtime = float(globalTimer.time_since_start().count())/1000.f;
    
    std::printf("ENGINE %0.2fms\nINSCT %lu\nINSCT/sec %0.1fM/s\n", 
    engtime,
    inscts, 
    double(inscts/1e+3)/engtime);
    
    WNDR.write_frame();

    bool quit = false;
    do
    {
        SDL_Event e;
        while (SDL_PollEvent(&e)) 
            if (e.type == SDL_QUIT)
                quit = true;
            else if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE)
                quit = true;

        micro_timer frameTimer;

        WNDR.write_frame();
       
        inscts = INSCT_CNTR.load(std::memory_order_relaxed);
        engtime = float(globalTimer.time_since_start().count())/1000.f;
        
        std::printf("ENGINE %0.2fms\nFRAME %0.2fms\nINSCT %lu\nINSCT/sec %0.1fM/s\n", 
        engtime,
        float(frameTimer.tick().count())/1000.f, 
        inscts, 
        double(inscts/1e+3)/engtime);
        
        if(!quit)
            std::printf("\033[F\033[F\033[F\033[F");
    }while(false);
    SaveWindowToPNG(WNDR.handle, "./exp");
    output::terminate();
    return 0;
}
