#pragma once

struct GLFWwindow;

namespace renderer::platform {

class Window {
public:
    Window(int width, int height, const char* title);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    [[nodiscard]] bool shouldClose() const;
    void swapBuffers();
    void pollEvents();
    [[nodiscard]] GLFWwindow* handle() const { return window_; }
    [[nodiscard]] int width() const { return width_; }
    [[nodiscard]] int height() const { return height_; }
    void setFramebufferSizeCallback();

private:
    GLFWwindow* window_ = nullptr;
    int width_;
    int height_;
};

}  // namespace renderer::platform
