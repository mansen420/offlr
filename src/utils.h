#pragma once

#include "glm/geometric.hpp"
#include "glm/glm.hpp"

#include <cstdlib>
#include <limits>
#include <random>

#include "interval.h"
#include "format.h"

namespace AiCo 
{
    constexpr float PI = 3.1415926535897932385;

    constexpr inline float degrees_to_radians(float degrees){return degrees * PI / 180.0;}  

    template<glm::length_t L, typename T>
    [[nodiscard]] glm::vec<L, T> lerp (float alpha, glm::vec<L, T> a, glm::vec<L, T> b) {return (1-alpha)*a + (alpha)*b;}
    

    inline float fast_rand()
    {
        static thread_local std::minstd_rand RNG;
        std::uniform_real_distribution<float> dist(0.f, 1.f);
        return dist(RNG);
    }
    /**
     * @brief Returns random number in [0, 1[
     * thread-safe.
     */
    inline float rand()
    {   //high quality, but slow 
        return fast_rand(); //XXX testing
        static thread_local std::mt19937 RNG(std::random_device{}());
        std::uniform_real_distribution<float> dist(0.f, 1.f);
        return dist(RNG);
    }

    inline float rand(interval K){return K.min + (K.max - K.min)*AiCo::rand();}
    
    inline glm::vec3 randvec(){return glm::vec3(rand(), rand(), rand());}
    inline glm::vec3 randvec(interval K){return glm::vec3(rand(K), rand(K), rand(K));}
    
    // TODO for fuck's sake this function is 25% of the frametime. 
    // for god's sake please implement spherical coordinate sampling!
    [[nodiscard]] inline glm::vec3 randvec_on_unit_sphere()
    {
        while (true)
        {
            auto candidate = randvec({-1.f, 1.f});
            auto length = glm::length(candidate);
            if (length <= 1.f && length > std::numeric_limits<float>::min())
                return glm::normalize(candidate);
        }
    }
    [[nodiscard]] inline glm::vec2 randvec_in_unit_disk()
    {
        while(true)
        {
            glm::vec2 candidate{rand({-1, 1}), rand({-1, 1})};
            auto length = glm::length(candidate);
            if(length <= 1.f && length > std::numeric_limits<float>::min())
                return candidate;
        }
    }
    [[nodiscard]] inline glm::vec3 randvec_on_hemisphere(glm::vec3 surface_outward_normal)
    {
        auto candidate = randvec_on_unit_sphere();
        if(glm::dot(candidate, surface_outward_normal) > 0.f)
            return candidate;
        else
            return -candidate;
    }
    template<glm::length_t len>
    [[nodiscard]] inline bool nearzero_vec(glm::vec<len, float> u, float precision = 1e-8){return interval(-precision, precision).contains(u);}
    [[nodiscard]] inline color3f gamma(color3f color, float gammanum)
    {
        assert(gammanum > 0);
        return glm::pow(color, glm::vec3(1.f/gammanum));
    }
    
    /**
     * @param input vector
     * @param about surface normal. @warning MUST be normalized by the user!
     * @return reflection of %in with respect to %about
     */
    inline glm::vec3 reflect (const glm::vec3& in, const glm::vec3& about){return in - 2 * glm::dot(in, about)*about;}

    // TODO implement
    /**
     * @brief Linear mapping between two intervals
     * %val must be in %from.
     */
    [[nodiscard]] inline float map(float val, interval from, interval to = interval::NORM);
}
