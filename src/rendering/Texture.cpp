#include "renderer/rendering/Texture.hpp"

#include <glad/glad.h>

#include <vector>

namespace renderer::rendering {

Texture::Texture(int width, int height, bool empty) : width_(width), height_(height) {
    glGenTextures(1, &textureId_);
    glBindTexture(GL_TEXTURE_2D, textureId_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    if (empty) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture() {
    if (textureId_ != 0) {
        glDeleteTextures(1, &textureId_);
    }
}

Texture::Texture(Texture&& other) noexcept
    : textureId_(other.textureId_), width_(other.width_), height_(other.height_) {
    other.textureId_ = 0;
}

Texture& Texture::operator=(Texture&& other) noexcept {
    if (this != &other) {
        if (textureId_ != 0) {
            glDeleteTextures(1, &textureId_);
        }
        textureId_ = other.textureId_;
        width_ = other.width_;
        height_ = other.height_;
        other.textureId_ = 0;
    }
    return *this;
}

void Texture::bind(unsigned int unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, textureId_);
}

void Texture::uploadRGBA(const unsigned char* data, int width, int height) {
    bind(0);
    if (width != width_ || height != height_) {
        width_ = width;
        height_ = height;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    } else {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width_, height_, GL_RGBA, GL_UNSIGNED_BYTE, data);
    }
}

void Texture::resize(int width, int height) {
    width_ = width;
    height_ = height;
    bind(0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
}

void Texture::generateCheckerboard(int width, int height, int tileSize) {
    width_ = width;
    height_ = height;
    std::vector<unsigned char> pixels(static_cast<std::size_t>(width * height * 4));
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const bool light = ((x / tileSize) + (y / tileSize)) % 2 == 0;
            const std::size_t idx = static_cast<std::size_t>((y * width + x) * 4);
            pixels[idx + 0] = light ? 220 : 40;
            pixels[idx + 1] = light ? 220 : 40;
            pixels[idx + 2] = light ? 220 : 40;
            pixels[idx + 3] = 255;
        }
    }
    bind(0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
}

}  // namespace renderer::rendering
