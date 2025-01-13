#pragma once

#include "camera.h"
#include "format.h"
#include "glm/exponential.hpp"
#include "raster.h"
#include "raytracing/ray.h"
#include "threadpool.h"
#include "utils.h"
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <functional>
#include <thread>

namespace AiCo 
{
    namespace RT
    {
        class renderer
        {
            static threadpool threads;
        public:
            const camera view;
            uint samplesPerPixel;
            std::function<color3f(const ray& R)> trace;

            renderer(camera view, uint samplesPerPixel, std::function<color3f(const ray& R)> trace) : 
            view(view), samplesPerPixel(samplesPerPixel), trace(trace){}
            
            void sample(raster& image){return sample(image, trace, view);}
            static void sample(raster& image, const std::function<color3f(const ray& R)>& trace, camera view)
            {
                render(image, trace, view, 1);
            }

            void render(raster& image){return render(image, trace, view, samplesPerPixel);}

            static void render(raster& image, const std::function<color3f(const ray& R)>& trace, camera view, uint samplesPerPixel)
            {
                unsigned int count = threads.count();
                
                unsigned int nrRows = std::sqrt(count);
                unsigned int nrCols = (count +  nrRows - 1)/nrRows;

                tiled_raster tiles(image, nrRows, nrCols);
                
                auto renderTile = [](raster_view tile, unsigned int samplesPerPixel, camera view, 
                const std::function<color3f(const ray& R)>& trace)->void
                {
                    for(size_t i = 0; i < tile.width; ++i)
                        for(size_t j = 0; j < tile.height; ++j)
                        {
                            color3f samplesAcc = color3f{0.f, 0.f, 0.f};
                            for(size_t k = 0; k < samplesPerPixel; k++)
                                samplesAcc += trace(view.samplePixel(i + tile.xOffset, j + tile.yOffset));
                            tile.at(i, j) = colorftoRGBA32(gamma(1.f/samplesPerPixel * samplesAcc, 2.f));
                        }
                };
                auto renderTileTest = [&image](raster_view tile, color3f color)->void
                {
                    for(size_t i = 0; i < tile.width; ++i)
                        for(size_t j = 0; j < tile.height; ++j)
                        {
                            if (&tile.at(i, j) != &image.at(i + tile.xOffset, j + tile.yOffset))
                                printf("Indexing Mismatch! Local: (%zu, %zu), Global: (%lu, %lu), Offset : (%u, %u)\n", i, j, 
                                i + tile.xOffset, j + tile.yOffset, tile.xOffset, tile.yOffset);
                            tile.at(i, j) = colorftoRGBA32(gamma(color, 2.f));
                        }
                };               
                for (auto tile : tiles.tiles)
                    renderTile(tile, samplesPerPixel, view, trace);
                    //threads.enqueue_job([=](){renderTile(tile, samplesPerPixel, view, trace);});
                
                while(threads.busy())
                {
                    continue;
                }
            }
            
            void operator()(raster& image)
            {
                render(image);
            }
        };
        inline threadpool renderer::threads = threadpool();
    }
}
