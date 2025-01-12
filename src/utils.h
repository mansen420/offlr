#pragma once

#include "glm/glm.hpp"

#include <cstdlib>
#include <limits>

#include "interval.h"
#include "format.h"

namespace AiCo 
{
    constexpr float PI = 3.1415926535897932385;

    constexpr inline float degrees_to_radians(float degrees){return degrees * PI / 180.0;}  

    template<glm::length_t L, typename T>
    [[nodiscard]] glm::vec<L, T> lerp (float alpha, glm::vec<L, T> a, glm::vec<L, T> b) {return (1-alpha)*a + (alpha)*b;}
    
    /**
     * @brief Returns random number in [0, 1[
     */
    inline float rand(){return std::rand()/(RAND_MAX + 1.f);}
    inline float rand(interval K){return K.min + (K.max - K.min)*AiCo::rand();}

    inline glm::vec3 randvec(){return glm::vec3(rand(), rand(), rand());}
    inline glm::vec3 randvec(interval K){return glm::vec3(rand(K), rand(K), rand(K));}
    
    inline glm::vec3 randvec_on_unit_sphere()
    {
        while (true)
        {
            auto candidate = randvec({-1.f, 1.f});
            if (glm::length(candidate) <= 1.f && glm::length(candidate) > std::numeric_limits<float>::min())
                return glm::normalize(candidate);
        }
    }
    inline glm::vec3 randvec_on_hemisphere(glm::vec3 surface_outward_normal)
    {
        auto candidate = randvec_on_unit_sphere();
        if(glm::dot(candidate, surface_outward_normal) > 0.f)
            return candidate;
        else
            return -candidate;
    }
    template<glm::length_t len>
    inline bool nearzero_vec(glm::vec<len, float> u, float precision = 1e-8){return interval(-precision, precision).contains(u);}
    inline color3f gamma(color3f color, float gammanum)
    {
        assert(gammanum > 0);
        return glm::pow(color, glm::vec3(1.f/gammanum));
    }
}
