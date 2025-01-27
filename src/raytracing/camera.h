#pragma once

#include "glm/geometric.hpp"
#include "ray.h"
#include "utils.h"

#include <cassert>

namespace AiCo 
{
    namespace RT
    {
        class camera
        {
            const float viewportWidth = 2.f; //arbitrary
            const float viewportHeight;
            const float pxdeltaU, pxdeltaV;
    public:
            const glm::vec3 eye;
            const glm::vec3 w, u, v; //canonical right, up, and 'inwards' vectors in camera space respectively
            
            const int imgWidth, imgHeight;
    private:
            const glm::vec3 topleftpx;
    public:
            camera() = delete;

            camera(float focalLength, int imgWidth, int imgHeight, float vFOV, const glm::vec3& lookat = {0.f, 0.f, -1.f}, 
            const glm::vec3& origin = {0, 0, 0}, const glm::vec3& canonicalUp = {0.f, 1.f, 0.f}) : 
            viewportWidth(/* viewportHeight */ (2.f * focalLength * tanf(0.5 * degrees_to_radians(vFOV))) * float(imgWidth)/imgHeight), 
            viewportHeight(viewportWidth * float(imgHeight)/imgWidth), 
            pxdeltaU(viewportWidth/imgWidth), pxdeltaV(viewportHeight/imgHeight),
            eye(origin),
            w(focalLength * glm::normalize(lookat)),
            u(0.5f * viewportWidth * glm::normalize(glm::cross(w, canonicalUp))),
            v(0.5f * viewportHeight * glm::normalize(glm::cross(u, w))),
            imgWidth(imgWidth), imgHeight(imgHeight),
            topleftpx(eye - u + v + w + 0.5f*glm::vec3(pxdeltaU, pxdeltaV, 0))
            {}

            camera(float focalLength, int imgWidth, int imgHeight, const glm::vec3& lookat = {0.f, 0.f, -1.f}, 
            const glm::vec3& origin = {0, 0, 0}, float viewportWidth = 2.f, const glm::vec3& canonicalUp = {0.f, 1.f, 0.f}) : 
            viewportWidth(viewportWidth), viewportHeight(float(imgHeight)/imgWidth * viewportWidth), 
            pxdeltaU(viewportWidth/imgWidth), pxdeltaV(viewportHeight/imgHeight),
            eye(origin),
            w(focalLength * glm::normalize(lookat)),
            u(0.5f * viewportWidth * glm::normalize(glm::cross(w, canonicalUp))),
            v(0.5f * viewportHeight * glm::normalize(glm::cross(u, w))),
            imgWidth(imgWidth), imgHeight(imgHeight),
            topleftpx(eye - u + v + w + 0.5f*glm::vec3(pxdeltaU, pxdeltaV, 0))
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
                glm::vec2 offset(AiCo::rand() - 0.5f, AiCo::rand() - 0.5f);
                return ray(topleftpx + glm::vec3((x + offset.x) * pxdeltaU, (-y + offset.y) * pxdeltaV, 0), eye);
            }
        };
    }
};
