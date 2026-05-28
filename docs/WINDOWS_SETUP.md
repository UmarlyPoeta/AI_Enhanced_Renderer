# Setup on Windows

## Wymagania

- **Windows 10/11**
- **Visual Studio 2022** (workload: *Desktop development with C++*)
- **CMake 3.20+** ([cmake.org](https://cmake.org/download/))
- **Git** ([git-scm.com](https://git-scm.com/download/win))
- **vcpkg** (zalecane do zależności) — [github.com/microsoft/vcpkg](https://github.com/microsoft/vcpkg)

Opcjonalnie: **Python 3** + `pip install opencv-contrib-python matplotlib` (benchmarki)

## 1. vcpkg

```powershell
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat
setx VCPKG_ROOT "C:\vcpkg"
```

Zainstaluj biblioteki (x64):

```powershell
vcpkg install glfw3:x64-windows opencv4[contrib,dnn]:x64-windows
```

## 2. Zależności projektu (GLM + ImGui)

W katalogu repozytorium:

```powershell
powershell -ExecutionPolicy Bypass -File tools\setup_extern.ps1
```

## 3. Modele AI (opcjonalnie)

Git Bash lub WSL:

```bash
bash models/download_models.sh
```

Albo ręcznie pobierz pliki `.pb` do folderu `models\` (patrz `models/download_models.sh`).

## 4. Build

```powershell
cd path\to\ai_enhanced_renderer

cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## 5. Uruchomienie

```powershell
.\build\src\Release\AIEnhancedRenderer.exe
```

> Visual Studio generator umieszcza exe w `build\src\Release\`.  
> Ninja/Makefiles: `build\src\AIEnhancedRenderer.exe`

## Sterowanie

| Input | Akcja |
|-------|-------|
| WASD | Ruch kamery |
| Q / E | Góra / dół |
| PPM (poza panelem UI) | Mouse look |
| ESC | Wyjście |
| ImGui panel | Tryb Bilinear/AI, model, skala renderowania |

## Rozwiązywanie problemów

| Problem | Rozwiązanie |
|---------|-------------|
| `opencv_dnn_superres` not found | `vcpkg install opencv4[contrib,dnn]:x64-windows` |
| Czarny ekran | Uruchom z katalogu repo; sprawdź czy istnieje `shaders\` |
| ImGui nie reaguje | Kliknij w okno aplikacji (focus); panel po lewej |
| Brak GPU / OpenGL 3.3 | Zaktualizuj sterowniki karty graficznej |

## ONNX Runtime (opcjonalnie)

1. Pobierz [ONNX Runtime Windows x64](https://github.com/microsoft/onnxruntime/releases)
2. Rozpakuj do `extern\onnxruntime\` (`include\`, `lib\`)
3. Rebuild z `-DAIER_ENABLE_ONNXRUNTIME=ON`
