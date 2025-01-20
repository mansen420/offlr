#pragma once

#include <cassert>
#include <cstddef>
#include <optional>
#include <vector>

#include "format.h"
#include "raytracing/ray.h"
#include "utils.h"
#include "intersection.h"

namespace AiCo
{
    namespace RT
    {
        class material
        {
        public:
            virtual ~material() = default;
            
            struct scatter_t
            {
                scatter_t(const ray& out, const color3f& attenuation) : out(out), attenuation(attenuation) {}
                scatter_t() = delete;
                const ray out;
                const color3f attenuation;
            };

            [[nodiscard]] virtual std::optional<scatter_t> scatter(const ray& insctRay, intersection_t insct)const = 0;
        };

        class lambertian_diffuse : public material
        {
        public:
            color3f albedo = {0, 0, 0};
            lambertian_diffuse(color3f albedo) : albedo(albedo) {}

            [[nodiscard]] virtual std::optional<scatter_t> scatter(const ray& insctRay, intersection_t insct)const
            {
                glm::vec3 scatterDir = insct.N + randvec_on_unit_sphere();
                if(nearzero_vec(scatterDir))
                    scatterDir = insct.N;
                return scatter_t(ray(scatterDir, insct.P), albedo);
            }
        };
        class material_registry
        {
            std::vector<material*> data;           
            std::vector<size_t> freeIndices;
        public:
            struct handle_t 
            {
            private:
                handle_t() = delete;
                handle_t(size_t id) : id(id) {}
                
                operator size_t(){return id;}
                
                size_t id; 
                
                friend class material_registry;
            };
            
            void remove_material(handle_t handle)
            {
                assert(handle < data.size() && data.at(handle) != nullptr);

                delete data.at(handle);
                data.at(handle) = nullptr;

                freeIndices.push_back(handle);
            }

            handle_t register_material(material* matptr)
            {
                assert(matptr != nullptr);
                if(freeIndices.empty())
                {
                    data.reserve(data.size() + 1);
                    data.push_back(matptr);
                    return handle_t{data.size() - 1};                   
                }
                else
                {
                    handle_t idx = freeIndices.back();
                    freeIndices.pop_back();

                    data.at(idx) = matptr;
                    return idx;
                }
            }
        };
    }
};
