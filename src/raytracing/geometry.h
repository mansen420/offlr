#pragma once

#include "interval.h"
#include "ray.h"
#include "intersection.h"
#include "metrics.h"
#include "utils.h"

#include <atomic>
#include <cfloat>
#include <cstddef>
#include <functional>
#include <limits.h>
#include <optional>
#include <utility>

namespace AiCo 
{
    namespace RT 
    {
        typedef std::function<bool(ray R)> spatial_rejector_t;
        
        class AABB
        {
            glm::vec3 min, max;

            bool operator()(const ray& R)
            {
                //reject parallel rays that start outside the box
                if (R.dir.x == 0 && (R.origin.x < min.x || R.origin.x > max.x)) return false;
                if (R.dir.y == 0 && (R.origin.y < min.y || R.origin.y > max.y)) return false;
                if (R.dir.z == 0 && (R.origin.z < min.z || R.origin.z > max.z)) return false;
 
                auto invDir = 1.f/R.dir;
                
                float Xtmin = invDir.x * (min.x - R.origin.x);
                float Xtmax = invDir.x * (max.x - R.origin.x);
                if(invDir.x < 0.f) std::swap(Xtmin, Xtmax);
                
                if(Xtmin > Xtmax) return false;

                float Ytmin = invDir.y * (min.y - R.origin.y);
                float Ytmax = invDir.y * (max.y - R.origin.y);
                if(invDir.y < 0.f) std::swap(Ytmin, Ytmax);

                if(Ytmin > Ytmax) return false;
                
                auto XYoverlap = overlap({Xtmin, Xtmax}, {Ytmin, Ytmax});
                if(XYoverlap.empty()) return false;
                
                float Ztmin = invDir.z * (min.z - R.origin.z);
                float Ztmax = invDir.z * (max.z - R.origin.z);
                if(invDir.z < 0.f) std::swap(Ztmin, Ztmax);
                
                if(Ztmin > Ztmax) return false;

                auto XYZoverlap = overlap({Ztmin, Ztmax}, XYoverlap);

                return !XYZoverlap.empty();
            };
        };
        

        typedef std::function<std::optional<intersection_t>(ray R, interval k)> intersector_t;
        class geometry
        {
        public:
            [[nodiscard]] virtual std::optional<intersection_t> operator()(ray R, interval K)const = 0;

            virtual ~geometry() = default;
        };

        class sphere : public geometry
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
        
        class nearest_intersect : public geometry
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
                    // intersection count mechanism. add thread local variable to globabl atomic every 2^16 intersects
                    LOCAL_INSCT_CNTR++;
                    if(LOCAL_INSCT_CNTR%(1<<16) == 0)
                        INSCT_CNTR.fetch_add(1<<16, std::memory_order_relaxed);
                    //

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
