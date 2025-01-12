#pragma once

#include "format.h"
#include "glm/common.hpp"
#include "material.h"
#include "geometry.h"
#include "raytracing/ray.h"
#include <sys/types.h>

namespace AiCo 
{
    namespace RT
    {
        class tracer
        {
        public:
            virtual color3f operator()(const ray& R) const = 0;

            virtual ~tracer() = default;
        };
        
        class ray_gradient : public tracer
        {
        public:
            color3f operator()(const ray& sample) const
            {
                AiCo::color3f blue = {0.25, 0.4, 0.8};
                AiCo::color3f white = {1, 1, 1};
                return AiCo::lerp(0.5 * sample.dir.y + 0.5, blue, white);
            }
        };

        class simple_tracer : public tracer
        {
        public:
            uint maxDepth;
            
            const nearest_hit_structure& structure;
            //temporary. material is global to all objects. TODO make material vary on a per-object or per-intersection basis 
            const material* matptr;
            
            simple_tracer(material* matptr, nearest_hit_structure& structure, uint maxDepth) : maxDepth(maxDepth), structure(structure), matptr(matptr){}
            
            virtual color3f operator()(const ray& R)const{uint currentDepth = 0; return trace(R, currentDepth);}
        private:
            color3f trace(const ray& R, uint currentDepth)const
            {
                if(currentDepth >= maxDepth)
                    return {0.f, 0.f, 0.f};
                if(auto insct = structure.testIntersect(R, {0.0001f, 10.f}); insct.has_value())
                {   
                    if(auto scatterinfo = matptr->scatter(R, *insct); scatterinfo.has_value())
                        return scatterinfo.value().attenuation * trace(scatterinfo->out, ++currentDepth);
                    else
                        return {0, 0, 0};
                }
                else
                    return ray_gradient()(R);
            }
        };
    }
}
