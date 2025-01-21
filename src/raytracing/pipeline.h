#pragma once

#include <functional>

#include "format.h"
#include "raytracing/geometry.h"
#include "raytracing/tracer.h"

namespace AiCo 
{
    namespace RT 
    {
        typedef std::function<color3f(ray)> pipeline_t;
        class pipeline_base
        {
        public:
            virtual color3f operator()(ray R) const = 0;
            virtual ~pipeline_base() = default;
        };

        class simple_pipeline : public pipeline_base
        {
        public:
            intersector_t sceneInsctr;
            interval K;
            tracer_t tracer;

            simple_pipeline(intersector_t sceneInsctr, interval K, tracer_t tracer) : sceneInsctr(sceneInsctr), K(K), tracer(tracer) {}

            color3f operator()(ray R)const override
            {
                return tracer(R, sceneInsctr, K);
            }
        };
    }
}
