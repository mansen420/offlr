#pragma once

#include <cassert>
#include <optional>

#include "format.h"
#include "raytracing/ray.h"
#include "utils.h"
#include "intersection.h"
#include "perlin.h"

namespace AiCo
{
    namespace RT
    {
        class scatter_model
        {
        public:
            [[nodiscard]] virtual std::optional<scatter_t> operator()(const intersection_t&)const = 0;

            virtual ~scatter_model() = default;
        };

        class lambertian_diffuse : public scatter_model
        {
        public:
            color3f albedo = {0.f, 0.f, 0.f}; // "fractional reflectance" 
            lambertian_diffuse(color3f albedo = {0.5f, 0.5f, 0.5f}) : albedo(albedo/PI) {}

            [[nodiscard]] virtual std::optional<scatter_t> operator()(const intersection_t& insct)const override
            {
                glm::vec3 scatterDir = insct.N + randvec_on_unit_sphere();
                if(nearzero_vec(scatterDir))
                    scatterDir = insct.N;
                return scatter_t(ray(scatterDir, insct.P), albedo);
            }
        };
        class metallic : public scatter_model
        {
        public:
            color3f albedo;
            float fuzzmin;
            float fuzzmax;
            float noise_freq;
            metallic(color3f albedo = {1.f, 1.f, 1.f}, float fuzzmin = 0.15, float fuzzmax = 0.2f,
                float noise_freq = 5.f) 
                : albedo(albedo), fuzzmin(fuzzmin), fuzzmax(fuzzmax), noise_freq(noise_freq) {}
            [[nodiscard]] virtual std::optional<scatter_t> operator()(const intersection_t& insct)const override
            {
                float fuzzfactor = 0.f;
                if(fuzzmin > 0.f)//-->avoid noise lookups if possible
                {
                    float ratio = map(noise(insct.P * noise_freq), /*from=*/{-1, 1}, /*to=*/{0, 1});
                     fuzzfactor = lerp(ratio, fuzzmin, fuzzmax);
                }
                return scatter_t(ray(fuzzfactor * randvec_on_unit_sphere() + reflect(insct.inDir, insct.N),
                    insct.P + 1e-4f*insct.N), albedo);
            }
        };

        class emissive : public scatter_model
        {
        public:
            emissive()=default;

            [[nodiscard]] virtual std::optional<scatter_t> operator()(const intersection_t& insct)const override
            {
                return {};
            }
        };
    }
};
