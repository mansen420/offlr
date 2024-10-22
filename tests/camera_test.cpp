#include "output.h"
#include "rasterizer.h"
#include "timer.h"

#include <iostream>
#include <string>

int main([[maybe_unused]]int argc, [[maybe_unused]]char** argv)
{
    output::init();
    rasterizer R(std::stoi(argv[1]), std::stoi(argv[2]), {0.f, 1.f}, {0.f, 1.f});
    output::window WND("title", 0, 0, 1280, 720);
    
    uint count  = 0;
    bool quit = false;
    while(!quit)
    {
        SDL_Event e;
        while(SDL_PollEvent(&e))
            if(e.type == SDL_QUIT)
                quit = true;
        
        ms_timer frameTimer;
        
        R.clear({255, 255, 255, 255});
        float yGaze = 0.5*sin(float(count++)/100)+0.5;
        R.set_camera_transform({0.0, 0, 0}, {0, yGaze, -1.f}, {0.f, 1.f, 0.f});

        std::cout << R.toSCR({0.5, 0.5, 0.5}).x << ' ' << R.toSCR({0.5, 0.5, 0.5}).y << '\n'
        << R.toSCR({0.5, 0.65, 0.5}).x << ' ' << R.toSCR({0.5, 0.65, 0.5}).y << '\n' 
        << R.toSCR({0.65f, 0.5f, 0.f}).x << ' ' << R.toSCR({0.65f, 0.5f, 0.f}).y << '\n'
        << std::endl;

        R.draw_triangle_scr(R.toSCR({0.5, 0.5, 0.5}), R.toSCR({0.5, 0.65, 0.5}), R.toSCR({0.65f, 0.5f, 0.f}),
        {{1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 0.f, 1.f}});

        R.sample_raster(WND.height, WND.width, WND.framebuffer); 
        WND.write_frame();

        std::cout << frameTimer.clock().count() << " ms" << std::endl;
    }
    output::terminate();
    return 0;
}
