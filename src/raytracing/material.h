#pragma once

#include <optional>

#include "format.h"
#include "raytracing/ray.h"
#include "raytracing/geometry.h"
#include "utils.h"

namespace AiCo
{
    namespace RT
    {
        class material
        {
        public:
            virtual ~material() = default;
            
            struct scatter_t
            {
                scatter_t(const ray& out, const color3f& attenuation) : out(out), attenuation(attenuation) {}
                scatter_t() = delete;
                const ray out;
                const color3f attenuation;
            };

            [[nodiscard]] virtual std::optional<scatter_t> scatter(const ray& insctRay, geometry::intersection_t insct)const = 0;
        };

        class lambertian_diffuse : public material
        {
        public:
            color3f albedo = {0, 0, 0};
            lambertian_diffuse(color3f albedo) : albedo(albedo) {}

            [[nodiscard]] virtual std::optional<scatter_t> scatter(const ray& insctRay, geometry::intersection_t insct)const
            {
                glm::vec3 scatterDir = insct.N + randvec_on_unit_sphere();
                if(nearzero_vec(scatterDir))
                    scatterDir = insct.N;
                return scatter_t(ray(scatterDir, insct.P), albedo);
            }
        };
    }
};
