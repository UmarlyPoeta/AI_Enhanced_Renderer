#pragma once

namespace renderer::rendering {

class Framebuffer {
public:
    Framebuffer() = default;
    Framebuffer(int width, int height);
    ~Framebuffer();

    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;
    Framebuffer(Framebuffer&& other) noexcept;
    Framebuffer& operator=(Framebuffer&& other) noexcept;

    void create(int width, int height);
    void destroy();
    void resize(int width, int height);
    void bind() const;
    void unbind() const;

    [[nodiscard]] unsigned int colorTexture() const { return colorTexture_; }
    [[nodiscard]] int width() const { return width_; }
    [[nodiscard]] int height() const { return height_; }
    [[nodiscard]] bool isValid() const { return fbo_ != 0; }

private:
    unsigned int fbo_ = 0;
    unsigned int colorTexture_ = 0;
    unsigned int depthTexture_ = 0;
    int width_ = 0;
    int height_ = 0;
};

}  // namespace renderer::rendering
