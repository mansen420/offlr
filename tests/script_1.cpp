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

#include <SDL_events.h>
#include <SDL_video.h>
#include <asm-generic/errno.h>
#include <atomic>
#include <cstdio>
#include <functional>
#include <string>

int main([[maybe_unused]]int argc, [[maybe_unused]]char** argv)
{
    using namespace AiCo;
    using namespace RT;

    output::init();
    int width = std::stoi(argv[1]), height = std::stoi(argv[2]);

    output::window WNDR("render", width, height, width, height);
    
    registry<material_t> mat_registry;
    
    auto EMIT = mat_registry.add(new material_t
        {
            .scatter = emissive(),
            .texture = [](const intersection_t& isct) {return color3f{1.f, 1.f, 1.f};}
        });
    auto RADIAL = mat_registry.add(new material_t
    {
        .scatter = metallic({0.8f, 0.8f, 0.8f}, 0.35f),
        .texture = [](const intersection_t& isect) {
            float rings = sin(glm::length(isect.P) * 100.0f);
            if (rings > 0)
                return color3f{0.2f, 0.2f, 1.0f}; // blue
            else
                return color3f{1.0f, 1.0f, 1.0f}; // white
        }
    });
    auto GOLD_METAL = mat_registry.add(new material_t{
        .scatter = metallic({1.0f, 0.85f, 0.57f}, 0.05f), // gold tone, slightly rough
        .texture = [](const intersection_t& isect) {
                float scale = 6.0f; // controls check size in world space
                int xCheck = static_cast<int>(floor(isect.P.x * scale));
                int yCheck = static_cast<int>(floor(isect.P.y * scale));
                int zCheck = static_cast<int>(floor(isect.P.z * scale));

                // 3D checker: alternates along all axes
                if ((xCheck + yCheck + zCheck) % 2 == 0) {
                    return color3f{1.0f, 1.0f, 1.0f}; // white
                } else {
                    return color3f{0.2f, 0.2f, 0.2f}; // black
                }
            }
    });

    auto STEEL_METAL = mat_registry.add(new material_t{
        .scatter = metallic({0.8f, 0.8f, 0.8f}, 0.0f),
        .texture = [](const intersection_t&) { return color3f{0.8f, 0.8f, 0.8f}; }
    });

    auto MATTE_RED = mat_registry.add(new material_t{
        .scatter = lambertian_diffuse(glm::vec3(0.9f)),
        .texture = [](const intersection_t&) { return color3f{0.7f, 0.0f, 0.0f}; }
    });


    // Geometry
    sphere goldBall(0.65f, {0.0f, 0.75f, -3.0f}, mat_registry[RADIAL]); // centerpiece
    cylinder cyln(0.3f, 20.5f, {0.f, 0.0f, -3.0f}, mat_registry[GOLD_METAL]);
    sphere blueBall(0.3f, {-0.5f, 0.1f, -2.5f}, mat_registry[MATTE_RED]); // support
    sphere steelBall(0.3f, {0.5f, 0.1f, -2.5f}, mat_registry[STEEL_METAL]); // support
    plane ground({0.f, 1.f, 0.f}, {0.f, 0.f, 0.f}, mat_registry[GOLD_METAL]);
    // Scene
    std::vector<intersector_t> scene = {
        std::ref(cyln),
        std::ref(blueBall),
        std::ref(steelBall),
        std::ref(ground)
    };

    renderer R
    (
    /*samplesPerPixel=*/20,
    /*pipeline=*/simple_pipeline
        (
        /*sceneInsctr=*/[&scene](ray R, interval K)
            {return nearest_intersect(scene)(R, K);}, 
        /*tracer=*/unbiased_tracer(/*maxDepth=*/1000, /*rayBounds=*/{0.001f, INF}, 
                    /*topGradient=*/{0.5f, 0.2f, 0.1f}, {0.25, 0.4, 0.8}),
        /*view=*/vFOV_camera(25.f, width, height, {0.f, 0.0f, -1.f}, 0.2f,
            {0.f, 0.5f, 0.f})
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

    while(!quit)
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
    }
    output::terminate();
    return 0;
}
