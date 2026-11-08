#pragma once

#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "interval.h"
#include "ray.h"
#include "intersection.h"
#include "metrics.h"
#include "utils.h"
#include "perlin.h"

#include <atomic>
#include <cfloat>
#include <cstddef>
#include <functional>
#include <limits.h>
#include <numeric>
#include <optional>
#include <utility>

namespace AiCo 
{
    namespace RT 
    {
        typedef std::function<bool(ray R)> spatial_rejector_t;
        
        inline glm::vec3 perturb_normal(const glm::vec3& N, const glm::vec3 P, float intensity = 1.5f,
            float freq = 5.f)
        {
            float eps = 1e-3f;
            float n0 = noise(P * freq);
            float nx = noise(P * freq + glm::vec3(eps,0,0));
            float nz = noise(P * freq + glm::vec3(0,0,eps));
            return normalize(N + intensity * glm::vec3(n0 - nx, 0, n0 - nz));
        }

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
                glm::vec3 N = (P - center) / radius; // surface normal
                float u = 0.5f + atan2(N.z, N.x) / (2.0f * M_PI);
                float v = 0.5f - asin(N.y) / M_PI;   // alternative: v = N.y * 0.5f + 0.5f

                return intersection_t(R, N, P, root, glm::vec2(u,v), mat);
            }
        };
        
        class cylinder : public geometry
        {
        public:
            float radius;
            float height;
            glm::vec3 center; // cylinder center
            const material_t& mat;

            cylinder() = delete;
            cylinder(float radius, float height, glm::vec3 center, const material_t& mat)
                : radius(radius), height(height), center(center), mat(mat) {}

            [[nodiscard]] virtual std::optional<intersection_t> operator()(ray R, interval K) const override
            {
                return test_intersect(R, K);
            }

        private:
            [[nodiscard]] virtual std::optional<intersection_t> test_intersect(ray R, interval K) const
            {
                // Transform ray into cylinder's local space (centered at origin)
                glm::vec3 o = R.origin - center;
                glm::vec3 d = R.dir;

                // Quadratic coefficients for x^2 + z^2 = r^2
                float a = d.x * d.x + d.z * d.z;
                float b = 2.0f * (o.x * d.x + o.z * d.z);
                float c = o.x * o.x + o.z * o.z - radius * radius;

                float disc = b * b - 4 * a * c;
                if (disc < 0) return {};

                float sqrtDisc = std::sqrt(disc);

                // pick smallest valid root
                float t0 = (-b - sqrtDisc) / (2 * a);
                float t1 = (-b + sqrtDisc) / (2 * a);

                float t = (K.contains(t0)) ? t0 : (K.contains(t1) ? t1 : std::numeric_limits<float>::quiet_NaN());
                if (std::isnan(t)) return {};

                glm::vec3 P = R.at(t);

                // check finite height: clamp around center.y ± height/2
                if (P.y < center.y - height * 0.5f || P.y > center.y + height * 0.5f) return {};

                // surface normal (radial, ignore y)
                glm::vec3 N = glm::normalize(glm::vec3(P.x - center.x, 0.0f, P.z - center.z));

                // simple UV mapping
                float u = 0.5f + atan2(P.z - center.z, P.x - center.x) / (2.0f * M_PI);
                float v = (P.y - (center.y - height * 0.5f)) / height;
                

                return intersection_t(R, N, P, t, glm::vec2(u, v), mat);
            }
        };

        class plane : public geometry
        {
        public:
            glm::vec3 N;//the plane normal
            glm::vec3 P0;//a point on the plane

            const material_t& mat;

            plane() = delete;
            plane(glm::vec3 normal, glm::vec3 position, const material_t& mat) :
                N(glm::normalize(normal)), 
                P0(position), 
                mat(mat) {}
            
            [[nodiscard]] virtual std::optional<intersection_t> operator()(ray R, interval K)const override
            {
                return test_intersect(R, K);
            }
            
        private:
            [[nodiscard]] virtual std::optional<intersection_t> test_intersect(ray R, interval K)const
            {
                float denominator = glm::dot(N, R.dir);
                if(fabs(denominator)<1e-6) /*ray parallel*/ return {};
                float numerator = glm::dot(N, P0) - glm::dot(N, R.origin);
                float t = numerator/denominator;
                if(!K.contains(t)) /*out of bounds*/ return {};
                return intersection_t {R, N, R.at(t), t, /*UV coords*/{}, mat};
            }
        };
        class noisy_terrain : public geometry
        {
        public:
            const material_t& mat;

            noisy_terrain(const material_t& mat) : mat(mat) {}            

            [[nodiscard]] virtual std::optional<intersection_t> operator()(ray R, interval K)const override
            {
                return test_intersect(R, K);
            }
            
        private:
            [[nodiscard]] virtual std::optional<intersection_t> test_intersect(ray R, interval K)const
            {
                //if (R.dir.y > 0.f) return {};

                float t0 = K.min, t1 = K.max == INF? 10000 : K.max;
                
                auto p0 = R.at(t0), p1 = R.at(t1);
                float y0 = R.origin.y + t0 * R.dir.y - fBm({p0.x, p0.z}, noise);
                float y1 = R.origin.y + t1 * R.dir.y - fBm({p1.x, p1.z}, noise);
                if (y0 * y1 > 0.0f)
                    return {}; // No sign change → no intersection

                for (int i = 0; i < 32; ++i) 
                {
                    float tMid = 0.5f * (t0 + t1);
                    glm::vec3 p = R.at(tMid);
                    float h = fBm({p.x, p.z}, noise);
                    if (p.y < h)
                        t1 = tMid;
                    else
                        t0 = tMid;
                }
                float t = t1;
                auto P = R.at(t);
    
                float h0 = fBm({P.x, P.z}, noise);

                //calc finite differences
                float epsilon = 1e-3f;
                float hx = fBm({P.x + epsilon, P.z}, noise);
                float hz = fBm({P.x, P.z +  epsilon}, noise);
                float dhdx = (hx - h0)/epsilon, dhdz = (hz - h0)/epsilon;
                glm::vec3 perturbedN{-dhdx, 1.f, -dhdz};
                perturbedN = glm::normalize(perturbedN);
                
                return intersection_t(R, perturbedN, P, t, {}, mat);
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
