#include "renderer/ai/AiSuperResolver.hpp"

#ifdef AIER_HAS_OPENCV
#include <opencv2/imgproc.hpp>
#endif

#include <chrono>
#include <filesystem>
#include <iostream>

#ifdef AIER_USE_ONNXRUNTIME
#include <onnxruntime_cxx_api.h>
#endif

namespace renderer::ai {

bool AiSuperResolver::loadModel(const std::string& modelPath, UpscaleMethod method, int scale) {
    method_ = method;
    scale_ = scale;
    modelPath_ = modelPath;
    ready_ = false;

#ifdef AIER_HAS_OPENCV
    srImpl_.release();
#endif

    if (method == UpscaleMethod::Bilinear) {
        ready_ = true;
        return true;
    }

#ifdef AIER_USE_ONNXRUNTIME
    if (loadOnnxModel(modelPath)) {
        ready_ = true;
        return true;
    }
#endif

#ifdef AIER_HAS_OPENCV
    switch (method) {
        case UpscaleMethod::ESPCN:
            modelName_ = "espcn";
            break;
        case UpscaleMethod::FSRCNN:
            modelName_ = "fsrcnn";
            break;
        case UpscaleMethod::EDSR:
            modelName_ = "edsr";
            break;
        default:
            break;
    }

    if (modelPath.empty()) {
        return false;
    }

    if (!std::filesystem::exists(modelPath)) {
        std::cerr << "Model file not found: " << modelPath << '\n';
        srImpl_.release();
        ready_ = false;
        return false;
    }

    try {
        srImpl_ = cv::makePtr<cv::dnn_superres::DnnSuperResImpl>();
        srImpl_->readModel(modelPath);
        srImpl_->setModel(modelName_, scale_);
        ready_ = true;
        std::cout << "Loaded SR model: " << modelPath << " (" << modelName_ << " x" << scale_ << ")\n";
        return true;
    } catch (const cv::Exception& e) {
        std::cerr << "Failed to load SR model: " << e.what() << '\n';
        srImpl_.release();
        ready_ = false;
        return false;
    }
#else
    (void)modelPath;
    return false;
#endif
}

void AiSuperResolver::setMethod(UpscaleMethod method) {
    method_ = method;
    if (method == UpscaleMethod::Bilinear) {
        ready_ = true;
    }
}

#ifdef AIER_HAS_OPENCV
cv::Mat AiSuperResolver::upsample(const cv::Mat& inputRGBA) {
    if (inputRGBA.empty()) {
        return {};
    }

    const auto start = std::chrono::high_resolution_clock::now();

    cv::Mat result;
    if (method_ == UpscaleMethod::Bilinear || !ready_) {
        result = upsampleBilinear(inputRGBA);
    } else {
#ifdef AIER_USE_ONNXRUNTIME
        if (onnxSession_ != nullptr) {
            cv::Mat rgb;
            cv::cvtColor(inputRGBA, rgb, cv::COLOR_RGBA2RGB);
            cv::Mat upscaled = upsampleOnnx(rgb);
            cv::cvtColor(upscaled, result, cv::COLOR_RGB2RGBA);
        } else
#endif
        {
            result = upsampleModel(inputRGBA);
        }
    }

    const auto end = std::chrono::high_resolution_clock::now();
    lastInferenceMs_ = std::chrono::duration<double, std::milli>(end - start).count();
    return result;
}

cv::Mat AiSuperResolver::upsampleBilinear(const cv::Mat& inputRGBA) {
    cv::Mat output;
    cv::resize(inputRGBA, output,
               cv::Size(inputRGBA.cols * scale_, inputRGBA.rows * scale_), 0, 0, cv::INTER_LINEAR);
    return output;
}

cv::Mat AiSuperResolver::upsampleModel(const cv::Mat& inputRGBA) {
    if (!srImpl_) {
        return upsampleBilinear(inputRGBA);
    }

    cv::Mat rgb;
    cv::cvtColor(inputRGBA, rgb, cv::COLOR_RGBA2RGB);

    try {
        cv::Mat output;
        srImpl_->upsample(rgb, output);
        cv::Mat rgba;
        cv::cvtColor(output, rgba, cv::COLOR_RGB2RGBA);
        return rgba;
    } catch (const cv::Exception& e) {
        std::cerr << "OpenCV SR inference failed: " << e.what() << ", falling back to bilinear\n";
        return upsampleBilinear(inputRGBA);
    }
}
#endif

#ifdef AIER_USE_ONNXRUNTIME
bool AiSuperResolver::loadOnnxModel(const std::string& modelPath) {
    try {
        if (onnxEnv_ == nullptr) {
            onnxEnv_ = new Ort::Env(ORT_LOGGING_LEVEL_WARNING, "AiSuperResolver");
        }
        if (onnxSession_ != nullptr) {
            delete static_cast<Ort::Session*>(onnxSession_);
            onnxSession_ = nullptr;
        }
        Ort::SessionOptions options;
        options.SetIntraOpNumThreads(4);
        options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
        onnxSession_ = new Ort::Session(*static_cast<Ort::Env*>(onnxEnv_), modelPath.c_str(), options);
        return true;
    } catch (const Ort::Exception& e) {
        std::cerr << "ONNX load failed: " << e.what() << "\n";
        return false;
    }
}

cv::Mat AiSuperResolver::upsampleOnnx(const cv::Mat& inputRGB) {
    auto* session = static_cast<Ort::Session*>(onnxSession_);
    Ort::AllocatorWithDefaultOptions allocator;

    cv::Mat inputFloat;
    inputRGB.convertTo(inputFloat, CV_32F, 1.0 / 255.0);

    std::vector<int64_t> inputShape = {1, 3, inputFloat.rows, inputFloat.cols};
    std::vector<float> inputTensorValues(static_cast<std::size_t>(1 * 3 * inputFloat.rows * inputFloat.cols));

    for (int c = 0; c < 3; ++c) {
        for (int y = 0; y < inputFloat.rows; ++y) {
            for (int x = 0; x < inputFloat.cols; ++x) {
                inputTensorValues[static_cast<std::size_t>(c * inputFloat.rows * inputFloat.cols + y * inputFloat.cols + x)] =
                    inputFloat.at<cv::Vec3f>(y, x)[c];
            }
        }
    }

    Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    Ort::Value inputTensor = Ort::Value::CreateTensor<float>(memoryInfo, inputTensorValues.data(),
                                                              inputTensorValues.size(), inputShape.data(),
                                                              inputShape.size());

    auto inputName = session->GetInputNameAllocated(0, allocator);
    auto outputName = session->GetOutputNameAllocated(0, allocator);
    const char* inputNames[] = {inputName.get()};
    const char* outputNames[] = {outputName.get()};

    auto outputTensors = session->Run(Ort::RunOptions{nullptr}, inputNames, &inputTensor, 1, outputNames, 1);

    float* outputData = outputTensors[0].GetTensorMutableData<float>();
    auto outputShape = outputTensors[0].GetTensorTypeAndShapeInfo().GetShape();
    const int outH = static_cast<int>(outputShape[2]);
    const int outW = static_cast<int>(outputShape[3]);

    cv::Mat output(outH, outW, CV_32FC3);
    for (int c = 0; c < 3; ++c) {
        for (int y = 0; y < outH; ++y) {
            for (int x = 0; x < outW; ++x) {
                output.at<cv::Vec3f>(y, x)[c] =
                    outputData[static_cast<std::size_t>(c * outH * outW + y * outW + x)];
            }
        }
    }

    cv::Mat output8u;
    output.convertTo(output8u, CV_8U, 255.0);
    return output8u;
}
#endif

}  // namespace renderer::ai
