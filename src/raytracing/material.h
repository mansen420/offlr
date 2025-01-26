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
        struct scatter_t
        {
            scatter_t(const ray& out, const color3f& attenuation) : out(out), attenuation(attenuation) {}
            scatter_t() = delete;
            const ray out;
            const color3f attenuation;
        };
        typedef std::function<std::optional<scatter_t>(intersection_t)> scatterer_t;
        class material
        {
        public:
            [[nodiscard]] virtual std::optional<scatter_t> operator()(intersection_t)const = 0;

            virtual ~material() = default;
        };

        class lambertian_diffuse : public material
        {
        public:
            color3f albedo = {0.f, 0.f, 0.f}; // "fractional reflectance" 
            lambertian_diffuse(color3f albedo) : albedo(albedo) {}

            [[nodiscard]] virtual std::optional<scatter_t> operator()(intersection_t insct)const override
            {
                glm::vec3 scatterDir = insct.N + randvec_on_unit_sphere();
                if(nearzero_vec(scatterDir))
                    scatterDir = insct.N;
                return scatter_t(ray(scatterDir, insct.P), albedo);
            }
        };
        class specular : public material
        {
        public:
            color3f albedo = {0.f, 0.f, 0.f};
            specular(color3f albedo) : albedo(albedo) {}
            [[nodiscard]] virtual std::optional<scatter_t> operator()(intersection_t insct)const override
            {
                return scatter_t(ray(reflect(insct.inDir, insct.N), insct.P), albedo);
            }
        };
    }
};
