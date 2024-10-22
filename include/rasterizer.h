#pragma once

#include "glm/ext/matrix_transform.hpp"
#include "output.h"
#include "glm/glm.hpp"

class rasterizer
{ 
    output::RGBA32* raster;
    /**
     * @brief Transforms from the canonical view volume [-1, 1]^3 to the screen coordinates [0, rasterWidth]x[0, rasterHeight], keeping the z coordinates unchanged.
     */
    glm::mat4 canonicalToSCR;
    /**
     * @brief Transforms from an arbitrary view volume --specified at construction time-- to the canonical view volume.
     */
    glm::mat4 volumeToCanonical;
    /**
     * @brief Transforms coordinates from xyz space to camera space
     */
    glm::mat4 cameraTransform;

    bool ownsRaster = true;
public:
    uint rasterWidth, rasterHeight;
    rasterizer(uint rasterWidth, uint rasterHeight, glm::vec2 XworldCoords = {-1, 1}, glm::vec2 YworldCoords = {-1, 1}, glm::vec2 ZworldCoords = {-1, 1}, output::RGBA32* rasterPtr = nullptr);

    void draw_point(glm::vec3 worldCoord, output::RGBA32 color);
    void draw_line_midpoint_scr(glm::vec<2, int> ScrP1, glm::vec<2, int> ScrP2, output::RGBA32 color);
    void draw_line_midpoint_world(glm::vec3 worldP1, glm::vec3 worldP2, output::RGBA32 color = {255, 255, 255, 255});
    void sample_raster(uint sampleHeight, uint sampleWidth, output::RGBA32* sample);
    void clear(output::RGBA32 color);
    void draw_triangle_scr(glm::vec<2, int> a, glm::vec<2, int> b, glm::vec<2, int> c, glm::vec<3, glm::vec3> per_vertex_color);
    void RGB_test();
    
    glm::vec<2, int> toSCR(glm::vec3 u);

    void set_camera_transform(glm::vec3 origin, glm::vec3 view_direction, glm::vec3 up_direction);

    ~rasterizer();
};
