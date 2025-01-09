#pragma once

#include "format.h"
#include <cassert>
#include <cstddef>
#include <cstring>

namespace AiCo
{
    class raster 
    {
        bool ownsData = true;
public:
        raster() = delete;
        raster(const raster& other) = delete;
        raster(raster&& other) = delete;

        raster(int width, int height, RGBA32* data = nullptr) : ownsData(data==nullptr ? true : false), width(width), height(height), 
        data(data == nullptr ? new RGBA32[width*height] : data) {}

        raster(RGBA32* data, int width, int height) : ownsData(false), width(width), height(height), data(data) {}

        const int width, height;
        RGBA32* const data = nullptr;
        
        inline RGBA32& at(size_t x, size_t y)
        {
            assert(x < width && y < height);
            return data[y*width + x];
        }

        ~raster(){if (ownsData) {delete [] data;}}
    };
};
