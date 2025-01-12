#pragma once

#include "camera.h"
#include "format.h"
#include "glm/exponential.hpp"
#include "raster.h"
#include "raytracing/ray.h"
#include "raytracing/tracer.h"
#include <cassert>
#include <cmath>
#include <cstddef>
#include <functional>

namespace AiCo 
{
    namespace RT
    {
        class renderer
        {
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
                for(size_t i = 0; i < image.width; ++i)
                    for(size_t j = 0; j < image.height; ++j)
                    {
                        color3f samplesAcc = color3f{0.f, 0.f, 0.f};
                        for(size_t k = 0; k < samplesPerPixel; k++)
                        {
                            ray sampler = view.samplePixel(i, j);
                            samplesAcc += trace(sampler);
                        }
                        image.at(i, j) = colorftoRGBA32(gamma(1.f/samplesPerPixel * samplesAcc, 2.f));
                    }
            }
            
            void operator()(raster& image)
            {
                render(image);
            }
        };
    }
}
