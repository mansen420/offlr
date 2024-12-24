#pragma once

#include "camera.h"
#include "format.h"
#include "raster.h"
#include "raytracing/ray.h"
#include <cstddef>

namespace AiCo 
{
    class renderer
    {
    public:
        const camera view;
        uint samplesPerPixel;
        color3f (*trace) (const ray& R);
        raster image;

        renderer(camera view, uint samplesPerPixel, int width, int height, color3f (*trace) (const ray& R), RGBA32* data = nullptr) : 
        view(view), samplesPerPixel(samplesPerPixel), trace(trace), image(width, height, data) {}

        void render()
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

        void operator()()
        {
            render();
        }
    };
}
