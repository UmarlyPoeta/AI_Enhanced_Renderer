#pragma once

#include <glm/glm.hpp>

#include <vector>

namespace renderer::rendering {

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
    glm::vec3 color{1.0f};
};

class Mesh {
public:
    Mesh() = default;
    explicit Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices = {});

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    ~Mesh();

    void draw() const;
    [[nodiscard]] std::size_t indexCount() const { return indexCount_; }

private:
    unsigned int vao_ = 0;
    unsigned int vbo_ = 0;
    unsigned int ebo_ = 0;
    std::size_t indexCount_ = 0;
    bool indexed_ = false;
};

}  // namespace renderer::rendering
