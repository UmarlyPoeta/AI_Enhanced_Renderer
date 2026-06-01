#!/usr/bin/env python3
"""Full benchmark: multiple methods, PSNR/SSIM metrics."""

from __future__ import annotations

import argparse
import csv
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


def ssim_gray(ref: np.ndarray, test: np.ndarray) -> float:
    ref_g = cv2.cvtColor(ref, cv2.COLOR_BGR2GRAY).astype(np.float64)
    test_g = cv2.cvtColor(test, cv2.COLOR_BGR2GRAY).astype(np.float64)
    c1 = (0.01 * 255) ** 2
    c2 = (0.03 * 255) ** 2
    mu1 = cv2.GaussianBlur(ref_g, (11, 11), 1.5)
    mu2 = cv2.GaussianBlur(test_g, (11, 11), 1.5)
    mu1_sq, mu2_sq, mu1_mu2 = mu1 ** 2, mu2 ** 2, mu1 * mu2
    sigma1_sq = cv2.GaussianBlur(ref_g ** 2, (11, 11), 1.5) - mu1_sq
    sigma2_sq = cv2.GaussianBlur(test_g ** 2, (11, 11), 1.5) - mu2_sq
    sigma12 = cv2.GaussianBlur(ref_g * test_g, (11, 11), 1.5) - mu1_mu2
    num = (2 * mu1_mu2 + c1) * (2 * sigma12 + c2)
    den = (mu1_sq + mu2_sq + c1) * (sigma1_sq + sigma2_sq + c2)
    return float(np.mean(num / den))


def create_superres():
    if hasattr(cv2.dnn_superres, "DnnSuperResImpl_create"):
        return cv2.dnn_superres.DnnSuperResImpl_create()
    return cv2.dnn_superres.DnnSuperResImpl()


def upscale(method: str, img: np.ndarray, scale: int) -> np.ndarray:
    if method == "bilinear":
        h, w = img.shape[:2]
        return cv2.resize(img, (w * scale, h * scale), interpolation=cv2.INTER_LINEAR)
    model_map = {"FSRCNN": "fsrcnn", "ESPCN": "espcn", "EDSR": "edsr"}
    path = MODELS / f"{method}_x{scale}.pb"
    sr = create_superres()
    sr.readModel(str(path))
    sr.setModel(model_map[method], scale)
    return sr.upsample(img)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--scale", type=int, default=2)
    args = parser.parse_args()

    OUTPUT.mkdir(parents=True, exist_ok=True)
    methods = ["bilinear", "ESPCN", "FSRCNN", "EDSR"]
    images = sorted(ASSETS.glob("test_*.png"))
    if not images:
        print("Run tools/benchmark_sr.py first to generate test images")
        return 1

    rows = []
    for img_path in images:
        img = cv2.imread(str(img_path))
        ref = upscale("FSRCNN", img, args.scale) if (MODELS / f"FSRCNN_x{args.scale}.pb").exists() else upscale("bilinear", img, args.scale)
        for method in methods:
            if method != "bilinear" and not (MODELS / f"{method}_x{args.scale}.pb").exists():
                continue
            import time
            t0 = time.perf_counter()
            out = upscale(method, img, args.scale)
            ms = (time.perf_counter() - t0) * 1000
            rows.append({
                "scene": img_path.stem,
                "method": method,
                "psnr": f"{psnr(ref, out):.2f}",
                "ssim": f"{ssim_gray(ref, out):.4f}",
                "time_ms": f"{ms:.2f}",
            })

    out_csv = OUTPUT / "benchmark_full.csv"
    with out_csv.open("w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(f, fieldnames=["scene", "method", "psnr", "ssim", "time_ms"])
        writer.writeheader()
        writer.writerows(rows)
    print(f"Wrote {out_csv}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
