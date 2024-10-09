#pragma once

#include "glm/fwd.hpp"
#include "output.h"
#include "glm/glm.hpp"
#include <sys/types.h>

class rasterizer
{ 
    output::RGBA32* raster;
    glm::mat3 WtoSCR;
    bool ownsRaster = true;
public:
    uint rasterWidth, rasterHeight;
    rasterizer(uint rasterWidth, uint rasterHeight, glm::vec2 XworldCoords, glm::vec2 YworldCoords, output::RGBA32* rasterPtr = nullptr);

    void draw_point(glm::vec2 worldCoord, output::RGBA32 color);
    void draw_line_midpoint_scr(glm::vec<2, int> ScrP1, glm::vec<2, int> ScrP2, output::RGBA32 color);
    void draw_line_midpoint_world(glm::vec2 worldP1, glm::vec2 worldP2, output::RGBA32 color = {255, 255, 255, 255});
    void sample_raster(uint sampleHeight, uint sampleWidth, output::RGBA32* sample);
    void clear(output::RGBA32 color);
    void draw_triangle_scr(glm::vec<2, int> a, glm::vec<2, int> b, glm::vec<2, int> c, glm::vec<3, glm::vec3> per_vertex_color);
    void RGB_test();

    ~rasterizer();
};
