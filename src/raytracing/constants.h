#pragma once

#include <limits>

constexpr float INF = std::numeric_limits<float>::infinity();
constexpr float PI = 3.1415926535897932385;

constexpr inline float degrees_to_radians(float degrees)
{
    return degrees * PI / 180.0;
}
