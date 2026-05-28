#include "renderer/rendering/FrameCapture.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <opencv2/imgproc.hpp>

#include <cassert>
#include <iostream>

int main() {
    if (!glfwInit()) {
        std::cerr << "GLFW init failed\n";
        return 1;
    }

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(64, 64, "test", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "GLAD init failed\n";
        return 1;
    }

    renderer::rendering::FrameCapture capture;
    capture.initialize(4, 4);

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    cv::Mat frame1 = capture.captureFrame(4, 4);
    assert(frame1.empty());

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    cv::Mat frame2 = capture.captureFrame(4, 4);
    assert(!frame2.empty());
    assert(frame2.cols == 4 && frame2.rows == 4);

    cv::Mat rgb;
    cv::cvtColor(frame2, rgb, cv::COLOR_RGBA2RGB);
    cv::Scalar mean = cv::mean(rgb);
    assert(mean[0] > 200.0);

    glfwDestroyWindow(window);
    glfwTerminate();
    std::cout << "test_frame_capture: OK\n";
    return 0;
}
