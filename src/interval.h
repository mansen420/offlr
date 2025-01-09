#pragma once

#include <limits>
#include <algorithm>

namespace AiCo
{ 
    constexpr float INF = std::numeric_limits<float>::infinity();
    
    class interval
    {
    public:
        const float min, max;

        [[nodiscard]] interval(float min, float max) : min(min), max(max){}
        [[nodiscard]] float span()const{return max - min;}
        [[nodiscard]] bool contains(float x)const{return x >= min && x <= max;}
        [[nodiscard]] bool cntains_proper(float x)const{return x > min && x < max;}
        [[nodiscard]] float clamp(float x)const{return std::clamp(x, min, max);}

        static const interval EMPTY, NORM, CANON, UNIVERSE;

        interval operator*(float x)const{return interval(x * min, x * max);}
        interval operator/(float x)const{return interval(min / x, max / x);}
    };
    inline interval operator*(float x, const interval u){return u * x;}
    inline interval operator/(float x, const interval u){return u / x;}

    const interval interval::EMPTY = interval(+INF, -INF);
    const interval interval::UNIVERSE = interval(-INF, +INF);
    const interval interval::NORM = interval(0.f, 1.f);
    const interval interval::CANON = interval(-1.f, 1.f);
}
