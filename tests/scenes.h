#pragma once
#include "format.h"
#include "raytracing/geometry.h"
#include "raytracing/material.h"
#include "raytracing/intersection.h"
#include "raytracing/tracer.h"
#include "materials.h"

using namespace AiCo;
using namespace AiCo::RT;

// Scene setup
inline plane ground({0.f, 1.f, 0.f}, {0.f, -0.45f, 0.f}, mat_registry[NOISE]);
inline noisy_terrain terra(mat_registry[MATTE_SAND]);
inline std::vector<intersector_t> spaceDuneScene = { std::ref(terra) };

inline plane mirror1({1.f, 1.f, 0.f}, {0.f, 0.f, 0.f}, mat_registry[FUZZ_METAL]);
inline plane mirror2({-1.f, 1.f, 0.f}, {0.f, 0.f, 0.f}, mat_registry[FUZZ_METAL]);
inline plane mirror3({0.f, 0.f, 1.f}, {0.f, 0.f, 0.f}, mat_registry[LAMBERT]);

inline noisy_terrain debugTerra(mat_registry[LAMBERT]);
inline sphere debugBall1(0.5f, {0.0f, -0.25f, -1.5f}, mat_registry[NOISE]);
inline sphere debugBall2(0.45f, {0.f, 0.f, -0.5f}, mat_registry[LAMBERT]);
inline sphere debugBall3(0.1f, {0.f, 0.6f, 0.4f}, mat_registry[MATTE_RED]);
inline std::vector<intersector_t> debugScene = {debugTerra};
inline lights_t debugLights{.directions={dir_light_t{{1.f, 1.f, 0.f}, color3f(1.f), 0.02f}}};
//{.directions={dir_light_t{{1.f, 1.f, 0.f}, color3f(1.f), 0.f}}};

inline std::vector<dir_light_t> dirLs{dir_light_t{{1.f, 1.f, 0.0f}, {0.2f, 0.3f, 0.5f}, 0.05f}};
inline glm::vec3 PLpos{0.5f, 2.5f, -2.5f}; 
inline float PLradius{0.05f};
inline std::vector<point_light_t> pointLs={point_light_t{PLpos, {0.6f, 0.4f, 0.5f}, PLradius}};
inline lights_t spaceDuneLights{.directions={dirLs[0]}, .points={}};

inline void init_scenes()
{
    // Lattice parameters
    const int NX = 1;         // number of spheres along X
    const int NZ = 2;         // number of spheres along Z
    const float SPACING = 2.5; // distance between sphere centers
    const float RADIUS = 1.f; // sphere radius

    for (int i = 0; i < NX; ++i)
    {
        for (int j = 0; j < NZ; ++j)
        {
            glm::vec3 center = {
                (i - NX / 2.f) * SPACING,
                RADIUS + 1.5f,  // resting on ground
                (j - NZ / 2.f) * SPACING
            };

            // pick a material for now — replace later as needed
            const material_t& mat = mat_registry[STEEL_METAL];
            spaceDuneScene.push_back(sphere(RADIUS, center, mat));
        }
    }
}
