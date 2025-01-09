#pragma once

#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "interval.h"
#include "ray.h"

#include <cfloat>
#include <limits.h>
#include <memory.h>
#include <memory>
#include <optional>

namespace AiCo 
{
    namespace RT 
    {
        class geometry
        {
        public:
            struct intersection_t
            {
                intersection_t(const ray& R, const glm::vec3& outwardNormal, const glm::vec3& P, float t) : 
                P(P), N(outwardNormal), t(t), frontFace(glm::dot(outwardNormal, R.dir) < 0){}
                
                intersection_t() = delete;

                const glm::vec3 P, N;
                const float t;
                const bool frontFace;
            };
            [[nodiscard]] virtual std::optional<intersection_t> testIntersect(const ray& R, interval K)const = 0;
            virtual ~geometry() = default;
        };

        class sphere : public geometry
        {
        public:
            float radius;
            glm::vec3 center;

            sphere() = delete;
            sphere(float radius, glm::vec3 center) : radius(radius), center(center) {}

            [[nodiscard]] virtual std::optional<intersection_t> testIntersect(const ray& R, interval K)const override
            {
                //just copied this code from RT in one weekend. should work
                glm::vec3 oc = center - R.origin;
                auto a = glm::dot(R.dir, R.dir);
                auto h = glm::dot(R.dir, oc);
                auto c = glm::dot(oc, oc) - radius*radius;

                auto discriminant = h*h - a*c;
                if (discriminant < 0)
                    return {};

                auto sqrtd = std::sqrt(discriminant);

                auto root = (h - sqrtd) / a;
                if (!K.contains(root)) 
                {
                    root = (h + sqrtd) / a;
                    if (!K.contains(root))
                        return {};
                }
                
                glm::vec3 P = R.at(root);
                return intersection_t(R, (P - center)/radius, P, root);
            }
        };

        class nearest_hit_structure : public geometry
        {
        public:
            std::vector<std::shared_ptr<geometry>> list;

            [[nodiscard]] virtual std::optional<intersection_t> testIntersect(const ray& R, interval K)const override
            {
                float closestIntersect = K.min;
                for(const auto& obj : list)
                {
                    auto insct = obj->testIntersect(R, {closestIntersect, K.max});
                    if(insct.has_value())
                    {
                        closestIntersect = insct->t;
                        return insct;
                    }
                }
                return {};
            }
        };
    };
};
