#pragma once

#ifdef AIER_HAS_OPENCV
#include <opencv2/core.hpp>
#endif

namespace renderer::rendering {

// Async PBO readback — call while the source FBO is still bound.

    enum class CaptureBackend
    {
        CPU,
        GPU
    };

class FrameCapture {
public:
    FrameCapture() = default;
    ~FrameCapture();

    void initialize(int width, int height);
    void resize(int width, int height);

#ifdef AIER_HAS_OPENCV
    [[nodiscard]] cv::Mat captureFrame(int width, int height);

private:
    cv::Mat captureFrameCPU(int width, int height);
    cv::Mat captureFrameGPU(int width, int height);

public:
#endif

    [[nodiscard]] int width() const { return width_; }
    [[nodiscard]] int height() const { return height_; }
    void setBackend(CaptureBackend backend) { backend_ = backend; }
    [[nodiscard]] CaptureBackend backend() const { return backend_; }
    [[nodiscard]] unsigned int sourceTexture() const
    {
        return sourceTexture_;
    }

    void setSourceTexture(unsigned int texture)
    {
        sourceTexture_ = texture;
    }

private:
    unsigned int pbo_[2]{0, 0};
    int writeIndex_ = 0;
    int width_ = 0;
    int height_ = 0;
    int framesSubmitted_ = 0;

    CaptureBackend backend_ = CaptureBackend::CPU;

    unsigned int sourceTexture_ = 0;
};

}  // namespace renderer::rendering
