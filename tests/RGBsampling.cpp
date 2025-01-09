#include "output.h"
#include "rasterizer.h"
#include "timer.h"

#include <iostream>
#include <string>

int main([[maybe_unused]]int argc, [[maybe_unused]]char** argv)
{
    using namespace AiCo;

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
        
        micro_timer frameTimer;

        R.RGB_test();
        R.sample_raster(WND.framebuffer.height, WND.framebuffer.width, WND.framebuffer.data); 
        WND.write_frame();

        std::cout << frameTimer.clock().count()/1000.f << " ms" << std::endl;
    }
    output::terminate();
    return 0;
}
