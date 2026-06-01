#!/usr/bin/env python3
"""Plot pipeline stage timings from CSV or sample data."""

from __future__ import annotations

import argparse
import csv
from pathlib import Path

import matplotlib.pyplot as plt

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "assets" / "benchmark_output"


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--csv", type=Path, default=OUTPUT / "pipeline_timings.csv")
    args = parser.parse_args()

    OUTPUT.mkdir(parents=True, exist_ok=True)

    if args.csv.exists():
        stages, values = [], []
        with args.csv.open(encoding="utf-8") as f:
            row = next(csv.DictReader(f))
            for key in ("render_ms", "readback_ms", "ai_ms", "upload_ms", "present_ms"):
                if key in row:
                    stages.append(key.replace("_ms", ""))
                    values.append(float(row[key]))
    else:
        stages = ["render", "readback", "ai", "upload", "present"]
        values = [3.2, 0.8, 35.0, 1.1, 0.5]

    fig, ax = plt.subplots(figsize=(8, 4))
    ax.bar(stages, values, color=["#4C72B0", "#55A868", "#C44E52", "#8172B2", "#CCB974"])
    ax.set_ylabel("Time (ms)")
    ax.set_title("Pipeline stage timings per frame")
    total = sum(values)
    ax.axhline(total, color="gray", linestyle="--", label=f"Total: {total:.1f} ms")
    ax.legend()
    fig.tight_layout()
    out = OUTPUT / "pipeline_timings.png"
    fig.savefig(out, dpi=150)
    print(f"Wrote {out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
