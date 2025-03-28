#pragma once

#include "format.h"
#include "interval.h"
#include "material.h"
#include "geometry.h"
#include "raytracing/intersection.h"
#include "raytracing/ray.h"

#include <functional>

namespace AiCo 
{
    namespace RT
    {
        typedef std::function<color3f(ray, const intersector_t&)> tracer_t;

        class tracer
        {
        public:
            virtual color3f operator()(ray, const intersector_t&) const = 0;

            virtual ~tracer() = default;
        };
        
        inline auto rayGradient = [](const ray& sample) -> color3f
        {
            AiCo::color3f blue = {0.25, 0.4, 0.8};
            AiCo::color3f white = {1, 1, 1};
            AiCo::color3f red = {0.5f, 0.2f, 0.1f};
            return AiCo::lerp(0.5 * sample.dir.y + 0.5, blue, white);
        };

        inline auto normalTracer = [](const ray& R, interval K, const intersector_t& insctr)->color3f
        {
            if(auto insct = insctr(R, K); insct.has_value())
                return 0.5f * insct->N + glm::vec3(0.5);
            else
                return rayGradient(R);
        };

        class unbiased_tracer : public tracer
        {
        public:
            uint maxDepth;
            
            interval K;

            unbiased_tracer(uint maxDepth, interval rayBounds) : maxDepth(maxDepth), K(rayBounds) {}

            inline virtual color3f operator()(ray R, const intersector_t& insctr)const override
            {
                uint currentDepth = 0; 
                return trace(R, currentDepth, insctr, K);
            }
        private:
            inline color3f trace(ray R, uint currentDepth, const intersector_t& intersector, interval K)const
            {
                if(currentDepth >= maxDepth)
                    return {0.f, 0.f, 0.f};
                
                //TODO early termination for low energy rays?
                
                if(auto insct = intersector(R, K); insct.has_value())
                {
                    if(auto scatterinfo = insct->mat.scatter(*insct); scatterinfo.has_value())
                        return insct->mat.texture(*insct) * trace(scatterinfo->out, ++currentDepth, intersector, K);
                    else
                        return insct->mat.texture(*insct);
                }
                else
                    return 0.8f * rayGradient(R);
            }
        };
    }
}
