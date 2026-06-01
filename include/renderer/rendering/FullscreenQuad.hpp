#pragma once

#include "renderer/rendering/Mesh.hpp"
#include "renderer/rendering/Shader.hpp"

namespace renderer::rendering {

class FullscreenQuad {
public:
    FullscreenQuad();
    void draw(Shader& shader, unsigned int textureId, float gamma = 2.2f, bool applyTonemap = false) const;

private:
    Mesh mesh_;
};

}  // namespace renderer::rendering
