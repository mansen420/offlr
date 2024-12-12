#include "glm/detail/qualifier.hpp"
#include "glm/glm.hpp"

namespace AiCo 
{
    template<glm::length_t L, typename T>
    [[nodiscard]] glm::vec<L, T> lerp (float alpha, glm::vec<L, T> a, glm::vec<L, T> b) {return (1-alpha)*a + (alpha)*b;}
}
