#include "core/Application.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#ifdef AIER_HAS_IMGUI
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#endif

#ifdef AIER_HAS_OPENCV
#include <opencv2/imgcodecs.hpp>
#endif

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace renderer::core {

static void glfwErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << '\n';
}

Application::Application() {
    glfwSetErrorCallback(glfwErrorCallback);

    projectRoot_ = AIER_PROJECT_ROOT;
    window_ = std::make_unique<platform::Window>(1280, 720, "AI-Enhanced Renderer");

    phongShader_ = rendering::Shader(shaderPath("scene/phong.vert"), shaderPath("scene/phong.frag"));
    quadShader_ = rendering::Shader(shaderPath("postprocess/quad.vert"), shaderPath("postprocess/quad.frag"));

    scene_ = std::make_unique<scene::Scene>();
    fullscreenQuad_ = std::make_unique<rendering::FullscreenQuad>();

    const int rw = renderWidth();
    const int rh = renderHeight();
    sceneFbo_.create(rw, rh);
    frameCapture_.initialize(rw, rh);
    resultTexture_ = rendering::Texture(window_->width(), window_->height(), true);

    const std::string modelDir = projectRoot_ + "/models";
    if (reloadActiveModel()) {
        lastLoadedMethod_ = aiMethod_;
        lastLoadedScale_ = scaleFactor_;
    } else {
        modelStatus_ = "FSRCNN x2 unavailable — using bilinear. Run models/download_models.sh";
        aiMethod_ = ai::UpscaleMethod::Bilinear;
        superResolver_.loadModel("", ai::UpscaleMethod::Bilinear, scaleFactor_);
    }

#ifdef AIER_HAS_IMGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui_ImplGlfw_InitForOpenGL(window_->handle(), false);
    ImGui_ImplOpenGL3_Init("#version 330 core");
    setupInputCallbacks();
#else
    glfwSetWindowUserPointer(window_->handle(), this);
    glfwSetCursorPosCallback(window_->handle(), [](GLFWwindow* w, double x, double y) {
        static_cast<Application*>(glfwGetWindowUserPointer(w))->onMouseMove(x, y);
    });
    glfwSetMouseButtonCallback(window_->handle(), [](GLFWwindow* w, int button, int action, int /*mods*/) {
        static_cast<Application*>(glfwGetWindowUserPointer(w))->onMouseButton(button, action);
    });
#endif
}

Application::~Application() {
#ifdef AIER_HAS_IMGUI
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
#endif
}

void Application::setupInputCallbacks() {
#ifdef AIER_HAS_IMGUI
    GLFWwindow* w = window_->handle();
    glfwSetWindowUserPointer(w, this);

    glfwSetMouseButtonCallback(w, [](GLFWwindow* win, int button, int action, int mods) {
        ImGui_ImplGlfw_MouseButtonCallback(win, button, action, mods);
        auto* app = static_cast<Application*>(glfwGetWindowUserPointer(win));
        if (!ImGui::GetIO().WantCaptureMouse) {
            app->onMouseButton(button, action);
        }
    });

    glfwSetCursorPosCallback(w, [](GLFWwindow* win, double x, double y) {
        ImGui_ImplGlfw_CursorPosCallback(win, x, y);
        auto* app = static_cast<Application*>(glfwGetWindowUserPointer(win));
        if (!ImGui::GetIO().WantCaptureMouse) {
            app->onMouseMove(x, y);
        }
    });

    glfwSetScrollCallback(w, ImGui_ImplGlfw_ScrollCallback);
    glfwSetKeyCallback(w, ImGui_ImplGlfw_KeyCallback);
    glfwSetCharCallback(w, ImGui_ImplGlfw_CharCallback);
#endif
}

void Application::run() {
    utils::Timer frameTimer;

    while (!window_->shouldClose()) {
        window_->pollEvents();

#ifdef AIER_HAS_IMGUI
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
#endif

        frameTimer.reset();
        const float deltaTime = 0.016f;

        processInput(deltaTime);
        scene_->update(deltaTime);

        utils::Timer stageTimer;

        renderScene();
        timings_.renderMs = stageTimer.elapsedMs();

#ifdef AIER_HAS_OPENCV
        if (displayMode_ == DisplayMode::AI && superResolver_.isReady()) {
            stageTimer.reset();
            cv::Mat frame = capturedFrame_;
            if (!frame.empty()) {
                timings_.readbackMs = stageTimer.elapsedMs();
                stageTimer.reset();

                cv::Mat upscaled = superResolver_.upsample(frame);
                timings_.aiMs = superResolver_.lastInferenceMs();

                if (!upscaled.empty()) {
                    stageTimer.reset();
                    resultTexture_.uploadRGBA(upscaled.data, upscaled.cols, upscaled.rows);
                    timings_.uploadMs = stageTimer.elapsedMs();
                }
            }
        } else {
            timings_.readbackMs = 0;
            timings_.aiMs = 0;
            timings_.uploadMs = 0;
        }
#endif

        stageTimer.reset();
        presentFrame();
        timings_.presentMs = stageTimer.elapsedMs();

#ifdef AIER_HAS_IMGUI
        renderUI();
#endif

        window_->swapBuffers();

        const double frameMs = frameTimer.elapsedMs();
        updateTimings(frameMs);
        fpsCounter_.tick();
    }
}

void Application::onMouseMove(double x, double y) {
    if (!mouseCaptured_) {
        return;
    }
    if (firstMouse_) {
        lastMouseX_ = static_cast<float>(x);
        lastMouseY_ = static_cast<float>(y);
        firstMouse_ = false;
    }
    const float xoffset = static_cast<float>(x) - lastMouseX_;
    const float yoffset = lastMouseY_ - static_cast<float>(y);
    lastMouseX_ = static_cast<float>(x);
    lastMouseY_ = static_cast<float>(y);
    camera_.processMouse(xoffset, yoffset);
}

void Application::onMouseButton(int button, int action) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        mouseCaptured_ = !mouseCaptured_;
        glfwSetInputMode(window_->handle(), GLFW_CURSOR,
                         mouseCaptured_ ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        if (mouseCaptured_) {
            firstMouse_ = true;
        }
    }
}

void Application::processInput(float deltaTime) {
#ifdef AIER_HAS_IMGUI
    if (ImGui::GetIO().WantCaptureKeyboard) {
        return;
    }
#endif

    GLFWwindow* w = window_->handle();
    if (glfwGetKey(w, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(w, true);
    }
    if (glfwGetKey(w, GLFW_KEY_W) == GLFW_PRESS) {
        camera_.processKeyboard(scene::FORWARD, deltaTime);
    }
    if (glfwGetKey(w, GLFW_KEY_S) == GLFW_PRESS) {
        camera_.processKeyboard(scene::BACKWARD, deltaTime);
    }
    if (glfwGetKey(w, GLFW_KEY_A) == GLFW_PRESS) {
        camera_.processKeyboard(scene::LEFT, deltaTime);
    }
    if (glfwGetKey(w, GLFW_KEY_D) == GLFW_PRESS) {
        camera_.processKeyboard(scene::RIGHT, deltaTime);
    }
    if (glfwGetKey(w, GLFW_KEY_Q) == GLFW_PRESS) {
        camera_.processKeyboard(scene::UP, deltaTime);
    }
    if (glfwGetKey(w, GLFW_KEY_E) == GLFW_PRESS) {
        camera_.processKeyboard(scene::DOWN, deltaTime);
    }
}

void Application::renderScene() {
    sceneFbo_.bind();
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.08f, 0.09f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const float aspect = static_cast<float>(renderWidth()) / static_cast<float>(renderHeight());
    scene_->draw(phongShader_, camera_, aspect);

#ifdef AIER_HAS_OPENCV
    capturedFrame_ = frameCapture_.captureFrame(renderWidth(), renderHeight());
#endif

    sceneFbo_.unbind();
}

void Application::presentFrame() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, window_->width(), window_->height());
    glDisable(GL_DEPTH_TEST);
    glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (displayMode_ == DisplayMode::Bilinear || resultTexture_.id() == 0) {
        fullscreenQuad_->draw(quadShader_, sceneFbo_.colorTexture(), 2.2f, false);
    } else {
        fullscreenQuad_->draw(quadShader_, resultTexture_.id(), 2.2f, false);
    }
}

void Application::renderUI() {
#ifdef AIER_HAS_IMGUI
    ImGui::Begin("AI-Enhanced Renderer");
    ImGui::Text("FPS: %.1f", fpsCounter_.fps());
    ImGui::Separator();

    const char* displayModes[] = {"Bilinear (FBO upscale)", "AI Super-Resolution"};
    int displayModeInt = static_cast<int>(displayMode_);
    if (ImGui::Combo("Display Mode", &displayModeInt, displayModes, 2)) {
        displayMode_ = static_cast<DisplayMode>(displayModeInt);
    }

    const ai::UpscaleMethod prevMethod = aiMethod_;
    const int prevScale = scaleFactor_;

    const char* aiMethods[] = {"Bilinear", "ESPCN", "FSRCNN", "EDSR"};
    int methodInt = static_cast<int>(aiMethod_);
    if (ImGui::Combo("AI Model", &methodInt, aiMethods, 4)) {
        aiMethod_ = static_cast<ai::UpscaleMethod>(methodInt);
        if (!reloadActiveModel()) {
            aiMethod_ = prevMethod;
            scaleFactor_ = lastLoadedScale_;
        }
    }

    if (ImGui::SliderInt("Upscale Factor", &scaleFactor_, 2, 4)) {
        if (aiMethod_ != ai::UpscaleMethod::Bilinear) {
            if (!modelFileExists(aiMethod_, scaleFactor_)) {
                scaleFactor_ = prevScale;
            } else if (!reloadActiveModel()) {
                scaleFactor_ = prevScale;
            }
        }
    }

    if (!modelStatus_.empty()) {
        ImGui::TextWrapped("%s", modelStatus_.c_str());
    }

    if (aiMethod_ != ai::UpscaleMethod::Bilinear && !modelFileExists(aiMethod_, scaleFactor_)) {
        ImGui::TextColored({1.f, 0.4f, 0.3f, 1.f}, "Brak pliku: %s",
                           modelPathFor(aiMethod_, scaleFactor_).c_str());
        ImGui::Text("Dostepne: ESPCN/FSRCNN x2,x3,x4 (bash models/download_models.sh)");
    }

    if (ImGui::SliderFloat("Render Scale", &resolutionScale_, 0.5f, 1.0f, "%.2f")) {
        const int rw = renderWidth();
        const int rh = renderHeight();
        sceneFbo_.resize(rw, rh);
        frameCapture_.resize(rw, rh);
    }

    ImGui::Separator();
    ImGui::Text("  Render:   %.2f", timings_.renderMs);
    ImGui::Text("  Readback: %.2f", timings_.readbackMs);
    ImGui::Text("  AI:       %.2f", timings_.aiMs);
    ImGui::Text("  Upload:   %.2f", timings_.uploadMs);
    ImGui::Text("  Present:  %.2f", timings_.presentMs);
    ImGui::Text("  Total:    %.2f", timings_.totalMs);
    ImGui::Separator();
    ImGui::Text("Controls: WASD move, Q/E up/down");
    ImGui::Text("Right-click (outside UI): mouse look");
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
}

void Application::updateTimings(double frameMs) {
    timings_.totalMs = frameMs;
    if (frameMs > 0.0) {
        timings_.fps = 1000.0 / frameMs;
    }

    static int frameCounter = 0;
    if (++frameCounter % 120 == 0) {
#ifdef AIER_HAS_OPENCV
        const std::string csvPath = projectRoot_ + "/assets/benchmark_output/pipeline_timings.csv";
        std::filesystem::create_directories(projectRoot_ + "/assets/benchmark_output");
        std::ofstream csv(csvPath);
        if (csv) {
            csv << "render_ms,readback_ms,ai_ms,upload_ms,present_ms,total_ms,fps\n";
            csv << timings_.renderMs << ',' << timings_.readbackMs << ',' << timings_.aiMs << ','
                << timings_.uploadMs << ',' << timings_.presentMs << ',' << timings_.totalMs << ','
                << timings_.fps << '\n';
        }
#endif
    }
}

int Application::renderWidth() const {
    int w = static_cast<int>(window_->width() * resolutionScale_);
    w = (w / 2) * 2;
    return std::max(w, 2);
}

int Application::renderHeight() const {
    int h = static_cast<int>(window_->height() * resolutionScale_);
    h = (h / 2) * 2;
    return std::max(h, 2);
}

std::string Application::shaderPath(const std::string& relative) const {
    return projectRoot_ + "/shaders/" + relative;
}

std::string Application::modelPathFor(ai::UpscaleMethod method, int scale) const {
    switch (method) {
        case ai::UpscaleMethod::ESPCN:
            return projectRoot_ + "/models/ESPCN_x" + std::to_string(scale) + ".pb";
        case ai::UpscaleMethod::FSRCNN:
            return projectRoot_ + "/models/FSRCNN_x" + std::to_string(scale) + ".pb";
        case ai::UpscaleMethod::EDSR:
            return projectRoot_ + "/models/EDSR_x" + std::to_string(scale) + ".pb";
        default:
            return {};
    }
}

bool Application::modelFileExists(ai::UpscaleMethod method, int scale) const {
    if (method == ai::UpscaleMethod::Bilinear) {
        return true;
    }
    const std::string path = modelPathFor(method, scale);
    return !path.empty() && std::filesystem::exists(path);
}

bool Application::reloadActiveModel() {
    if (aiMethod_ == ai::UpscaleMethod::Bilinear) {
        superResolver_.loadModel("", ai::UpscaleMethod::Bilinear, scaleFactor_);
        modelStatus_ = "Tryb bilinear (bez modelu AI)";
        lastLoadedMethod_ = aiMethod_;
        lastLoadedScale_ = scaleFactor_;
        return true;
    }

    const std::string path = modelPathFor(aiMethod_, scaleFactor_);
    if (!std::filesystem::exists(path)) {
        modelStatus_ = "Brak pliku: " + path;
        return false;
    }

    if (superResolver_.loadModel(path, aiMethod_, scaleFactor_)) {
        modelStatus_ = "Model: " + path;
        lastLoadedMethod_ = aiMethod_;
        lastLoadedScale_ = scaleFactor_;
        return true;
    }

    modelStatus_ = "Nie udalo sie zaladowac: " + path;
    return false;
}

}  // namespace renderer::core
