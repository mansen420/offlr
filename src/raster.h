#include "format.h"
#include <cstddef>

namespace AiCo
{
    class raster 
    {
        bool ownsData = true;
public:
        raster() = delete;
        raster(int width, int height) : width(width), height(height), data(new RGBA32[width*height]) {}
        raster(RGBA32* data, int width, int height) : ownsData(false), width(width), height(height), data(data) {}

        const int width, height;
        RGBA32* const data = nullptr;
        
        inline RGBA32& at(size_t x, size_t y){return data[y*width + x];}

        ~raster(){if (ownsData) {delete [] data;}}
    };
};
