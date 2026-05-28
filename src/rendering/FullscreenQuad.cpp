#include "renderer/rendering/FullscreenQuad.hpp"

#include <glad/glad.h>

namespace renderer::rendering {

FullscreenQuad::FullscreenQuad()
    : mesh_({
          {{-1.0f, 1.0f, 0.0f}, {0, 0, 1}, {0.0f, 1.0f}},
          {{-1.0f, -1.0f, 0.0f}, {0, 0, 1}, {0.0f, 0.0f}},
          {{1.0f, -1.0f, 0.0f}, {0, 0, 1}, {1.0f, 0.0f}},
          {{-1.0f, 1.0f, 0.0f}, {0, 0, 1}, {0.0f, 1.0f}},
          {{1.0f, -1.0f, 0.0f}, {0, 0, 1}, {1.0f, 0.0f}},
          {{1.0f, 1.0f, 0.0f}, {0, 0, 1}, {1.0f, 1.0f}},
      }) {}

void FullscreenQuad::draw(Shader& shader, unsigned int textureId, float gamma, bool applyTonemap) const {
    shader.use();
    shader.setInt("screenTexture", 0);
    shader.setFloat("gamma", gamma);
    shader.setBool("applyTonemap", applyTonemap);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);
    mesh_.draw();
}

}  // namespace renderer::rendering
