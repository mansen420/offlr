#pragma once

#include "camera.h"
#include "format.h"
#include "raster.h"
#include "raytracing/ray.h"
#include <cstddef>
#include <functional>

namespace AiCo 
{
    class renderer
    {
    public:
        const camera view;
        uint samplesPerPixel;
        std::function<color3f(const ray& R)> trace;

        renderer(camera view, uint samplesPerPixel, const std::function<color3f(const ray& R)>& tracer) : 
        view(view), samplesPerPixel(samplesPerPixel), trace(tracer){}
        
        void sample(raster& image)
        {
            for(size_t i = 0; i < image.width; ++i)
                for(size_t j = 0; j < image.height; ++j)
                    image.at(i, j) = colorftoRGBA32(trace(view.samplePixel(i, j)));
        }
        void render(raster& image)
        {
            for(size_t i = 0; i < image.width; ++i)
                for(size_t j = 0; j < image.height; ++j)
                {
                    color3f samplesAcc = color3f{0.f, 0.f, 0.f};
                    for(size_t k = 0; k < samplesPerPixel; k++)
                    {
                        ray sampler = view.samplePixel(i, j);
                        samplesAcc += trace(sampler);
                    }
                    image.at(i, j) = colorftoRGBA32(samplesAcc * (1.f/samplesPerPixel));
                }
        }

        void operator()(raster& image)
        {
            render(image);
        }
    };
}
