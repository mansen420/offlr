#pragma once

#include "format.h"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <vector>

namespace AiCo
{
    class raster_base
    {
    public:
        int width, height;
        RGBA32* data = nullptr;
        
        raster_base() = delete;
        raster_base(int width, int height, RGBA32* data) noexcept : width(width), height(height), data(data) {}

        inline RGBA32& at(size_t x, size_t y)
        {
            assert(x < width && y < height);
            return data[y*width + x];
        }
        inline const RGBA32& at(size_t x, size_t y)const 
        {
            assert(x < width && y < height);
            return data[y*width + x];
        }
        
        virtual ~raster_base() = 0;
    };
    inline raster_base::~raster_base() {}

    class raster : raster_base
    {
    public:
        raster() = delete;
        raster(const raster& other) = delete;
        raster& operator=(const raster& other) = delete;
        raster(raster&& other) noexcept : raster_base(other.width, other.height, other.data)
        {
            other.data = nullptr;
        }
        raster& operator=(raster&& other) noexcept
        {
            if(this == &other)
                return *this;

            delete [] this->data;
            this->width = other.width; this->height = other.height;
            this->data = other.data;

            other.data = nullptr;
            other.width = 0; other.height = 0;

            return *this;
        }

        raster(int width, int height) : raster_base(width, height, new RGBA32[width*height]({0, 0, 0, 0})) {}

        ~raster() noexcept override {delete [] data;}
    };

    class raster_view : public raster_base
    {
    public:
       raster_view(int width, int height, RGBA32* data) noexcept : raster_base(width, height, data) {}
       ~raster_view() noexcept override {}
    };
    class tiled_raster
    {
    public:
        raster& img;
        std::vector<raster_view> tiles;

        void gen_tiles(unsigned int count)
        {
        }

        const unsigned int count;
        tiled_raster(raster& image, unsigned int tiles) : img(image), count(tiles) {}

    };
};
