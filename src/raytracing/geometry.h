#pragma once

#include "interval.h"
#include "ray.h"
#include "intersection.h"
#include "metrics.h"

#include <atomic>
#include <cfloat>
#include <cstddef>
#include <limits.h>
#include <optional>

namespace AiCo 
{
    namespace RT 
    {
        typedef std::function<std::optional<intersection_t>(ray R, interval k)> intersector_t;
        class geometry_base
        {
        public:
            [[nodiscard]] virtual std::optional<intersection_t> operator()(ray R, interval K)const = 0;

            virtual ~geometry_base() = default;
        };

        class sphere : public geometry_base
        {
        public:
            float radius;
            glm::vec3 center;

            const material_t& mat;

            sphere() = delete;
            sphere(float radius, glm::vec3 center, const material_t& mat) : radius(radius), center(center), mat(mat) {}
            
            [[nodiscard]] virtual std::optional<intersection_t> operator()(ray R, interval K)const override
            {
                return test_intersect(R, K);
            }
            
        private:
            [[nodiscard]] virtual std::optional<intersection_t> test_intersect(ray R, interval K)const
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
                return intersection_t(R, (P - center)/radius, P, root, mat);
            }
        };
        
        class nearest_intersect : public geometry_base
        {
        public:
            const std::vector<intersector_t>& list;
            
            nearest_intersect(const std::vector<intersector_t>& list) : list(list) {}

            [[nodiscard]] virtual std::optional<intersection_t> operator()(ray R, interval K)const override
            {
                return test_intersect(R, K);
            }

        private:
            [[nodiscard]] inline virtual std::optional<intersection_t> test_intersect(ray R, interval K)const
            {
                std::optional<intersection_t> result = {};
                float closestIntersect = K.max;
                for(const auto& insctr : list)
                {
                    LOCAL_INSCT_CNTR++;
                    if(LOCAL_INSCT_CNTR%(1<<16) == 0)
                        INSCT_CNTR.fetch_add(1<<16, std::memory_order_relaxed);
                    if(auto insct = insctr(R, {K.min, closestIntersect}); insct.has_value())
                        if(insct->t < closestIntersect)
                        {
                            closestIntersect = insct->t;
                            result.emplace(*insct);
                        }
                }
                return result;
            }
        };
    };
};
