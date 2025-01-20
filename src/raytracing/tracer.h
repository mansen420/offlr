#pragma once

#include "format.h"
#include "glm/common.hpp"
#include "interval.h"
#include "material.h"
#include "geometry.h"
#include "raytracing/intersection.h"
#include "raytracing/ray.h"
#include <functional>
#include <sys/types.h>

namespace AiCo 
{
    namespace RT
    {
        typedef std::function<color3f(const ray&)> tracer_t;
        class tracer_base
        {
        public:
            virtual color3f operator()(const ray& R, interval K, intersector_t insctr) const = 0;

            virtual ~tracer_base() = default;
        };
        
        inline color3f rayGradient(const ray& sample)
        {
            AiCo::color3f blue = {0.25, 0.4, 0.8};
            AiCo::color3f white = {1, 1, 1};
            return AiCo::lerp(0.5 * sample.dir.y + 0.5, blue, white);
        };

        inline color3f normaltracer(const ray& R, interval K, intersector_t insctr)
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
            const material* matptr;
            
            simple_tracer(material* matptr, intersector_t intersector, uint maxDepth) : maxDepth(maxDepth), 
            matptr(matptr){}
            
            virtual color3f operator()(const ray& R, interval K, intersector_t insctr)const
            {
                uint currentDepth = 0; 
                return trace(R, currentDepth, insctr);
            }
        private:
            color3f trace(const ray& R, uint currentDepth, intersector_t intersector)const
            {
                if(currentDepth >= maxDepth)
                    return {0.f, 0.f, 0.f};
                if(auto insct = intersector(R, {0.0001f, 10.f}); insct.has_value())
                {   
                    if(auto scatterinfo = matptr->scatter(R, *insct); scatterinfo.has_value())
                        return scatterinfo.value().attenuation * trace(scatterinfo->out, ++currentDepth, intersector);
                    else
                        return {0, 0, 0};
                }
                else
                    return rayGradient(R);
            }
        };
    }
}
