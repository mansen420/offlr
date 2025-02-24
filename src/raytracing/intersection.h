#pragma once

#include "raytracing/ray.h"
#include "format.h"

#include <optional>

namespace AiCo::RT
{
    struct material_t;

    struct intersection_t
    {
        intersection_t(const ray& R, const glm::vec3& outwardNormal, const glm::vec3& P, float t, const material_t& mat) : 
        P(P), N(outwardNormal), inDir(R.dir), t(t), frontFace(glm::dot(outwardNormal, R.dir) < 0), mat(mat) {}
        
        intersection_t() = delete;

        const glm::vec3 P, N, inDir;
        const float t;
        const bool frontFace;

        const material_t& mat;
    };
    typedef std::function<color3f(const intersection_t&)> texturer_t;
    
    struct scatter_t
    {
        scatter_t(const ray& out) : out(out) {}
        scatter_t() = delete;
        const ray out;
    };
    typedef std::function<std::optional<scatter_t>(const intersection_t&)> scatterer_t;

    struct material_t
    {
        scatterer_t scatter;
        texturer_t texture;
    };
}
