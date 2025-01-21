#pragma once

#include <vector>
#include <cstddef>

namespace AiCo
{
    template <typename T>
    class registry
    {
        std::vector<T*> data;           
        std::vector<size_t> freeIndices;
    public:
        struct handle_t 
        {
        private:
            handle_t() = delete;
            handle_t(size_t id) : id(id) {}
            
            operator size_t(){return id;}
            
            size_t id; 
            
            friend class registry<T>;
        };
        
        void remove(handle_t handle)
        {
            assert(handle < data.size() && data.at(handle) != nullptr);

            delete data.at(handle);
            data.at(handle) = nullptr;

            freeIndices.push_back(handle);
        }
        handle_t add(T* Tptr)
        {
            assert(Tptr != nullptr);
            if(freeIndices.empty())
            {
                data.reserve(data.size() + 1);
                data.push_back(Tptr);
                return handle_t{data.size() - 1};                   
            }
            else
            {
                handle_t idx = freeIndices.back();
                freeIndices.pop_back();

                data.at(idx) = Tptr;
                return idx;
            }
        }
    };
}

