#pragma once

#include <glm/glm.hpp>

namespace renderer::scene {

class Camera {
public:
    Camera(glm::vec3 position = {0.0f, 0.0f, 5.0f}, glm::vec3 up = {0.0f, 1.0f, 0.0f}, float yaw = -90.0f,
           float pitch = 0.0f);

    [[nodiscard]] glm::mat4 viewMatrix() const;
    [[nodiscard]] glm::mat4 projectionMatrix(float aspect, float fov = 45.0f, float nearPlane = 0.1f,
                                           float farPlane = 100.0f) const;
    [[nodiscard]] glm::vec3 position() const { return position_; }
    [[nodiscard]] glm::vec3 front() const { return front_; }

    void processKeyboard(int direction, float deltaTime);
    void processMouse(float xoffset, float yoffset, float sensitivity = 0.1f);
    void setAspect(float aspect) { aspect_ = aspect; }

private:
    void updateVectors();

    glm::vec3 position_;
    glm::vec3 front_;
    glm::vec3 up_;
    glm::vec3 right_;
    glm::vec3 worldUp_;
    float yaw_;
    float pitch_;
    float aspect_ = 16.0f / 9.0f;
    float movementSpeed_ = 3.0f;
};

enum CameraMovement { FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN };

}  // namespace renderer::scene
