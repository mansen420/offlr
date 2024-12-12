#pragma once

#include <glm/glm.hpp>

namespace AiCo
{
    typedef glm::vec4 color4f;
    typedef glm::vec3 color3f;
    typedef glm::vec<4, glm::uint8> RGBA32;
    inline RGBA32 colorftoRGBA32(const color3f& color){return RGBA32(color.r * 255.999, color.g * 255.999, color.b * 255.999, 255);}
    inline RGBA32 colorftoRGBA32(const color4f& color){return RGBA32(color.r * 255.999, color.g * 255.999, color.b * 255.999, color.a * 255.999);}
}
