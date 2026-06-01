#!/usr/bin/env python3
"""Benchmark FSRCNN vs ESPCN on test images."""

from __future__ import annotations

import argparse
import csv
import sys
import time
from pathlib import Path

import cv2
import numpy as np

ROOT = Path(__file__).resolve().parents[1]
MODELS = ROOT / "models"
ASSETS = ROOT / "assets" / "test_images"
OUTPUT = ROOT / "assets" / "benchmark_output"


def psnr(ref: np.ndarray, test: np.ndarray) -> float:
    ref = ref.astype(np.float64)
    test = test.astype(np.float64)
    mse = np.mean((ref - test) ** 2)
    if mse == 0:
        return float("inf")
    return 20 * np.log10(255.0 / np.sqrt(mse))


def create_superres():
    for factory in (
        lambda: cv2.dnn_superres.DnnSuperResImpl_create(),
        lambda: cv2.dnn_superres.DnnSuperResImpl(),
        lambda: cv2.dnn_superres.DnnSuperResImpl.create(),
    ):
        try:
            return factory()
        except AttributeError:
            continue
    raise RuntimeError(
        "cv2.dnn_superres unavailable. Install: pip install opencv-contrib-python"
    )


def run_model(model_path: Path, algo: str, scale: int, image: np.ndarray) -> tuple[np.ndarray, float]:
    try:
        sr = create_superres()
    except RuntimeError as e:
        print(f"Warning: {e}; using bilinear fallback", file=sys.stderr)
        h, w = image.shape[:2]
        t0 = time.perf_counter()
        out = cv2.resize(image, (w * scale, h * scale), interpolation=cv2.INTER_LINEAR)
        return out, (time.perf_counter() - t0) * 1000.0
    sr.readModel(str(model_path))
    sr.setModel(algo, scale)
    t0 = time.perf_counter()
    out = sr.upsample(image)
    elapsed_ms = (time.perf_counter() - t0) * 1000.0
    return out, elapsed_ms


def ensure_test_images() -> list[Path]:
    ASSETS.mkdir(parents=True, exist_ok=True)
    paths: list[Path] = []
    for i in range(5):
        p = ASSETS / f"test_{i}.png"
        if not p.exists():
            img = np.zeros((128, 128, 3), dtype=np.uint8)
            cv2.rectangle(img, (10 + i * 5, 10), (100, 100), (50 + i * 30, 100, 200), -1)
            cv2.putText(img, f"T{i}", (40, 70), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 2)
            cv2.imwrite(str(p), img)
        paths.append(p)
    return paths


def main() -> int:
    parser = argparse.ArgumentParser(description="Benchmark super-resolution models")
    parser.add_argument("--scale", type=int, default=2, choices=[2, 3, 4])
    args = parser.parse_args()

    OUTPUT.mkdir(parents=True, exist_ok=True)
    images = ensure_test_images()

    models = {
        "FSRCNN": MODELS / f"FSRCNN_x{args.scale}.pb",
        "ESPCN": MODELS / f"ESPCN_x{args.scale}.pb",
    }

    rows: list[dict] = []
    for name, model_path in models.items():
        if not model_path.exists():
            print(f"Warning: missing model {model_path}", file=sys.stderr)
            continue
        algo = name.lower()
        for img_path in images:
            img = cv2.imread(str(img_path))
            if img is None:
                continue
            out, ms = run_model(model_path, algo, args.scale, img)
            up_bilinear = cv2.resize(img, (out.shape[1], out.shape[0]), interpolation=cv2.INTER_LINEAR)
            score = psnr(out, up_bilinear)
            rows.append({
                "model": name,
                "image": img_path.name,
                "inference_ms": f"{ms:.2f}",
                "psnr_vs_bilinear": f"{score:.2f}",
                "output_w": out.shape[1],
                "output_h": out.shape[0],
            })
            cv2.imwrite(str(OUTPUT / f"{name}_{img_path.stem}_x{args.scale}.png"), out)

    csv_path = OUTPUT / "benchmark_sr.csv"
    if rows:
        with csv_path.open("w", newline="", encoding="utf-8") as f:
            writer = csv.DictWriter(f, fieldnames=rows[0].keys())
            writer.writeheader()
            writer.writerows(rows)
        print(f"Wrote {csv_path} ({len(rows)} rows)")
        return 0
    print("No results — install opencv-contrib-python and run models/download_models.sh", file=sys.stderr)
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
