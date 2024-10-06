#include "glm/fwd.hpp"
#include "glm/glm.hpp"
#include "output.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <sys/types.h>
#include <utility>
#include "rasterizer.h"

/**
 * @brief World to raster transform : typical translate -> scale -> rotate transform
 */
rasterizer::rasterizer(uint rasterWidth, uint rasterHeight, glm::vec2 worldX, glm::vec2 worldY, output::RGBA32* rasterPtr) 
: raster(rasterPtr == nullptr ? new output::RGBA32[rasterWidth * rasterHeight] : rasterPtr), rasterWidth(rasterWidth), rasterHeight(rasterHeight), 
ownsRaster(rasterPtr == nullptr ? true : false)
{
    if(ownsRaster)
        std::fill(raster, raster + rasterHeight*rasterWidth, output::RGBA32{255, 255, 255, 255});

    float worldXSpan = worldX[1] - worldX[0];
    float worldYSpan = worldY[1] - worldY[0];
    float worldXmidPoint = (worldX[0] + worldX[1])/2.f;
    float worldYmidPoint = (worldY[0] + worldY[1])/2.f;

    glm::vec3 rightCol(-worldXmidPoint, -worldYmidPoint, 1.f);
    glm::vec3 midCol(0.f, rasterHeight/worldYSpan, 0.f);
    glm::vec3 leftCol(rasterWidth/worldXSpan, 0.f, 0.f);

    WtoSCR = glm::mat3(leftCol, midCol, rightCol);
}
inline void rasterizer::draw_point(glm::vec2 worldCoords, output::RGBA32 color)
{
    glm::vec3 homogCoords(worldCoords, 1.f);
    auto screenCoords = WtoSCR * homogCoords;
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
void rasterizer::draw_line_midpoint_world(glm::vec2 worldP1, glm::vec2 worldP2, output::RGBA32 color)
{
    glm::vec<2, uint> p1(WtoSCR * glm::vec3(worldP1, 1.f));
    glm::vec<2, uint> p2(WtoSCR * glm::vec3(worldP2, 1.f));
    draw_line_midpoint_scr(p1,  p2, color);
}
void rasterizer::clear(output::RGBA32 color)
{
    std::fill(raster, raster + rasterHeight*rasterWidth, color);
}
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

rasterizer::~rasterizer()
{
    if(ownsRaster)
        delete [] raster;
}
