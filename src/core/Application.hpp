#pragma once

#include "renderer/ai/AiSuperResolver.hpp"
#include "renderer/platform/Window.hpp"
#include "renderer/rendering/FrameCapture.hpp"
#include "renderer/rendering/Framebuffer.hpp"
#include "renderer/rendering/FullscreenQuad.hpp"
#include "renderer/rendering/Shader.hpp"
#include "renderer/rendering/Texture.hpp"
#include "renderer/scene/Camera.hpp"
#include "renderer/scene/Scene.hpp"
#include "renderer/utils/Timer.hpp"

#ifdef AIER_HAS_OPENCV
#include <opencv2/core.hpp>
#endif

#include <memory>
#include <string>

namespace renderer::core {

enum class DisplayMode { Bilinear, AI };

class Application {
public:
    Application();
    ~Application();
    void run();
    void onMouseMove(double x, double y);
    void onMouseButton(int button, int action);

private:
    void processInput(float deltaTime);
    void renderScene();
    void presentFrame();
    void renderUI();
    void setupInputCallbacks();
    void updateTimings(double frameMs);
    int renderWidth() const;
    int renderHeight() const;
    std::string shaderPath(const std::string& relative) const;
    std::string modelPathFor(ai::UpscaleMethod method, int scale) const;
    bool modelFileExists(ai::UpscaleMethod method, int scale) const;
    bool reloadActiveModel();

    std::unique_ptr<platform::Window> window_;
    scene::Camera camera_;
    std::unique_ptr<scene::Scene> scene_;

    rendering::Shader phongShader_;
    rendering::Shader quadShader_;
    rendering::Framebuffer sceneFbo_;
    rendering::FrameCapture frameCapture_;
    std::unique_ptr<rendering::FullscreenQuad> fullscreenQuad_;
    rendering::Texture resultTexture_;

    ai::AiSuperResolver superResolver_;
    utils::FpsCounter fpsCounter_;
    utils::PipelineTimings timings_;

    DisplayMode displayMode_ = DisplayMode::Bilinear;
    ai::UpscaleMethod aiMethod_ = ai::UpscaleMethod::FSRCNN;
    float resolutionScale_ = 0.33f; // Obniżone z 0.67f by potężnie podwyższyć wydajność na iGPU/mobile CPU
    int scaleFactor_ = 2;
    float lastMouseX_ = 0.0f;
    float lastMouseY_ = 0.0f;
    bool firstMouse_ = true;
    bool mouseCaptured_ = false;

    std::string projectRoot_;
    std::string modelStatus_;
    ai::UpscaleMethod lastLoadedMethod_ = ai::UpscaleMethod::FSRCNN;
    int lastLoadedScale_ = 2;
#ifdef AIER_HAS_OPENCV
    cv::Mat capturedFrame_;
#endif
};

}  // namespace renderer::core
