#include "output.h"
#include  "rasterizer.h"
#include "timer.h"

#include <iostream>
#include <string>

int main([[maybe_unused]]int argc, [[maybe_unused]]char** argv)
{
    output::init();
    rasterizer R(std::stoi(argv[1]), std::stoi(argv[2]), {0.f, 1.f}, {0.f, 1.f});
    output::window WND("title", 0, 0, 1280, 720);
 
    bool quit = false;
    while(!quit)
    {
        SDL_Event e;
        while(SDL_PollEvent(&e))
            if(e.type == SDL_QUIT)
                quit = true;
        
        ms_timer frameTimer;
        
        R.draw_triangle_scr({0, 0}, {50, 0}, {0, 25}, {{1.f, 0.2f, 0.2f}, {0.2f, 1.f, 0.2f}, {0.2f, 0.2f, 1.f}});
        R.draw_triangle_scr({0, 0}, {25, 0}, {0, 50}, {{1.f, 0.2f, 0.2f}, {0.2f, 1.f, 0.2f}, {0.2f, 0.2f, 1.f}});
        R.sample_raster(WND.height, WND.width, WND.framebuffer); 
        WND.write_frame();

        std::cout << frameTimer.clock().count() << " ms" << std::endl;
    }
    output::terminate();
    return 0;
}
