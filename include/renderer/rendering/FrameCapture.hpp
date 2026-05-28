#pragma once

#ifdef AIER_HAS_OPENCV
#include <opencv2/core.hpp>
#endif

namespace renderer::rendering {

// Async PBO readback — call while the source FBO is still bound.
class FrameCapture {
public:
    FrameCapture() = default;
    ~FrameCapture();

    void initialize(int width, int height);
    void resize(int width, int height);

#ifdef AIER_HAS_OPENCV
    // Issues glReadPixels into a ping-pong PBO and returns the *previous* frame (one-frame latency).
    [[nodiscard]] cv::Mat captureFrame(int width, int height);
#endif

    [[nodiscard]] int width() const { return width_; }
    [[nodiscard]] int height() const { return height_; }

private:
    unsigned int pbo_[2]{0, 0};
    int writeIndex_ = 0;
    int width_ = 0;
    int height_ = 0;
    int framesSubmitted_ = 0;
};

}  // namespace renderer::rendering
