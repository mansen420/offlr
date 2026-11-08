#pragma once

#include "format.h"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "interval.h"
#include "material.h"
#include "geometry.h"
#include "raytracing/intersection.h"
#include "raytracing/ray.h"
#include "utils.h"

#include <functional>
#include <vector>

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
        
        inline auto rayGradient = [](const ray& sample, 
            color3f a={0.25, 0.4, 0.8}, 
            color3f b={1.f, 1.f, 1.f}) -> color3f
        {
            AiCo::color3f blue = {0.25, 0.4, 0.8};
            AiCo::color3f white = {1, 1, 1};
            AiCo::color3f gray = {0.0, 0.0, 0.0};
            AiCo::color3f red = {0.5f, 0.2f, 0.1f};
            return AiCo::lerp(0.5 * sample.dir.y + 0.5, red, blue);
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
            color3f topG, bottomG;
            interval K;

            unbiased_tracer(uint maxDepth, interval rayBounds, 
                color3f topGradient={0.25f, 0.4f, 0.8f}, 
                color3f bottomGradient={1.f, 1.f, 1.f}) : 
            maxDepth(maxDepth),
            topG(topGradient), bottomG(bottomGradient), K(rayBounds) {}

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
                        return scatterinfo->albedo * insct->mat.texture(*insct) * trace(scatterinfo->out, ++currentDepth, intersector, K);
                    else
                        return insct->mat.texture(*insct);
                }
                else//-->Gradient
                    return AiCo::lerp(/*alpha=*/map(R.dir.y, /*from=*/{-1.f, 1.f}),
                        bottomG, topG);
            }
        };
        
        struct dir_light_t
        {
            glm::vec3 dir; //normalized
            color3f c;
            float radius=0.1f;
        };
        struct point_light_t
        {
            glm::vec3 pos;
            color3f c;
            float radius=0.1f;
        };
        struct lights_t
        {
            std::vector<dir_light_t> directions;
            std::vector<point_light_t> points;
        };
        inline color3f light_contrib(const intersector_t& scene, interval K, const intersection_t& insct,
            const lights_t& lights, size_t nrSamples=8)
        {
            color3f acc(0.f);
            for(const auto& dirL : lights.directions)
            {
                color3f samplesAcc(0.f);
                glm::vec3 T, B;
                make_tangent_frame(T, B, insct.N);
                auto lightContrib = std::max(glm::dot(insct.N, dirL.dir), 0.f)*dirL.c;
                for(size_t i = 0; i < nrSamples; ++i)
                {
                    auto jitter = dirL.radius * randvec_in_unit_disk();
                    //note: lightDir goes surface-->light
                    ray shadow_ray(/*dir=*/dirL.dir, /*origin=*/insct.P + 1e-4f*insct.N + 
                        jitter.x*T + jitter.y*B);
                    auto shadowInsct = scene(shadow_ray, K);
                    //light contribution 
                    samplesAcc +=  shadowInsct.has_value()? color3f(0.f): lightContrib;
                }
                samplesAcc /= float(nrSamples);
                acc += samplesAcc;
            }
            for(const auto& pointL : lights.points)
            {
                color3f samplesAcc(0.f);
                auto lightDir = glm::normalize(pointL.pos - insct.P);
                glm::vec3 T, B;
                make_tangent_frame(T, B, lightDir);
                for(size_t i = 0; i < nrSamples; ++i)
                {
                    auto jitter = pointL.radius * randvec_in_unit_disk();
                    auto jPos = jitter.x * T + jitter.y * B + pointL.pos;
                    auto dir = glm::normalize(jPos - insct.P);
                    //note: lightDir goes surface-->light
                    ray shadow_ray(/*dir=*/dir, /*origin=*/insct.P + 1e-4f*insct.N);
                    auto shadowInsct = scene(shadow_ray, K);
                    //light contribution 
                    samplesAcc +=  shadowInsct.has_value()? color3f(0.f):
                        std::max(glm::dot(insct.N, dir), 0.f)*pointL.c;
                }
                samplesAcc /= float(nrSamples);
                acc += samplesAcc;
            }
            return acc;
        }


        //simple biased integrator. direct illumination + recursive reflection
        class direct_recursive_tracer : public tracer
        {
        public:
            uint maxDepth;
            color3f topG, bottomG;
            interval K;
            lights_t lights;

            direct_recursive_tracer(uint maxDepth, interval rayBounds,
                lights_t lights,
                color3f topGradient={0.25f, 0.4f, 0.8f}, 
                color3f bottomGradient={1.f, 1.f, 1.f}) : 
            maxDepth(maxDepth),
            topG(topGradient), bottomG(bottomGradient), 
            K(rayBounds), lights(lights){}

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
                    {
                        color3f contrib = insct->mat.texture(*insct) *
                            scatterinfo->albedo *
                            light_contrib(intersector, K, *insct, lights);
                        auto energy =  contrib + scatterinfo->albedo * insct->mat.texture(*insct) * 
                            trace(scatterinfo->out, currentDepth + 1, intersector, K);

                        float fogDensity = 0.04f; // tune this
                        float fogAmount = 1.f - exp(-fogDensity * insct->t);
                        color3f fogColor = {0.7f, 0.7f, 0.8f}; // pale gray-blue sky color
                        color3f result = mix(energy, fogColor, fogAmount);
                        
                        return energy * expf(-insct->t * 0.1f); //modulate energy with distance
                        //return pow(((insct->P)), glm::vec3{1.5f});
                    }
                    else
                        return insct->mat.texture(*insct);
                }
                else//-->Gradient
                    return AiCo::lerp(/*alpha=*/map(R.dir.y, /*from=*/{-1.f, 1.f}),
                        bottomG, topG);
            }
        };
    }
}
