#pragma once

#include "glm/fwd.hpp"
#include "output.h"
#include "glm/glm.hpp"
#include <sys/types.h>

class rasterizer
{ 
    output::RGBA32* raster;
    glm::mat3 WtoSCR;
public:
    uint rasterWidth, rasterHeight;
    rasterizer(uint rasterWidth, uint rasterHeight, glm::vec2 XworldCoords, glm::vec2 YworldCoords);
    
    void draw_point(glm::vec2 worldCoord, output::RGBA32 color);
    void draw_line_midpoint_scr(glm::vec<2, uint> ScrP1, glm::vec<2, uint> ScrP2, output::RGBA32 color);
    void draw_line_midpoint_world(glm::vec2 worldP1, glm::vec2 worldP2, output::RGBA32 color = {255, 255, 255, 255});
    void sample_raster(uint sampleHeight, uint sampleWidth, output::RGBA32* sample);
    
    void RGB_test();

    ~rasterizer();
};
