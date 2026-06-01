#include "renderer/scene/Scene.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace renderer::scene {

Scene::Scene() {
    objects_ = {
        {{0.0f, 0.0f, 0.0f}, {0, 0, 0}, {1, 1, 1}, {1.0f, 0.3f, 0.3f}, 0.6f},
        {{2.5f, 0.0f, -1.0f}, {0, 0, 0}, {0.8f, 0.8f, 0.8f}, {0.3f, 0.8f, 0.4f}, 0.4f},
        {{-2.0f, 0.5f, 1.5f}, {0, 0, 0}, {0.6f, 0.6f, 0.6f}, {0.3f, 0.5f, 1.0f}, 0.8f},
        {{0.0f, -1.5f, 2.0f}, {0, 0, 0}, {1.2f, 0.3f, 1.2f}, {1.0f, 0.8f, 0.2f}, 0.3f},
        {{1.5f, 1.0f, 2.5f}, {0, 0, 0}, {0.5f, 0.5f, 0.5f}, {0.9f, 0.4f, 0.9f}, 1.0f},
    };
}

void Scene::update(float deltaTime) {
    time_ += deltaTime;
    for (auto& obj : objects_) {
        obj.rotation.y += obj.rotationSpeed * deltaTime;
        obj.rotation.x += obj.rotationSpeed * 0.3f * deltaTime;
    }
}

void Scene::draw(rendering::Shader& shader, const Camera& camera, float aspect) const {
    shader.use();
    shader.setVec3("lightPos", {2.0f, 4.0f, 2.0f});
    shader.setVec3("viewPos", camera.position());
    shader.setInt("diffuseMap", 0);

    const glm::mat4 projection = camera.projectionMatrix(aspect);
    const glm::mat4 view = camera.viewMatrix();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);

    cubeMesh_.texture().bind(0);

    for (const auto& obj : objects_) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, obj.position);
        model = glm::rotate(model, obj.rotation.x, {1, 0, 0});
        model = glm::rotate(model, obj.rotation.y, {0, 1, 0});
        model = glm::scale(model, obj.scale);
        shader.setMat4("model", model);
        shader.setVec3("objectColor", obj.color);
        cubeMesh_.draw();
    }
}

}  // namespace renderer::scene
