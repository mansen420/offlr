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
        // this is a private implementation detail
        inline static threadpool threads = threadpool();

        class renderer
        {
        public:
            uint samplesPerPixel;
            pipeline_t pipeline;

            renderer(uint samplesPerPixel, const pipeline_t& pipeline) : samplesPerPixel(samplesPerPixel), pipeline(pipeline){}

            void render(raster& image){return render(image, pipeline, samplesPerPixel);}

            static void render(raster& image, const pipeline_t& pipeline, uint samplesPerPixel)
            {
                unsigned int count = 10*threads.count(); //experimental. Reduces cache-misses
                
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
                            //average samples
                            samplesAcc = 1.f/samplesPerPixel * samplesAcc;
                            //apply Reinhard tonemapping
                            color3f mapped = (0.5f*samplesAcc)/(0.5f*samplesAcc + 1.f);
                            tile.at(j, i) = colorftoRGBA32(gamma(mapped, 2.f));
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
                    threads.enqueue_job([tile, &pipeline, samplesPerPixel, renderTile](){renderTile(tile, samplesPerPixel, pipeline);});
                
                threads.wait_till_done();
            }
            
            void operator()(raster& image)
            {
                render(image);
            }
        };

        class step_renderer
        {
            color3f* acc;
            std::vector<raster_view> tiles;
            uint currentSample=0, samplesPerPixel=1, w, h;
            pipeline_t p;

            //emits a single sample render into a color buffer without post-processing
            void emit_sample()
            {
                if(currentSample>=samplesPerPixel) return;

                auto renderTile = [this](raster_view tile)
                {
                    for(size_t i=0; i<tile.height; ++i)
                        for(size_t j=0; j<tile.width; ++j)
                        {
                            uint idx = (tile.yOffset+i)*w + (tile.xOffset+j);
                            acc[idx] += p(tile.xOffset+j, tile.yOffset+i);
                        }
                };
                for (auto tile : tiles)
                    threads.enqueue_job([tile, renderTile](){renderTile(tile);});

                //XXX this threadpool could be doing other jobs since it's static!
                threads.wait_till_done();
                
                currentSample++;
            }
            void combine_samples(raster& image)
            {
                unsigned int count = 10*threads.count(); //experimental. Reduces cache-misses
                
                unsigned int nrRows = std::sqrt(count);
                unsigned int nrCols = (count +  nrRows - 1)/nrRows;

                auto write_tiles=tile_raster(&image, nrRows, nrCols);

                auto postprocess = [this](raster_view tile)
                {
                    for(size_t i=0; i<tile.height; ++i)
                        for(size_t j=0; j<tile.width; ++j)
                        {
                            uint idx = (tile.yOffset+i)*w + (tile.xOffset+j);

                            //average samples
                            color3f avg = 1.f/currentSample * acc[idx];
                            //apply Reinhard tonemapping
                            color3f mapped = (0.5f*avg)/(0.5f*avg + 1.f);
                            tile.at(j, i) = colorftoRGBA32(gamma(mapped, 2.f));
                        }
                };

                for(auto tile: write_tiles)
                    threads.enqueue_job([tile, postprocess](){postprocess(tile);});
                //XXX
                threads.wait_till_done();
            }
        public:
            step_renderer(uint spp, const pipeline_t& p, uint w, uint h):
            acc((color3f*) calloc(w*h, sizeof(color3f))),
            samplesPerPixel(spp),
            w(w), h(h),
            p(p)
            {
                unsigned int count = 10*threads.count(); //experimental. Reduces cache-misses
                
                unsigned int nrRows = std::sqrt(count);
                unsigned int nrCols = (count +  nrRows - 1)/nrRows;

                raster dummy(w, h);

                tiles=tile_raster(&dummy, nrRows, nrCols);
            }

            virtual void operator()(raster& image)
            {
                //render one sample into acc
                emit_sample();
                //blit average of samples
                combine_samples(image);
            }

            ~step_renderer(){free(acc);}
        };
    }
}
