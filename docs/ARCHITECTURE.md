# Architecture (Skeleton)

## Core Modules

- `core`: app lifecycle, main loop, timing, input
- `rendering`: OpenGL context, shaders, meshes, FBO pipeline
- `ai`: super-resolution adapters (OpenCV/ONNX)
- `scene`: camera, entities, transforms, materials
- `platform`: windowing + OS specifics
- `utils`: logging, config, profiling helpers

## Pipeline Target

1. Render low-resolution scene to FBO
2. Readback (PBO) or GPU-side post-process
3. Super-resolution stage
4. Present final full-screen result
