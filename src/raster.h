#pragma once

#include "format.h"
#include <cassert>
#include <cmath>
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

        virtual inline RGBA32& at(size_t x, size_t y)
        {
            assert(x < width && y < height);
            return data[y*width + x];
        }
        virtual inline const RGBA32& at(size_t x, size_t y)const 
        {
            assert(x < width && y < height);
            return data[y*width + x];
        }
        
        virtual ~raster_base() = 0;
    };
    inline raster_base::~raster_base() {}

    class raster : public raster_base
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
        unsigned int xOffset, yOffset, parentWidth;
        raster_view(int width, int height, unsigned int xOffset, unsigned int yOffset, unsigned int parentWidth, RGBA32* data) noexcept : 
        raster_base(width, height, data), xOffset(xOffset), yOffset(yOffset), parentWidth(parentWidth){}
        
        inline RGBA32& at(size_t x, size_t y) override
        {
            assert(x < width && y < height);
            return data[y*parentWidth + x];
        }
        inline const RGBA32& at(size_t x, size_t y)const override 
        {
            assert(x < width && y < height);
            return data[y*parentWidth + x];
        }

        ~raster_view() noexcept override {}
    };
    class tiled_raster
    {
    public:
        raster& img;
        std::vector<raster_view> tiles;

        void gen_tiles (unsigned int nrRows, unsigned nrCols) noexcept
        {
            unsigned int count = nrCols * nrRows;

            tiles.reserve(count);

            unsigned int tileWidth = img.width/nrCols, tileHeight = img.height/nrRows;
            for (size_t i = 0; i < nrRows; ++i)
                for (size_t j = 0; j <  nrCols; ++j)
                {
                    unsigned int xOffset = tileWidth * j, yOffset = tileHeight * i;

                    unsigned int realWidth = j == nrCols - 1 ? img.width - xOffset : tileWidth;
                    unsigned int realHeight = i == nrRows - 1 ? img.height - yOffset : tileHeight;

                    tiles.push_back(raster_view(realWidth, realHeight, xOffset, yOffset, img.width, &img.at(xOffset, yOffset)));
                }
        }

        const unsigned int rows, cols;
        tiled_raster(raster& image, unsigned int nrRows, unsigned nrCols) : img(image), rows(nrRows), cols(nrCols) {gen_tiles(nrRows, nrCols);}

    };
};
