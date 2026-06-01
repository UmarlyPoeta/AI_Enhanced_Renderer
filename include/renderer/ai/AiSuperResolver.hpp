#pragma once

#include <string>

#ifdef AIER_HAS_OPENCV
#include <opencv2/core.hpp>
#include <opencv2/dnn_superres.hpp>
#endif

namespace renderer::ai {

enum class UpscaleMethod { Bilinear, ESPCN, FSRCNN, EDSR };

class AiSuperResolver {
public:
    AiSuperResolver() = default;

    bool loadModel(const std::string& modelPath, UpscaleMethod method, int scale);
    void setMethod(UpscaleMethod method);

#ifdef AIER_HAS_OPENCV
    cv::Mat upsample(const cv::Mat& inputRGBA);
#endif

    [[nodiscard]] UpscaleMethod method() const { return method_; }
    [[nodiscard]] int scale() const { return scale_; }
    [[nodiscard]] bool isReady() const { return ready_; }
    [[nodiscard]] double lastInferenceMs() const { return lastInferenceMs_; }

private:
#ifdef AIER_HAS_OPENCV
    cv::Mat upsampleBilinear(const cv::Mat& inputRGBA);
    cv::Mat upsampleModel(const cv::Mat& inputRGBA);
    cv::Ptr<cv::dnn_superres::DnnSuperResImpl> srImpl_;
#endif

#ifdef AIER_USE_ONNXRUNTIME
    bool loadOnnxModel(const std::string& modelPath);
    cv::Mat upsampleOnnx(const cv::Mat& inputRGB);
    void* onnxSession_ = nullptr;
    void* onnxEnv_ = nullptr;
#endif

    UpscaleMethod method_ = UpscaleMethod::Bilinear;
    int scale_ = 2;
    bool ready_ = false;
    double lastInferenceMs_ = 0.0;
    std::string modelPath_;
    std::string modelName_;
};

}  // namespace renderer::ai
