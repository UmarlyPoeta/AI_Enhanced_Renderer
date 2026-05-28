#!/usr/bin/env bash
# Download vendored dependencies into extern/ (GLAD is already included).
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
EXTERN="$ROOT/extern"

mkdir -p "$EXTERN"

if [[ ! -f "$EXTERN/glm/glm/glm.hpp" ]]; then
  echo "Downloading GLM..."
  curl -sL "https://github.com/g-truc/glm/archive/refs/tags/0.9.9.8.tar.gz" | tar xz -C "$EXTERN"
  mv "$EXTERN/glm-0.9.9.8" "$EXTERN/glm"
fi

if [[ ! -f "$EXTERN/imgui/imgui.h" ]]; then
  echo "Downloading ImGui..."
  curl -sL "https://github.com/ocornut/imgui/archive/refs/tags/v1.90.1.tar.gz" | tar xz -C "$EXTERN"
  mv "$EXTERN/imgui-1.90.1" "$EXTERN/imgui"
fi

echo "extern/ dependencies ready."
