#pragma once

#include "renderer/scene/Camera.hpp"
#include "renderer/scene/Cube.hpp"
#include "renderer/rendering/Shader.hpp"
#include "renderer/rendering/ModelLoader.hpp"
#include <memory>

#include <glm/glm.hpp>
#include <vector>

namespace renderer::scene {

class Scene {
public:
    Scene();

    void update(float deltaTime);
    void draw(rendering::Shader& shader, const Camera& camera, float aspect) const;

    [[nodiscard]] const std::vector<SceneObject>& objects() const { return objects_; }
    bool loadModel(const std::string& path);

private:
    Cube cubeMesh_;
    std::unique_ptr<rendering::Mesh> importedMesh_;
    std::vector<SceneObject> objects_;
    float time_ = 0.0f;
};

}  // namespace renderer::scene
