#pragma once

#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "ray.h"
#include "utils.h"

#include <cassert>
#include <cstddef>
#include <cstdio>
#include <functional>

namespace AiCo 
{
    namespace RT
    {
        typedef std::function<ray(size_t x, size_t y)> camera_t;
        
        class camera
        {
        public:
            virtual ray operator()(size_t x, size_t y)const = 0;
        };
        
        class static_camera : public camera
        {
            const glm::vec3 w, u, v;
            const glm::vec3 eye;
            const float viewportWidth, viewportHeight;
            const float pxWidth, pxHeight;
            const glm::vec3 topleft;
        public:
            static_camera(size_t imgWidth, size_t imgHeight, glm::vec3 origin, glm::vec3 lookat, float viewportWidth = 2.f,
            glm::vec3 worldUp = {0.f, 1.f, 0.f}):
            w(glm::normalize(origin - lookat)), u(glm::normalize(glm::cross(worldUp, w))), v(glm::normalize(glm::cross(w, u))),
            eye(origin), viewportWidth(viewportWidth), viewportHeight(float(imgHeight)/imgWidth * viewportWidth),
            pxWidth(viewportWidth/imgWidth), pxHeight(viewportHeight/imgHeight),
            topleft(eye + 0.5f * viewportHeight * v + 0.5f * viewportWidth * -u + glm::length(origin - lookat) * -w)
            {}

            ray operator()(size_t x, size_t y) const noexcept override
            {
                return ray(topleft + float(x + 0.5) * pxWidth * u - float(y + 0.5) *pxHeight * v, eye);
            }
        };

        class old_camera : public camera
        {
            const float viewportWidth;
            const float viewportHeight;
            const float pxdeltaU, pxdeltaV;
    public:
            const glm::vec3 eye;
            const glm::vec3 w, u, v; //canonical right, up, and 'inwards' vectors in camera space respectively
        
            const int imgWidth, imgHeight;
    private:
            const glm::vec3 topleftpx;
            const float defocusRadius;
            const glm::vec3 normU, normV, normW;
    public:
            old_camera() = delete;
            
            old_camera(float focalLength, int imgWidth, int imgHeight, float vFOV, float defocusAngle, 
            const glm::vec3& lookat = {0.f, 0.f, -1.f}, const glm::vec3& origin = {0, 0, 0}, 
            const glm::vec3& canonicalUp = {0.f, 1.f, 0.f}) : 
            viewportWidth(/* viewportHeight */ (2.f * focalLength * tanf(degrees_to_radians(vFOV/2.f))) * float(imgWidth)/imgHeight), 
            viewportHeight(viewportWidth * float(imgHeight)/imgWidth), 
            pxdeltaU(viewportWidth/imgWidth), pxdeltaV(viewportHeight/imgHeight),
            eye(origin),
            w(focalLength * glm::normalize(origin - lookat)),
            u(0.5f * viewportWidth * glm::normalize(glm::cross(canonicalUp, w))),
            v(0.5f * viewportHeight * glm::normalize(glm::cross(w, u))),
            imgWidth(imgWidth), imgHeight(imgHeight),
            topleftpx(eye - u + v - w),
            defocusRadius(focalLength * tanf(degrees_to_radians(defocusAngle/2.f))),
            normU(glm::normalize(u)), normV(glm::normalize(v)), normW(glm::normalize(w))
            {}

            old_camera(float focalLength, int imgWidth, int imgHeight, float defocusAngle, const glm::vec3& lookat = {0.f, 0.f, -1.f}, 
            const glm::vec3& origin = {0, 0, 0}, float viewportWidth = 2.f, const glm::vec3& canonicalUp = {0.f, 1.f, 0.f}) : 
            viewportWidth(viewportWidth), viewportHeight(float(imgHeight)/imgWidth * viewportWidth), 
            pxdeltaU(viewportWidth/imgWidth), pxdeltaV(viewportHeight/imgHeight),
            eye(origin),
            w(focalLength * glm::normalize(lookat - origin)),
            u(0.5f * viewportWidth * glm::normalize(glm::cross(canonicalUp, w))),
            v(0.5f * viewportHeight * glm::normalize(glm::cross(w, u))),
            imgWidth(imgWidth), imgHeight(imgHeight),
            topleftpx(eye - u + v + w),
            defocusRadius(focalLength * tanf(0.5 * degrees_to_radians(defocusAngle))),
            normU(glm::normalize(u)), normV(glm::normalize(v)), normW(glm::normalize(w))
            {}

            /**
             * @brief Returns ray through center of pixel (x, y) of the viewport
             *
             * @param x Horizontal index in [0, imgWidth)
             * @param y Vertical index in [0, imgHeight), positive downwards
             */
            [[nodiscard]] inline ray pixelCenter(int x, int y)const
            {
                assert(x < imgWidth && y < imgHeight);
                return ray(topleftpx + glm::vec3(x*pxdeltaU, -y*pxdeltaV, 0), eye);
            }
            /**
             * @brief Returns ray sample through the unit square surrounding pixel (x, y) of the viewport.
             *
             * @param x Horizontal index in [0, imgWidth)
             * @param y Vertical index in [0, imgHeight), positive downwards
             */
            [[nodiscard]] inline ray samplePixel(int x, int y)const
            {
                assert(x < imgWidth && y < imgHeight);
                using namespace glm;
                auto lensRand = randvec_in_unit_disk();
                glm::vec2 offset(rand() - 0.5f, rand() - 0.5f);
                return ray(topleftpx + vec3((x + offset.x) * pxdeltaU, (-y + offset.y) * pxdeltaV, 0),
                eye + defocusRadius*(lensRand.x * u + lensRand.y * v));
            }
            inline ray operator()(size_t x, size_t y)const {return pixelCenter(x, y);}
        };
    }
};
