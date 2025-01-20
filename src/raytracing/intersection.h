#pragma once

#include "raytracing/ray.h"

namespace AiCo::RT
{
    struct intersection_t
    {
        intersection_t(const ray& R, const glm::vec3& outwardNormal, const glm::vec3& P, float t) : 
        P(P), N(outwardNormal), t(t), frontFace(glm::dot(outwardNormal, R.dir) < 0){}
        
        intersection_t() = delete;

        const glm::vec3 P, N;
        const float t;
        const bool frontFace;
    };
}
