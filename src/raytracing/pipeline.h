#pragma once

#include <cstddef>
#include <functional>

#include "format.h"
#include "raytracing/camera.h"
#include "raytracing/geometry.h"
#include "raytracing/tracer.h"

namespace AiCo 
{
    namespace RT 
    {
        typedef std::function<color3f(size_t x, size_t y)> pipeline_t;
        class pipeline
        {
        public:
            virtual color3f operator()(size_t x, size_t y) const = 0;
            virtual ~pipeline() = default;
        };

        class simple_pipeline : public pipeline
        {
        public:
            intersector_t sceneInsctr;
            tracer_t tracer;
            camera_t view;

            simple_pipeline(const intersector_t& sceneInsctr, const tracer_t& tracer, const camera_t& view) : 
            sceneInsctr(sceneInsctr), tracer(tracer), view(view) {}

            color3f operator()(size_t x, size_t y)const override
            {
                return tracer(view(x, y), sceneInsctr);
            }
        };
    }
}
