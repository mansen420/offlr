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
        typedef std::function<color3f(ray, intersector_t, interval)> tracer_t;
        class tracer_base
        {
        public:
            virtual color3f operator()(ray, intersector_t, interval) const = 0;

            virtual ~tracer_base() = default;
        };
        
        inline auto rayGradient = [](const ray& sample) -> color3f
        {
            AiCo::color3f blue = {0.25, 0.4, 0.8};
            AiCo::color3f white = {1, 1, 1};
            return AiCo::lerp(0.5 * sample.dir.y + 0.5, blue, white);
        };

        inline auto normalTracer = [](const ray& R, interval K, intersector_t insctr)->color3f
        {
            if(auto insct = insctr(R, K); insct.has_value())
                return 0.5f * insct->N + glm::vec3(0.5);
            else
                return rayGradient(R);
        };

        class simple_tracer : public tracer_base
        {
        public:
            uint maxDepth;
            
            //temporary. material is global to all objects. TODO make material vary on a per-object or per-intersection basis 
            // perhaps combine geometries and materials into a separate functor, or keep some global scene data (registries)
            scatterer_t scatter;
            
            simple_tracer(scatterer_t scatter, uint maxDepth) : maxDepth(maxDepth), 
            scatter(scatter){}
            
            virtual color3f operator()(ray R, intersector_t insctr, interval K)const override
            {
                uint currentDepth = 0; 
                return trace(R, currentDepth, insctr, K);
            }
        private:
            color3f trace(ray R, uint currentDepth, intersector_t intersector, interval K)const
            {
                if(currentDepth >= maxDepth)
                    return {0.f, 0.f, 0.f};
                if(auto insct = intersector(R, K); insct.has_value())
                {   
                    if(auto scatterinfo = scatter(*insct); scatterinfo.has_value())
                        return scatterinfo.value().attenuation * trace(scatterinfo->out, ++currentDepth, intersector, K);
                    else
                        return {0, 0, 0};
                }
                else
                    return {1.f, 1.f, 1.f};
            }
        };
    }
}
