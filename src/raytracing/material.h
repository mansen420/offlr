#pragma once

#include <optional>

#include "format.h"
#include "raytracing/ray.h"
#include "raytracing/geometry.h"

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
                const ray out;
                const color3f attenuation;
            };

            [[nodiscard]] virtual std::optional<scatter_t> scatter(const ray& insctRay, geometry::intersection_t insct)const = 0;
        };

        class lambertian_diffuse : public material
        {
            [[nodiscard]] virtual std::optional<scatter_t> scatter(const ray& insctRay, geometry::intersection_t insct)const
            {

            }
        };
    }
};
