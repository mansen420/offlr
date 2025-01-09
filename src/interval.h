#pragma once

#include "glm/detail/qualifier.hpp"
#include "glm/glm.hpp"

#include <limits>
#include <algorithm>
#include <type_traits>

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
        [[nodiscard]] bool contains_proper(float x)const{return x > min && x < max;}

        template<glm::length_t len, typename T>
        requires (std::is_arithmetic_v<T>)
        [[nodiscard]] bool contains(glm::vec<len, T> u)const
        {
            for (glm::length_t i = 0; i < len; ++i)
                if(!contains(u[i]))
                    return false;
            return true;
        }
        
        template<glm::length_t len, typename T>
        requires (std::is_arithmetic_v<T>)
        [[nodiscard]] bool contains_proper(glm::vec<len, T> u)const
        {
            for (glm::length_t i = 0; i < len; ++i)
                if(!contains_proper(u[i]))
                    return false;
            return true;
        }
        
        [[nodiscard]] float clamp(float x)const{return std::clamp(x, min, max);}
 
        template<glm::length_t len, typename T>
        requires (std::is_arithmetic_v<T>)
        [[nodiscard]] glm::vec<len, T> clamp(const glm::vec<len, T>& u)const
        {
            glm::vec<len, T> res;
            for (glm::length_t i = 0; i < len; ++i)
                res[i] = clamp(u[i]);
        }
        
        static const interval EMPTY, NORM, CANON, UNIVERSE;
        
        interval operator+(float x)const{return {min + x, max + x};}
        // symmetric inverse interval, bad idea?
        interval operator-()const{return {-max, -min};}
        interval operator-(float x)const{return (*this) + -x;}
        
        interval operator*(float x)const{return interval(x * min, x * max);}
        interval operator/(float x)const{assert(x!=0); return interval(min / x, max / x);}
    };
    inline interval operator*(float x, const interval u){return u * x;}
    inline interval operator/(float x, const interval u){return u / x;}
    inline interval operator+(float x, interval K){return K + x;}
    inline interval operator-(float x, interval K){return K - x;}

    const interval interval::EMPTY = interval(+INF, -INF);
    const interval interval::UNIVERSE = interval(-INF, +INF);
    const interval interval::NORM = interval(0.f, 1.f);
    const interval interval::CANON = interval(-1.f, 1.f);
}
