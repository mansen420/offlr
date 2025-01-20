#pragma once

#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "interval.h"
#include "ray.h"
#include "raytracing/material.h"
#include "intersection.h"

#include <cfloat>
#include <limits.h>
#include <memory.h>
#include <memory>
#include <optional>
#include <vector>

namespace AiCo 
{
    namespace RT 
    {
        class surface_mapper_base;

        typedef std::function<std::optional<intersection_t>(const ray& R, interval k)> intersector_t;
        class geometry
        {
        public:
            [[nodiscard]] virtual std::optional<intersection_t> test_intersect(const ray& R, interval K)const = 0;
            
            [[nodiscard]] virtual std::optional<intersection_t> operator()(const ray& R, interval K)const
            {
                return test_intersect(R, K);
            }

            virtual ~geometry() = default;
        };

        class sphere : public geometry
        {
        public:
            float radius;
            glm::vec3 center;

            sphere() = delete;
            sphere(float radius, glm::vec3 center) : radius(radius), center(center) {}

            [[nodiscard]] virtual std::optional<intersection_t> test_intersect(const ray& R, interval K)const override
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
            std::vector<intersector_t> list;
            
            nearest_hit_structure(std::vector<intersector_t> list) : list(list) {}

            [[nodiscard]] virtual std::optional<intersection_t> test_intersect(const ray& R, interval K)const override
            {
                float closestIntersect = K.min;
                for(const auto& insctr : list)
                {
                    auto insct = insctr(R, {closestIntersect, K.max});
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
