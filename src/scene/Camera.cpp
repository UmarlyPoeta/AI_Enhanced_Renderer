#include "renderer/scene/Camera.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

namespace renderer::scene {

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : position_(position), worldUp_(up), yaw_(yaw), pitch_(pitch) {
    updateVectors();
}

glm::mat4 Camera::viewMatrix() const {
    return glm::lookAt(position_, position_ + front_, up_);
}

glm::mat4 Camera::projectionMatrix(float aspect, float fov, float nearPlane, float farPlane) const {
    return glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
}

void Camera::processKeyboard(int direction, float deltaTime) {
    const float velocity = movementSpeed_ * deltaTime;
    if (direction == FORWARD) {
        position_ += front_ * velocity;
    }
    if (direction == BACKWARD) {
        position_ -= front_ * velocity;
    }
    if (direction == LEFT) {
        position_ -= right_ * velocity;
    }
    if (direction == RIGHT) {
        position_ += right_ * velocity;
    }
    if (direction == UP) {
        position_ += worldUp_ * velocity;
    }
    if (direction == DOWN) {
        position_ -= worldUp_ * velocity;
    }
}

void Camera::processMouse(float xoffset, float yoffset, float sensitivity) {
    xoffset *= sensitivity;
    yoffset *= sensitivity;
    yaw_ += xoffset;
    pitch_ += yoffset;
    pitch_ = std::clamp(pitch_, -89.0f, 89.0f);
    updateVectors();
}

void Camera::updateVectors() {
    glm::vec3 front;
    front.x = std::cos(glm::radians(yaw_)) * std::cos(glm::radians(pitch_));
    front.y = std::sin(glm::radians(pitch_));
    front.z = std::sin(glm::radians(yaw_)) * std::cos(glm::radians(pitch_));
    front_ = glm::normalize(front);
    right_ = glm::normalize(glm::cross(front_, worldUp_));
    up_ = glm::normalize(glm::cross(right_, front_));
}

}  // namespace renderer::scene
