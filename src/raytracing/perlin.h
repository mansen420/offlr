#pragma once
#include <glm/glm.hpp>
#include <algorithm>
#include <numeric>
#include <random>
#include <vector>

class perlin
{
public:
    explicit perlin(unsigned int seed = 1337)
    {
        p.resize(256);
        std::iota(p.begin(), p.end(), 0);

        std::mt19937 rng(seed);
        std::shuffle(p.begin(), p.end(), rng);

        // Duplicate the permutation vector
        p.insert(p.end(), p.begin(), p.end());
    }

    // 3D perlin noise
    float operator()(const glm::vec3& pos) const
    {
        int X = (int)floor(pos.x) & 255;
        int Y = (int)floor(pos.y) & 255;
        int Z = (int)floor(pos.z) & 255;

        float xf = pos.x - floor(pos.x);
        float yf = pos.y - floor(pos.y);
        float zf = pos.z - floor(pos.z);

        float u = fade(xf);
        float v = fade(yf);
        float w = fade(zf);

        int A  = p[X] + Y;
        int AA = p[A] + Z;
        int AB = p[A + 1] + Z;
        int B  = p[X + 1] + Y;
        int BA = p[B] + Z;
        int BB = p[B + 1] + Z;

        float res = lerp(w,
            lerp(v,
                lerp(u, grad(p[AA], xf, yf, zf),
                        grad(p[BA], xf - 1, yf, zf)),
                lerp(u, grad(p[AB], xf, yf - 1, zf),
                        grad(p[BB], xf - 1, yf - 1, zf))),
            lerp(v,
                lerp(u, grad(p[AA + 1], xf, yf, zf - 1),
                        grad(p[BA + 1], xf - 1, yf, zf - 1)),
                lerp(u, grad(p[AB + 1], xf, yf - 1, zf - 1),
                        grad(p[BB + 1], xf - 1, yf - 1, zf - 1)))
        );
        
        return (res + 1.0f) * 0.5f; // normalize to [0,1]
    }

private:
    std::vector<int> p;

    static float fade(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }
    static float lerp(float t, float a, float b) { return a + t * (b - a); }
    static float grad(int hash, float x, float y, float z)
    {
        int h = hash & 15;
        float u = h < 8 ? x : y;
        float v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
        return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
    }
};
inline perlin noise;
inline float fBm(glm::vec2 xz, const perlin& noise, int nrOctaves = 3, float amp = 2.f, float freq = 0.5f,
    float freqGain = 2.f, float ampGain = 0.5f)
{
    float sum = 0.f;
    for(size_t i = 0; i < nrOctaves; ++i)
    {
        sum += amp * noise( freq * glm::vec3{xz.x, 0.f, xz.y});
        amp *= ampGain;
        freq *= freqGain;
    }
    return sum;
}
