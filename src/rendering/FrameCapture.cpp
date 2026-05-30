#include "renderer/rendering/FrameCapture.hpp"
#include <glad/glad.h>
#include <cstring>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#ifdef AIER_HAS_OPENCV
#include <opencv2/imgproc.hpp>
#endif

namespace renderer::rendering {

    FrameCapture::~FrameCapture() {
        if (pbo_[0] != 0) {
            glDeleteBuffers(2, pbo_);
        }
    }
    void FrameCapture::initialize(int width, int height) {
        width_ = width;
        height_ = height;
        const GLsizeiptr size = static_cast<GLsizeiptr>(width_ * height_ * 4);

        if (pbo_[0] == 0) {
            glGenBuffers(2, pbo_);
        }

        for (int i = 0; i < 2; ++i) {
            glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo_[i]);

            glBufferData(GL_PIXEL_PACK_BUFFER, size, nullptr, GL_STREAM_READ);
        }



        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    }

    void FrameCapture::resize(int width, int height) {
        initialize(width, height);
        framesSubmitted_ = 0;
        writeIndex_ = 0;
    }

#ifdef AIER_HAS_OPENCV

    cv::Mat FrameCapture::captureFrame(int width, int height)
    {
        switch (backend_)
        {
        case CaptureBackend::CPU:
            return captureFrameCPU(width, height);

        case CaptureBackend::GPU:
            return captureFrameGPU(width, height);
        }

        return {};
    }

    cv::Mat FrameCapture::captureFrameCPU(int width, int height)
    {
        if (width != width_ || height != height_) {
            resize(width, height);
        }

        const int readIndex = (writeIndex_ + 1) % 2;

        glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo_[writeIndex_]);

        glReadPixels(
            0,
            0,
            width_,
            height_,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            nullptr);

        cv::Mat result;

        if (framesSubmitted_ > 0) {
            glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo_[readIndex]);

            glFinish();

            void* ptr =
                glMapBuffer(
                    GL_PIXEL_PACK_BUFFER,
                    GL_READ_ONLY);

            if (ptr) {
                result.create(
                    height_,
                    width_,
                    CV_8UC4);

                std::memcpy(
                    result.data,
                    ptr,
                    static_cast<std::size_t>(
                        width_ * height_ * 4));

                glUnmapBuffer(
                    GL_PIXEL_PACK_BUFFER);

                cv::flip(
                    result,
                    result,
                    0);
            }
        }

        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

        writeIndex_ = readIndex;
        ++framesSubmitted_;

        return result;
    }

    cv::Mat FrameCapture::captureFrameGPU(int width, int height)
    {
        // Na razie u¿ywa tej samej œcie¿ki.
        // PóŸniej podmienimy na VRAM-only path.
        return captureFrameCPU(width, height);
    }

#endif
}  //namespace renderer::rendering