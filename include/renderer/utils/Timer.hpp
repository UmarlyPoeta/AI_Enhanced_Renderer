#pragma once

#include <chrono>

namespace renderer::utils {

class Timer {
public:
    Timer() { reset(); }

    void reset() { start_ = std::chrono::high_resolution_clock::now(); }

    [[nodiscard]] double elapsedMs() const {
        const auto now = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(now - start_).count();
    }

private:
    std::chrono::high_resolution_clock::time_point start_;
};

struct PipelineTimings {
    double renderMs = 0.0;
    double readbackMs = 0.0;
    double aiMs = 0.0;
    double uploadMs = 0.0;
    double presentMs = 0.0;
    double totalMs = 0.0;
    double fps = 0.0;
};

class FpsCounter {
public:
    void tick() {
        ++frameCount_;
        const double elapsed = timer_.elapsedMs();
        if (elapsed >= 1000.0) {
            fps_ = frameCount_ * 1000.0 / elapsed;
            frameCount_ = 0;
            timer_.reset();
        }
    }

    [[nodiscard]] double fps() const { return fps_; }

private:
    Timer timer_;
    int frameCount_ = 0;
    double fps_ = 0.0;
};

}  // namespace renderer::utils
