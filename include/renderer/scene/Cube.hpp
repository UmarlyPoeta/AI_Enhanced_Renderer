#pragma once

#include "renderer/rendering/Mesh.hpp"
#include "renderer/rendering/Texture.hpp"

#include <glm/glm.hpp>

namespace renderer::scene {

class Cube {
public:
    Cube();

    void draw() const;
    [[nodiscard]] const rendering::Mesh& mesh() const { return mesh_; }
    [[nodiscard]] const rendering::Texture& texture() const { return texture_; }

private:
    rendering::Mesh mesh_;
    rendering::Texture texture_;
};

struct SceneObject {
    glm::vec3 position{0.0f};
    glm::vec3 rotation{0.0f};
    glm::vec3 scale{1.0f};
    glm::vec3 color{1.0f};
    float rotationSpeed = 0.5f;
};

}  // namespace renderer::scene
