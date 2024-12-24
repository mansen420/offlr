#pragma once

#include "glm/glm.hpp"

#include <algorithm>
#include <cstdlib>
#include <limits>

namespace AiCo 
{
    constexpr float INF = std::numeric_limits<float>::infinity();
    constexpr float PI = 3.1415926535897932385;

    constexpr inline float degrees_to_radians(float degrees){return degrees * PI / 180.0;}  

    template<glm::length_t L, typename T>
    [[nodiscard]] glm::vec<L, T> lerp (float alpha, glm::vec<L, T> a, glm::vec<L, T> b) {return (1-alpha)*a + (alpha)*b;}
    
    inline float rand(){return std::rand()/(RAND_MAX + 1.f);}
    inline float rand(float min, float max){return min + (max - min)*AiCo::rand();}

    class interval
    {
    public:
        const float min, max;
        interval(float min, float max) : min(min), max(max){}
        [[nodiscard]] float span(){return max - min;}
        [[nodiscard]] bool contains(float x){return x >= min && x <= max;}
        [[nodiscard]] bool cntains_proper(float x){return x > min && x < max;}
        [[nodiscard]] float clamp(float x){return std::clamp(x, min, max);}
        static const interval EMPTY, NORM, UNIVERSE;
    };

    const interval interval::EMPTY = interval(+INF, -INF);
    const interval interval::UNIVERSE = interval(-INF, +INF);
    const interval interval::NORM = interval(0.f, 1.f);
}
