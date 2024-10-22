#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "glm/glm.hpp"
#include "output.h"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <sys/types.h>
#include <tuple>
#include <utility>
#include "rasterizer.h"

rasterizer::rasterizer(uint rasterWidth, uint rasterHeight, glm::vec2 worldX, glm::vec2 worldY, glm::vec2 worldZ, output::RGBA32* rasterPtr) 
: raster(rasterPtr == nullptr ? new output::RGBA32[rasterWidth * rasterHeight] : rasterPtr), rasterWidth(rasterWidth), rasterHeight(rasterHeight), 
ownsRaster(rasterPtr == nullptr ? true : false)
{
    if(ownsRaster)
        std::fill(raster, raster + rasterHeight*rasterWidth, output::RGBA32{255, 255, 255, 255});

    glm::vec4 col4(float(rasterWidth - 1)/2.f, float(rasterHeight - 1)/2.f, 0.f, 1.f);
    glm::vec4 col3(0.f, 0.f, 1.f, 0.f);
    glm::vec4 col2(0.f, float(rasterHeight)/2.f, 0.f, 0.f);
    glm::vec4 col1(float(rasterWidth)/2.f, 0.f, 0.f, 0.f);

    canonicalToSCR = glm::mat4(col1, col2, col3, col4);
    
    float worldXSpan = worldX[1] - worldX[0];
    float worldYSpan = worldY[1] - worldY[0];
    float worldZSpan = worldZ[1] - worldZ[0];
    float worldXshift = (worldX[0] + worldX[1])/worldXSpan;
    float worldYshift = (worldY[0] + worldY[1])/worldYSpan;
    float worldZshift = (worldZ[0] + worldZ[1])/worldZSpan;
    
    col4 = glm::vec4(-worldXshift, -worldYshift, -worldZshift, 1.f);
    col3 = glm::vec4(0.f, 0.f, 2.f/worldZSpan, 0.f);
    col2 = glm::vec4(0.f, 2.f/worldYSpan, 0.f, 0.f);
    col1 = glm::vec4(2.f/worldXSpan, 0.f, 0.f, 0.f);
    
    volumeToCanonical = glm::mat4(col1, col2, col3, col4);

    cameraTransform = glm::mat4(1);
}
void rasterizer::set_camera_transform(glm::vec3 origin, glm::vec3 view, glm::vec3 up)
{
    glm::vec4 col4(-origin, 1.f);

    glm::vec3 w(glm::normalize(-view));
    
    glm::vec3 u = glm::cross(glm::normalize(up), w);

    glm::vec3 v = glm::cross(w, u);

    glm::vec4 col3(u.z, v.z, w.z, 0.f);
    glm::vec4 col2(u.y, v.y, w.y, 0.f);
    glm::vec4 col1(u.x, v.x, w.x, 0.f);

    cameraTransform = glm::mat4(col1, col2, col3, col4);
}
inline glm::vec3 homogenize(glm::vec4 u)
{
    assert(u.w != 0);
    return glm::vec3(u.x/u.w, u.y/u.w, u.z/u.w);
}

inline void rasterizer::draw_point(glm::vec3 worldCoords, output::RGBA32 color)
{
    glm::vec4 homogCoords(worldCoords, 1.f);
    glm::vec3 screenCoords = homogenize(canonicalToSCR * volumeToCanonical * homogCoords);
    raster[(uint)screenCoords.y*rasterWidth + (uint)screenCoords.x] = color;
}

void rasterizer::sample_raster(uint sampleHeight, uint sampleWidth, output::RGBA32* result)
{    
    for(size_t i = 0; i < sampleHeight; ++i)
        for(size_t j = 0; j < sampleWidth; ++j)
        {
            // sample pixel centers from raster
            constexpr float halfPixel = 0.5f;
            float vCenter = float(i + halfPixel)/sampleHeight; // [0, 1] vertical pixel center
            float hCenter = float(j + halfPixel)/sampleWidth; // [0, 1] horizontal pixel center
            
            uint vRasterPixel = uint(vCenter * rasterHeight);
            uint hRasterPixel = uint(hCenter * rasterWidth);

            result[i*sampleWidth + j] = raster[vRasterPixel*rasterWidth + hRasterPixel];
        }
}

glm::vec<2, int> rasterizer::toSCR(glm::vec3 u){return homogenize(canonicalToSCR*volumeToCanonical*cameraTransform*glm::vec4(u, 1.f));}

void rasterizer::draw_line_midpoint_world(glm::vec3 worldP1, glm::vec3 worldP2, output::RGBA32 color)
{
    glm::mat4 WtoSCR = canonicalToSCR*volumeToCanonical*cameraTransform;

    glm::vec<2, uint> p1(homogenize(WtoSCR * glm::vec4(worldP1, 1.f)));
    glm::vec<2, uint> p2(homogenize(WtoSCR * glm::vec4(worldP2, 1.f)));

    draw_line_midpoint_scr(p1,  p2, color);
}
void rasterizer::clear(output::RGBA32 color){ std::fill(raster, raster + rasterHeight*rasterWidth, color); }

void rasterizer::draw_line_midpoint_scr(glm::vec<2, int> P1, glm::vec<2, int> P2, output::RGBA32 color = {255, 255, 255, 255})
{   
    int dx = std::abs(P2.x - P1.x), dy = std::abs(P2.y - P1.y);
    
    bool steep = dy > dx;

    if(steep)
        std::swap(dy, dx);

    short int xStep = 1, yStep = 1;
    if(P1.x > P2.x)
        xStep = -1;
    if(P1.y > P2.y)
        yStep = -1;
        
    int D = 2 * dy - dx;
    int delatDprimary = 2 * dy;
    int deltaDsecondary = 2 * (dy - dx);

    uint x = P1.x;
    uint y = P1.y;
    
    // draw first point
    raster[y*rasterWidth + x] = color;
   
    for(size_t i = 0; i < dx; ++i)
    {
        if(D > 0)
        {
            if(steep)
                x += xStep;
            else
                y +=yStep;
            D += deltaDsecondary;
        }
        else
            D += delatDprimary;
        if(steep)
            y += yStep;
        else
            x += xStep;
        if(y < rasterHeight && x < rasterWidth) 
            raster[y*rasterWidth + x] = color;
    }
}
void rasterizer::RGB_test()
{
    for (size_t i = 0; i < rasterHeight; ++i)
        for (size_t j = 0; j < rasterWidth; ++j)
        {
            output::RGBA32 C = {uint8_t(255 * (float(i)/rasterHeight)), 0, uint8_t(255 * (float(j)/rasterWidth)), 0};
            this->raster[i*rasterWidth + j] = C;
        }
}
inline std::tuple<float, float, float> get_barycentric_coords(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 P)
{
    float alpha = ((b.y - c.y)*P.x + (c.x - b.x)*P.y + b.x*c.y - c.x*b.y)/((b.y - c.y)*a.x + (c.x - b.x)*a.y + b.x*c.y - c.x*b.y);
    float beta  = ((c.y - a.y)*P.x + (a.x - c.x)*P.y + a.y*c.x - c.y*a.x)/((c.y - a.y)*b.x + (a.x - c.x)*b.y + a.y*c.x - c.y*a.x);
    float gamma = ((a.y - b.y)*P.x + (b.x - a.x)*P.y + a.x*b.y - a.y*b.x)/((a.y - b.y)*c.x + (b.x - a.x)*c.y + a.x*b.y - a.y*b.x);
    return std::make_tuple(alpha, beta, gamma);
}
void rasterizer::draw_triangle_scr(glm::vec<2, int> a, glm::vec<2, int> b, glm::vec<2, int> c, glm::vec<3, glm::vec3> per_vertex_color)
{
    int xMin = std::min(std::min(a.x, b.x), c.x);
    int xMax = std::max(std::max(a.x, b.x), c.x);
    int yMin = std::min(std::min(a.y, b.y), c.y);
    int yMax = std::max(std::max(a.y, b.y), c.y);

    for(size_t x = xMin; x < xMax; ++x)
        for(size_t y = yMin; y < yMax; ++y)
        {
            auto bary_coords = get_barycentric_coords(a, b, c, {x + 0.5, y + 0.5});
            if (std::get<0>(bary_coords) > 0 && std::get<1>(bary_coords) > 0 && std::get<2>(bary_coords) > 0)
            {
                glm::vec3 color = per_vertex_color[0] * std::get<0>(bary_coords) + per_vertex_color[1] * std::get<1>(bary_coords) + 
                per_vertex_color[2] * std::get<2>(bary_coords);
                raster[y*rasterWidth + x] = {uint8_t(color.r * 255), uint8_t(color.g * 255), uint8_t(color.b * 255), 255};
            }
        }
}
rasterizer::~rasterizer()
{
    if(ownsRaster)
        delete [] raster;
}
