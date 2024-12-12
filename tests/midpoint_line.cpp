#include "glm/detail/qualifier.hpp"
#include "glm/fwd.hpp"
#include "output.h"
#include  "rasterizer.h"
#include "timer.h"

#include <cmath>
#include <cstdint>
#include <iostream>
#include <string>
#include <sys/types.h>

int main([[maybe_unused]]int argc, [[maybe_unused]]char** argv)
{
    using namespace AiCo;

    output::init();
    
    uint w = 1280, h = 720;

    if(argc > 1)
        w = std::stoi(argv[1]), h = std::stoi(argv[2]);

    output::window WND("title", 0, 0, w, h);

    uint slices = 360;
    if(argc >= 4)
        slices = std::stoi(argv[3]);

    rasterizer R(WND.width, WND.height, {0.f, 1.f}, {0.f, 1.f}, {0, 1.f}, WND.framebuffer);
    R.clear({255, 255, 255, 255}); 
   
    typedef glm::vec<2, int> vec2i;
    RGBA32 black = {0,0,0,255};
    vec2i center {R.rasterWidth/2, R.rasterHeight/2};
    
    R.draw_line_midpoint_scr({0,0}, {R.rasterWidth, R.rasterHeight}, black);

    R.draw_line_midpoint_scr(center, center + vec2i{0, center.y/2}, black);
    R.draw_line_midpoint_scr(center, center + vec2i{center.x/2, 0}, black);
    R.draw_line_midpoint_scr(center, center + vec2i{center.x/2, center.y/2}, black);
    R.draw_line_midpoint_scr(center, center + vec2i{0, -center.y/2}, black);
    R.draw_line_midpoint_scr(center, center + vec2i{-center.x/2, 0}, black);
    R.draw_line_midpoint_scr(center, center + vec2i{-center.x/2, center.y/2}, black);
    R.draw_line_midpoint_scr(center, center + vec2i{center.x/2, -center.y/2}, black);
    R.draw_line_midpoint_scr(center, center + vec2i{-center.x/2, -center.y/2}, black);

   bool quit = false;

    uint frameCount = 0;
    while(!quit)
    {
        SDL_Event e;
        while(SDL_PollEvent(&e))
            if(e.type == SDL_QUIT)
                quit = true;
        
        ms_timer frameTimer;
        
        float frameRatio = float(frameCount)/slices * 2*3.14;
        glm::vec3 worldCenter = {0.5, 0.5, 0.5};
        R.draw_line_midpoint_world(worldCenter, worldCenter + 0.5f*glm::vec3{0.5*cos(frameRatio), 0.5*sin(frameRatio), 0},{uint8_t((0.5 * sin(frameRatio) + 0.5) * 255), 0, 0, 255});

        WND.write_frame();
        
        R.clear({255, 255, 255, 255});

        std::cout << frameTimer.clock().count() << " ms" << std::endl;

        if(frameCount > slices)
            frameCount = 0;
        frameCount++;
    }
    output::terminate();
    return 0;
}
