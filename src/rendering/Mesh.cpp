#include "renderer/rendering/Mesh.hpp"

#include <glad/glad.h>

namespace renderer::rendering {

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) {
    indexed_ = !indices.empty();
    indexCount_ = indexed_ ? indices.size() : vertices.size();

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    if (indexed_) {
        glGenBuffers(1, &ebo_);
    }

    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)), vertices.data(),
                 GL_STATIC_DRAW);

    if (indexed_) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)),
                     indices.data(), GL_STATIC_DRAW);
    }

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(sizeof(glm::vec3)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(sizeof(glm::vec3) * 2));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(sizeof(glm::vec3) * 2 + sizeof(glm::vec2)));

    glBindVertexArray(0);
}

Mesh::~Mesh() {
    if (ebo_ != 0) {
        glDeleteBuffers(1, &ebo_);
    }
    if (vbo_ != 0) {
        glDeleteBuffers(1, &vbo_);
    }
    if (vao_ != 0) {
        glDeleteVertexArrays(1, &vao_);
    }
}

void Mesh::draw() const {
    glBindVertexArray(vao_);
    if (indexed_) {
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indexCount_), GL_UNSIGNED_INT, nullptr);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(indexCount_));
    }
    glBindVertexArray(0);
}

}  // namespace renderer::rendering
