#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "ray.h"

#include <cfloat>
#include <limits.h>

namespace AiCo 
{
    class geometry
    {
public:
        struct intersection_t
        {
            intersection_t(const ray& R, const glm::vec3& outwardNormal, const glm::vec3& P, float t) : 
            P(P), N(outwardNormal), t(t), frontFace(glm::dot(outwardNormal, R.dir) < 0) {}
            
            intersection_t() : P(), N(), t(-FLT_MAX), frontFace(false) {}

            glm::vec3 P, N;
            float t;
            bool frontFace;
        };

        intersection_t lastIntersect;
        virtual bool intersects(const ray& R, float tmin, float tmax) = 0;
    };

    class sphere : public geometry
    {
public:
        const float radius;
        const glm::vec3 center;

        sphere() = delete;
        sphere(float radius, glm::vec3 center) : radius(radius), center(center) {}


        virtual bool intersects(const ray& R, float tmin, float tmax) override
        {
            //just copied this code from RT in one weekend. should work
            glm::vec3 oc = center - R.origin;
            auto a = R.dir.length() * R.dir.length();
            auto h = glm::dot(R.dir, oc);
            auto c = oc.length()*oc.length() - radius*radius;

            auto discriminant = h*h - a*c;
            if (discriminant < 0)
                return false;

            auto sqrtd = std::sqrt(discriminant);

            auto root = (h - sqrtd) / a;
            if (root <= tmin || tmax <= root) 
            {
                root = (h + sqrtd) / a;
                if (root <= tmin || tmax <= root)
                    return false;
            }
            
            auto P = R.at(root);
            lastIntersect = intersection_t(R, (P - center)/radius, P, root);

            return true;
        }
    };
};

