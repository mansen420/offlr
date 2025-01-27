#pragma once

#include "camera.h"
#include "format.h"
#include "raster.h"
#include "raytracing/tracer.h"
#include "threadpool.h"
#include "utils.h"
#include "raytracing/pipeline.h"
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <functional>

namespace AiCo 
{
    namespace RT
    {
        typedef std::function<void(raster_view*)> renderer_t;

        class renderer
        {
            static threadpool threads;
        public:
            uint samplesPerPixel;
            pipeline_t pipeline;

            renderer(uint samplesPerPixel, const pipeline_t& pipeline) : samplesPerPixel(samplesPerPixel), pipeline(pipeline){}

            void render(raster& image){return render(image, pipeline, samplesPerPixel);}

            static void render(raster& image, const pipeline_t& pipeline, uint samplesPerPixel)
            {
                unsigned int count = 20*threads.count(); //experimental. Reduces cache-misses
                
                unsigned int nrRows = std::sqrt(count);
                unsigned int nrCols = (count +  nrRows - 1)/nrRows;
                
                auto tiles = tile_raster(&image, nrRows, nrCols);

                auto renderTile = [](raster_view tile, unsigned int samplesPerPixel, const pipeline_t& pipeline)->void
                {
                    for(size_t i = 0; i < tile.height; ++i)
                        for(size_t j = 0; j < tile.width; ++j)
                        {
                            color3f samplesAcc = color3f{0.f, 0.f, 0.f};
                            for(size_t k = 0; k < samplesPerPixel; k++)
                                samplesAcc += pipeline(j + tile.xOffset, i + tile.yOffset);
                            tile.at(j, i) = colorftoRGBA32(gamma(1.f/samplesPerPixel * samplesAcc, 2.f));
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
                
                //TODO why is taking tile by reference wrong here?
                for (auto tile : tiles)
                    threads.enqueue_job([tile, &pipeline, samplesPerPixel, renderTile]()
                    {renderTile(tile, samplesPerPixel, pipeline);});
                
                threads.wait_till_done();
            }
            
            void operator()(raster& image)
            {
                render(image);
            }
        };
        inline threadpool renderer::threads = threadpool();
    }
}
