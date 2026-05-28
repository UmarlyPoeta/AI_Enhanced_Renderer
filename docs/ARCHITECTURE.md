# Architecture

## Core Modules

| Module | Path | Responsibility |
|--------|------|----------------|
| `core` | `src/core/` | Application lifecycle, main loop, ImGui UI |
| `platform` | `src/platform/` | GLFW window, OpenGL 3.3 context |
| `rendering` | `src/rendering/` | Shaders, FBO, PBO, textures, fullscreen quad |
| `scene` | `src/scene/` | Camera (WASD), multi-object Phong scene |
| `ai` | `src/ai/` | `AiSuperResolver` — OpenCV / optional ONNX |
| `utils` | `include/renderer/utils/` | FPS counter, pipeline timings |

## Pipeline (implemented)

```text
1. Render scene → FBO (dynamic resolution, e.g. 67% of window)
2. glReadPixels → PBO (double-buffered async readback)
3. RGBA → RGB → OpenCV/ONNX super-resolution
4. glTexSubImage2D → result texture
5. Full-screen quad + gamma / ACES tone mapping → swap buffers
```

## Data contract

```cpp
cv::Mat FrameCapture::getFrame();           // RGBA8, Y-flipped
cv::Mat AiSuperResolver::upsample(cv::Mat);  // RGBA8 in → RGBA8 out (scaled)
void Texture::uploadRGBA(data, w, h);        // back to GPU
```

See [MODEL_DECISION.md](MODEL_DECISION.md) for SR model selection rationale.
