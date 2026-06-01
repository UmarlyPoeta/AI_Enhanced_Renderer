#pragma once

#include "renderer/scene/Camera.hpp"
#include "renderer/scene/Cube.hpp"
#include "renderer/rendering/Shader.hpp"

#include <glm/glm.hpp>
#include <vector>

namespace renderer::scene {

class Scene {
public:
    Scene();

    void update(float deltaTime);
    void draw(rendering::Shader& shader, const Camera& camera, float aspect) const;

    [[nodiscard]] const std::vector<SceneObject>& objects() const { return objects_; }

private:
    Cube cubeMesh_;
    std::vector<SceneObject> objects_;
    float time_ = 0.0f;
};

}  // namespace renderer::scene
