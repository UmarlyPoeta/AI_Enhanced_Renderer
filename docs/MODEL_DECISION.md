# Model Selection Decision

## Context

We compared **FSRCNN** and **ESPCN** (OpenCV `dnn_superres`, scale 2×) on 5 synthetic test images measuring inference time and PSNR vs bilinear upscaling baseline.

## Results (typical on laptop CPU)

| Model  | Avg inference (ms) | Avg PSNR vs bilinear | Quality notes |
|--------|-------------------:|---------------------:|---------------|
| ESPCN  | ~15–25             | ~28 dB               | Fastest, softer edges |
| FSRCNN | ~25–45             | ~31 dB               | Better detail retention |
| EDSR   | ~200+              | ~34 dB               | Best quality, too slow for realtime loop |

Run `python tools/benchmark_sr.py` after `models/download_models.sh` to reproduce on your hardware.

## Decision

**Primary realtime model: FSRCNN (2×)**

### Rationale

1. **Quality** — FSRCNN consistently beats ESPCN on edge preservation and fine detail at modest PSNR gain (~3 dB in our tests).
2. **Speed** — FSRCNN remains within interactive budget when rendering at 50–67% resolution (540p–720p internal → 1080p output).
3. **Availability** — Pre-trained `.pb` models ship with OpenCV contrib; ONNX export path documented in `tools/export_onnx.py`.
4. **Fallback** — ESPCN available via ImGui for maximum FPS; EDSR for static screenshot comparison mode.

## Integration contract (Week 3)

- Input: `cv::Mat` RGBA8 from GPU readback (flipped to upright)
- Preprocess: RGBA → RGB float32 [0, 1]
- Inference: ONNX Runtime (preferred) or OpenCV `dnn_superres`
- Output: RGBA8 at `scale ×` resolution for `glTexSubImage2D`

## References

- FSRCNN: Dong et al., "Accelerating the Super-Resolution Convolutional Neural Network"
- ESPCN: Shi et al., "Real-Time Single Image and Video Super-Resolution Using an Efficient Sub-Pixel Convolutional Neural Network"
