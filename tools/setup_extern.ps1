# Download vendored GLM + ImGui into extern/ (Windows).
$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$Extern = Join-Path $Root "extern"

New-Item -ItemType Directory -Force -Path $Extern | Out-Null

$GlmDir = Join-Path $Extern "glm"
if (-not (Test-Path (Join-Path $GlmDir "glm\glm.hpp"))) {
    Write-Host "Downloading GLM..."
    $GlmZip = Join-Path $env:TEMP "glm-0.9.9.8.zip"
    Invoke-WebRequest -Uri "https://github.com/g-truc/glm/archive/refs/tags/0.9.9.8.zip" -OutFile $GlmZip
    Expand-Archive -Path $GlmZip -DestinationPath $Extern -Force
    Rename-Item (Join-Path $Extern "glm-0.9.9.8") $GlmDir -Force
    Remove-Item $GlmZip
}

$ImguiDir = Join-Path $Extern "imgui"
if (-not (Test-Path (Join-Path $ImguiDir "imgui.h"))) {
    Write-Host "Downloading ImGui..."
    $ImguiZip = Join-Path $env:TEMP "imgui-1.90.1.zip"
    Invoke-WebRequest -Uri "https://github.com/ocornut/imgui/archive/refs/tags/v1.90.1.zip" -OutFile $ImguiZip
    Expand-Archive -Path $ImguiZip -DestinationPath $Extern -Force
    Rename-Item (Join-Path $Extern "imgui-1.90.1") $ImguiDir -Force
    Remove-Item $ImguiZip
}

Write-Host "extern/ dependencies ready."
