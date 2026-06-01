#pragma once

namespace renderer::rendering {

class Texture {
public:
    Texture() = default;
    explicit Texture(int width, int height, bool empty = false);
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    void bind(unsigned int unit = 0) const;
    void uploadRGBA(const unsigned char* data, int width, int height);
    void resize(int width, int height);
    void generateCheckerboard(int width, int height, int tileSize = 32);

    [[nodiscard]] unsigned int id() const { return textureId_; }
    [[nodiscard]] int width() const { return width_; }
    [[nodiscard]] int height() const { return height_; }

private:
    unsigned int textureId_ = 0;
    int width_ = 0;
    int height_ = 0;
};

}  // namespace renderer::rendering
