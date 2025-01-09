#pragma once
#include "glm/geometric.hpp"
#include "glm/glm.hpp"

namespace AiCo 
{
    namespace RT
    {
        struct ray
        {
            const glm::vec3 dir;
            const glm::vec3 origin;
            ray(const glm::vec3& dir, const glm::vec3& origin) : dir(glm::normalize(dir)), origin(origin) {}
            glm::vec3 at (float t)const{return t*dir + origin;}
        };
    }
}
