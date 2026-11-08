#pragma once

#include "format.h"
#include "raytracing/material.h"
#include "raytracing/intersection.h"
#include "raytracing/perlin.h"
#include "registry.h"

using namespace AiCo::RT;
using namespace AiCo;

inline registry<material_t> mat_registry;

inline auto BRUSHED_STEEL = mat_registry.add(new material_t{
    .scatter = metallic(glm::vec3(1.f), 0.25f),
    .texture = [](const intersection_t& isect) {
        float pattern = sin(isect.P.z * 100.0f) * 0.5f + 0.5f;
        color3f base = {0.7f, 0.7f, 0.75f};
        color3f grain = {0.6f, 0.6f, 0.65f};
        return glm::mix(base, grain, pattern);
    }
});

inline auto PORCELAIN = mat_registry.add(new material_t{
    .scatter = lambertian_diffuse(glm::vec3(0.9f)),
    .texture = [](const intersection_t& isect) {
        float noise = sin(dot(isect.P, glm::vec3(10.0f))) * 0.05f;
        return color3f{0.95f + noise, 0.93f + noise, 0.9f + noise};
    }
});

inline auto IRIDESCENT_OIL = mat_registry.add(new material_t{
    .scatter = metallic(glm::vec3(1.f), 0.05f),
    .texture = [](const intersection_t& isect) {
        float f = fabs(sin(dot(isect.N, glm::vec3(0.5f, 0.8f, 0.1f)) * 12.0f));
        color3f c1 = {1.0f, 0.2f, 0.7f};
        color3f c2 = {0.2f, 0.8f, 1.0f};
        color3f c3 = {1.0f, 1.0f, 0.3f};
        if (f < 0.33f) return glm::mix(c1, c2, f * 3.0f);
        else if (f < 0.66f) return glm::mix(c2, c3, (f - 0.33f) * 3.0f);
        else return glm::mix(c3, c1, (f - 0.66f) * 3.0f);
    }
});

inline auto LAVA_ROCK = mat_registry.add(new material_t{
    .scatter = metallic(),
    .texture = [](const intersection_t& isect) {
        float n = sin(isect.P.x * 8.0f) + sin(isect.P.y * 8.0f) + sin(isect.P.z * 8.0f);
        n = fmod(fabs(n), 1.0f);
        if (n > 0.8f)
            return color3f{1.0f, 0.4f, 0.1f}; // lava cracks
        else
            return color3f{0.05f, 0.05f, 0.05f}; // cooled rock
    }
});

inline auto COPPER_MARBLE = mat_registry.add(new material_t{
    .scatter = metallic(glm::vec3(1.f), 0.15f),
    .texture = [](const intersection_t& isect) {
        float v = sin(isect.P.x * 3.0f + sin(isect.P.y * 5.0f) + cos(isect.P.z * 4.0f));
        v = 0.5f + 0.5f * v;
        color3f base = {0.9f, 0.6f, 0.3f};  // copper
        color3f vein = {0.1f, 0.05f, 0.02f};
        return glm::mix(base, vein, v);
    }
});
inline auto EMIT = mat_registry.add(new material_t
    {
        .scatter = emissive(),
        .texture = [](const intersection_t&){return color3f(1.f);}
    });

auto radialTex = [](const intersection_t& isect) {
        float rings = sin(glm::length(isect.P) * 100.0f);
        if (rings > 0)
            return color3f{0.2f, 0.2f, 1.0f}; // blue
        else
            return color3f{1.0f, 1.0f, 1.0f}; // white
    };
auto RADIAL = mat_registry.add(new material_t
{
    .scatter = metallic(glm::vec3(1.f), 0.35f),
    .texture = radialTex
});
inline color3f checkerboard (const intersection_t& isect)
{
    float scale = 6.0f; // controls check size in world space
    int xCheck = static_cast<int>(floor(isect.P.x * scale));
    int yCheck = static_cast<int>(floor(isect.P.y * scale));
    int zCheck = static_cast<int>(floor(isect.P.z * scale));

    // 3D checker: alternates along all axes
    if ((xCheck + yCheck + zCheck) % 2 == 0) {
        return  color3f{1.0f, 1.0f, 1.0f}; // white
    } else {
        return color3f{0.2f, 0.2f, 0.2f}; // black
    }
}
inline auto GOLD_METAL = mat_registry.add(new material_t{
    .scatter = metallic(glm::vec3(1.f), 0.05f), // gold tone, slightly rough
    .texture = [](const intersection_t& insct){return color3f{1.f, 0.8f, 0.6f} * 
        checkerboard(insct);}
});

inline auto CHECK_METAL = mat_registry.add(new material_t{
    .scatter = metallic(glm::vec3(1.f), 0.05f), // gold tone, slightly rough
    .texture = [](const intersection_t& insct){return color3f{0.8f, 0.8f, 0.8f} * 
        checkerboard(insct);}
});

inline auto STEEL_METAL = mat_registry.add(new material_t{
    .scatter = metallic(glm::vec3(1.f), 0.05f, .2f, 30.f),
    .texture = [](const intersection_t& insct) { return color3f{0.95f, 0.8f, 0.8f}; }
});

inline auto MATTE_RED = mat_registry.add(new material_t{
    .scatter = lambertian_diffuse(glm::vec3(0.9f)),
    .texture = [](const intersection_t&) { return color3f{0.7f, 0.1f, 0.1f}; }
});
inline auto MATTE_SAND = mat_registry.add(new material_t{
    .scatter = metallic(glm::vec3(1.0), 0.8f, 1.0, 40.f),
    .texture = [](const intersection_t& insct) { return color3f{0.86f, 0.7f, 0.5f}; }
});

inline auto NORMAL_MAP = mat_registry.add(new material_t{
    .scatter = lambertian_diffuse(color3f(1.f)),
    .texture = [](const intersection_t& insct) { return (0.5f*insct.N + 0.5f);}
});
inline auto LAMBERT = mat_registry.add(new material_t{
    .scatter = lambertian_diffuse(glm::vec3(1.0f)),
    .texture = [](const intersection_t&) { return color3f(1.f);}
});
inline auto NOISE = mat_registry.add(new material_t{
    .scatter = lambertian_diffuse(glm::vec3(0.8f, 0.8f, 0.9f)),
    .texture = [](const intersection_t& insct) { return 
        fBm(insct.UV, noise, 6, 3.5f,25.f) * color3f(1.f);}
});

inline auto FUZZ_METAL = mat_registry.add(new material_t{
    .scatter = metallic(color3f(1.f), 0.02, 0.03),
    .texture = [](const intersection_t&) { return color3f(0.8f, 0.85, 0.9f);}
});
