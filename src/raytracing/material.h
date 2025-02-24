#pragma once

#include <cassert>
#include <optional>

#include "format.h"
#include "raytracing/ray.h"
#include "utils.h"
#include "intersection.h"

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
            lambertian_diffuse(color3f albedo = {0.5f, 0.5f, 0.5f}) : albedo(albedo) {}

            [[nodiscard]] virtual std::optional<scatter_t> operator()(const intersection_t& insct)const override
            {
                glm::vec3 scatterDir = insct.N + randvec_on_unit_sphere();
                if(nearzero_vec(scatterDir))
                    scatterDir = insct.N;
                return scatter_t(ray(scatterDir, insct.P));
            }
        };
        class metallic : public scatter_model
        {
        public:
            color3f albedo;
            float fuzz;
            metallic(color3f albedo = {1.f, 1.f, 1.f}, float fuzz = 0.15) : albedo(albedo), fuzz(fuzz) {}
            [[nodiscard]] virtual std::optional<scatter_t> operator()(const intersection_t& insct)const override
            {
                return scatter_t(ray(fuzz * randvec_on_unit_sphere() + reflect(insct.inDir, insct.N), insct.P));
            }
        };
    }
};
