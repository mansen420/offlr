#include "output.h"
#include  "rasterizer.h"
#include "timer.h"

#include <cmath>
#include <cstdint>
#include <iostream>
#include <string>

int main([[maybe_unused]]int argc, [[maybe_unused]]char** argv)
{
    output::init();
    output::window WND("title", 0, 0, 1280, 720);
    
    uint w = 20, h = 20;
    if(argc > 1)
        w = std::stoi(argv[1]), h = std::stoi(argv[2]);

    uint slices = 360;
    if(argc >= 4)
        slices = std::stoi(argv[3]);

    rasterizer R(w, h, {0.f, 1.f}, {0.f, 1.f});
    
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
        R.draw_line_midpoint_world({0.5, 0.5}, {(0.5 * cos(frameRatio)+0.5), (0.5 * sin(frameRatio)+0.5)},
        {uint8_t((0.5 * sin(frameRatio) + 0.5) * 255), 0, 0, 255});

        R.sample_raster(WND.height, WND.width, WND.framebuffer); 
        WND.write_frame();

        std::cout << frameTimer.clock().count() << " ms" << std::endl;

        if(frameCount > slices)
            frameCount = 0;
        frameCount++;
    }
    output::terminate();
    return 0;
}
