#!/usr/bin/env python3
"""Export / verify ONNX model for super-resolution (placeholder workflow)."""

from __future__ import annotations

import argparse
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MODELS = ROOT / "models"


def main() -> int:
    parser = argparse.ArgumentParser(description="ONNX export helper")
    parser.add_argument("--verify", type=Path, help="Verify existing ONNX model")
    args = parser.parse_args()

    if args.verify:
        try:
            import onnxruntime as ort
        except ImportError:
            print("Install: pip install onnxruntime")
            return 1
        session = ort.InferenceSession(str(args.verify))
        print(f"Model: {args.verify}")
        print(f"Inputs:  {[i.name for i in session.get_inputs()]}")
        print(f"Outputs: {[o.name for o in session.get_outputs()]}")
        return 0

    onnx_path = MODELS / "FSRCNN_x2.onnx"
    if onnx_path.exists():
        print(f"ONNX model already present: {onnx_path}")
        return 0

    print("To export FSRCNN to ONNX:")
    print("  1. pip install torch torchvision")
    print("  2. Load pretrained FSRCNN weights")
    print("  3. torch.onnx.export(model, dummy_input, 'models/FSRCNN_x2.onnx')")
    print("Alternatively download from ONNX Model Zoo.")
    print(f"Place result at: {onnx_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
