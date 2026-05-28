# AI-Enhanced Real-Time Renderer

> **Renderer 3D łączący Modern OpenGL z modelami super-resolution dla rekonstrukcji jakości obrazu w czasie rzeczywistym.**

[![OpenGL](https://img.shields.io/badge/OpenGL-3.3+-blue)](https://www.opengl.org/)
[![OpenCV](https://img.shields.io/badge/OpenCV-4.x-green)](https://opencv.org/)
[![C++](https://img.shields.io/badge/C++-17-orange)](https://isocpp.org/)
[![License](https://img.shields.io/badge/License-MIT-yellow)](LICENSE)

---

## TL;DR

Projekt łączy renderowanie 3D w niższej rozdzielczości przez **Modern OpenGL** z rekonstrukcją jakości przez modele **super-resolution** (FSRCNN/EDSR). Dla początkujących zalecane podejście CPU: renderuj scenę do FBO w 540p, przechwyć przez PBO do OpenCV, upscaluj przez AI, wyświetl wynik.


## GitHub Setup

Repo zawiera gotowe elementy pod współpracę zespołową:
- `.github/workflows/ci.yml` - podstawowy build CMake na Ubuntu,
- `.github/ISSUE_TEMPLATE/bug_report.md` - zgłaszanie błędów,
- `.github/ISSUE_TEMPLATE/feature_request.md` - propozycje funkcji,
- `.github/pull_request_template.md` - checklista dla PR.

## Struktura katalogów

```text
.
├── .github/
│   ├── ISSUE_TEMPLATE/
│   └── workflows/
├── assets/
├── cmake/
│   ├── Dependencies.cmake
│   └── ProjectOptions.cmake
├── docs/
├── include/
│   └── renderer/
│       ├── ai/
│       ├── core/
│       ├── platform/
│       ├── rendering/
│       ├── scene/
│       └── utils/
├── models/
├── shaders/
│   ├── compute/
│   ├── postprocess/
│   └── scene/
├── src/
│   ├── ai/
│   ├── core/
│   ├── platform/
│   ├── rendering/
│   ├── scene/
│   ├── utils/
│   ├── CMakeLists.txt
│   └── main.cpp
├── tests/
└── tools/
```

## Shared CMakeLists

Projekt jest podzielony na warstwy CMake:
- `CMakeLists.txt` (root): konfiguracja projektu, opcje, zależności i `add_subdirectory(src)`.
- `src/CMakeLists.txt`: budowa targetu aplikacji, linkowanie OpenGL/GLFW/ASSIMP/GLM.
- `cmake/ProjectOptions.cmake`: wspólne ustawienia kompilacji i include path dla targetów.
- `cmake/Dependencies.cmake`: FetchContent dla bibliotek zewnętrznych.

### Opcje builda

- `AIER_ENABLE_ONNXRUNTIME=ON` - dołącza lokalne ONNX Runtime z `extern/onnxruntime`.

## Quick Start

### Wymagania

- CMake 3.20+, kompilator C++20
- OpenGL 3.3+, GLFW, OpenCV 4.x z modułem `dnn_superres`
- Python 3 (skrypty benchmarków)

**Fedora:**
```bash
sudo dnf install cmake gcc-c++ glfw-devel opencv-devel
```

**Ubuntu/Debian:**
```bash
sudo apt install cmake g++ libglfw3-dev libopencv-dev libopencv-contrib-dev
```

**Windows:** pełna instrukcja w [docs/WINDOWS_SETUP.md](docs/WINDOWS_SETUP.md)

```powershell
powershell -ExecutionPolicy Bypass -File tools\setup_extern.ps1
vcpkg install glfw3:x64-windows opencv4[contrib,dnn]:x64-windows
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build build --config Release
.\build\src\Release\AIEnhancedRenderer.exe
```

### Build (Linux / macOS)

```bash
bash tools/setup_extern.sh          # pobiera GLM + ImGui do extern/
bash models/download_models.sh      # opcjonalnie: modele FSRCNN/ESPCN
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/src/AIEnhancedRenderer
```

### Sterowanie

| Klawisz | Akcja |
|---------|-------|
| WASD | Ruch kamery |
| Q / E | Góra / dół |
| PPM (poza panelem UI) | Włącz/wyłącz mouse look |
| ESC | Wyjście |

ImGui panel: tryb wyświetlania (Bilinear / AI), wybór modelu, skala renderowania, FPS i timings.

### Benchmarki (Python)

```bash
python3 tools/benchmark_sr.py       # FSRCNN vs ESPCN
python3 tools/benchmark_full.py   # PSNR/SSIM raport CSV
python3 tools/plot_pipeline.py    # wykres etapów pipeline
```

Decyzja modelu: [docs/MODEL_DECISION.md](docs/MODEL_DECISION.md)

### Opcje CMake

| Opcja | Domyślnie | Opis |
|-------|-----------|------|
| `AIER_ENABLE_OPENCV` | ON | Integracja OpenCV SR |
| `AIER_ENABLE_IMGUI` | ON | Panel debug UI |
| `AIER_ENABLE_ONNXRUNTIME` | OFF | ONNX z `extern/onnxruntime` |

---

## What we learned

- **FBO + dynamic resolution** — render w niższej rozdzielczości daje realny zysk FPS; AI rekompensuje utratę detali.
- **PBO double-buffering** — asynchroniczny readback pozwala nakładać pracę GPU i CPU.
- **Trade-off modeli** — ESPCN szybszy, FSRCNN lepszy jakościowo; EDSR tylko offline.
- **Integracja GPU↔CPU** — najwięcej bugów przy flip Y, RGBA↔BGR i rozmiarach tensorów.

---

| Faza | Czas | Opis |
|------|------|------|
| Podstawowy OpenGL | 2–3 tyg. | Kontekst, shadery, FBO, scena 3D |
| Dynamic Resolution | 1–2 tyg. | Zmienna rozdzielczość, tradycyjny upscaling |
| Integracja AI | 3–4 tyg. | OpenCV dnn_superres, PBO, pipelining |
| Optymalizacje | 2–3 tyg. | Buffering, GUI, porównania A/B |

---

## Spis treści

- [1. Fundamenty Modern OpenGL (Core Profile 3.3+)](#1-fundamenty-modern-opengl-core-profile-33)
    - [1.1 Architektura i Filozofia Modern OpenGL](#11-architektura-i-filozofia-modern-opengl)
    - [1.2 Podstawowe Operacje Renderingu](#12-podstawowe-operacje-renderingu)
    - [1.3 System Współrzędnych i Przekształcenia](#13-system-współrzędnych-i-przekształcenia)
- [2. Zaawansowane Techniki Renderingu dla AI Super Resolution](#2-zaawansowane-techniki-renderingu-dla-ai-super-resolution)
    - [2.1 Framebuffer Objects (FBO) – Klucz do Dynamic Resolution](#21-framebuffer-objects-fbo--klucz-do-dynamic-resolution)
    - [2.2 Dynamic Resolution Rendering](#22-dynamic-resolution-rendering)
    - [2.3 Post-processing i Full-Screen Effects](#23-post-processing-i-full-screen-effects)
- [3. Compute Shaders – Obliczenia GPU dla AI](#3-compute-shaders--obliczenia-gpu-dla-ai)
    - [3.1 Wprowadzenie do Compute Shaders (OpenGL 4.3+)](#31-wprowadzenie-do-compute-shaders-opengl-43)
    - [3.2 Przetwarzanie Tekstur w Compute Shaders](#32-przetwarzanie-tekstur-w-compute-shaders)
- [4. Modele AI dla Super Resolution](#4-modele-ai-dla-super-resolution--wybór-i-przygotowanie)
    - [4.1 Przegląd Architektur Super Resolution](#41-przegląd-architektur-super-resolution)
    - [4.2 Przygotowanie Modelu do Wdrożenia](#42-przygotowanie-modelu-do-wdrożenia)
- [5. Integracja AI z Pipeline'm Renderingu OpenGL](#5-integracja-ai-z-pipelinem-renderingu-opengl)
    - [5.1 Strategie Integracji – Porównanie Podejść](#51-strategie-integracji--porównanie-podejść)
    - [5.2 Implementacja Podejścia CPU](#52-szczegółowa-implementacja-podejścia-cpu-rekomendowana-dla-początkujących)
    - [5.3 Optymalizacje Wydajnościowe](#53-optymalizacje-wydajnościowe)
- [6. Zaawansowane Techniki i Rozszerzenia](#6-zaawansowane-techniki-i-rozszerzenia)
- [7. Roadmapa Projektu](#7-roadmapa-projektu--od-początku-do-końca)
- [8. Zasoby i Materiały Uzupełniające](#8-zasoby-i-materiały-uzupełniające)

---

## 1. Fundamenty Modern OpenGL (Core Profile 3.3+)

### 1.1 Architektura i Filozofia Modern OpenGL

#### 1.1.1 Core Profile vs Compatibility Profile

Modern OpenGL wprowadza fundamentalną zmianę paradygmatu w porównaniu do starszych wersji API. **Core Profile**, dostępny od wersji 3.2, całkowicie eliminuje przestarzałe funkcje fixed-function pipeline:

- `glBegin`/`glEnd`, `glVertex3f`
- `glMatrixMode`, `glLoadIdentity`, `glTranslatef`, `glRotatef`, `glScalef`
- `glFrustum`, `glOrtho`, `glPushMatrix`/`glPopMatrix`
- `glLightfv`, `glMaterialfv`, `glEnable(GL_LIGHTING)`, `glEnable(GL_TEXTURE_2D)`
- `glShadeModel`, `glPolygonMode` w kontekście fixed-function

Ta radykalna redukcja API wymusza pisanie wszystkich operacji graficznych w shaderach **GLSL**, co prowadzi do czystszego, bardziej przewidywalnego kodu i znacząco redukuje narzut sterownika.

Dla projektu AI-Enhanced Real-Time Renderer wybór **Core Profile 3.3+** jest obligatoryjny z trzech kluczowych powodów:

1. **Framebuffer Objects (FBO)** – fundamentalna technika dla dynamic resolution rendering – działają znacznie bardziej przewidywalnie w Core Profile.
2. **Compute Shaders (OpenGL 4.3+)**, kluczowe narzędzie dla potencjalnej implementacji inferencji AI bezpośrednio na GPU, są dostępne wyłącznie w kontekście Core Profile.
3. **Platforma macOS** wspiera wyłącznie Core Profile, co zapewnia przenośność kodu.

#### 1.1.2 Programowalny pipeline graficzny

Programowalny pipeline Modern OpenGL składa się z kilku etapów, z których każdy może być dostosowany przez odpowiedni shader GLSL:

```
Vertex Data ──► [Vertex Shader] ──► [Geometry Shader*] ──► Rasterization ──► [Fragment Shader] ──► Framebuffer
                                                                                 (* opcjonalny)
```

**Vertex Shader** — Pierwszy programowalny etap, przetwarzający każdy wierzchołek niezależnie. Transformuje pozycję wierzchołka z przestrzeni modelowej do przestrzeni przycięcia (clip space).

**Geometry Shader** — Opcjonalny etap wykonujący się raz na każdy prymityw i mogący generować nowe prymitywy. Użyteczny dla efektów proceduralnych.

**Fragment Shader** — Najważniejszy dla efektów wizualnych, wykonuje się dla każdego potencjalnego piksela i determinuje jego końcowy kolor.

#### 1.1.3 Zarządzanie stanem przez obiekty (VAO, VBO, FBO, tekstury)

Modern OpenGL przeszedł od modelu globalnego stanu maszyny stanów do modelu obiektowego, gdzie większość zasobów jest reprezentowana przez uchwyty (*handles*), które muszą być jawnie tworzone, konfigurowane, wiązane i niszczone.

| Obiekt | Przeznaczenie | Kluczowe funkcje |
|--------|--------------|------------------|
| **VAO** (Vertex Array Object) | Enkapsulacja konfiguracji atrybutów wierzchołków | `glGenVertexArrays`, `glBindVertexArray`, `glVertexAttribPointer` |
| **VBO** (Vertex Buffer Object) | Przechowywanie danych wierzchołków w pamięci GPU | `glGenBuffers`, `glBindBuffer`, `glBufferData` |
| **FBO** (Framebuffer Object) | Renderowanie do tekstur (render-to-texture) | `glGenFramebuffers`, `glBindFramebuffer`, `glFramebufferTexture2D` |
| **Tekstury** | Dane obrazowe, cele renderowania, wejścia dla shaderów | `glGenTextures`, `glBindTexture`, `glTexImage2D` |

> **📌 Kluczowe:** FBO stanowi absolutnie fundamentalny element dla projektu super-resolution, umożliwiając:
> - Renderowanie sceny w niższej rozdzielczości niż wyświetlacz (*dynamic resolution*)
> - Przechwytywanie wyniku do tekstury dla dalszego przetwarzania (w tym AI upscaling)
> - Implementację wielu etapów post-processingu
> - Techniki takie jak deferred rendering i shadow mapping

#### 1.1.4 GLFW i GLAD – inicjalizacja kontekstu OpenGL w C++

**GLFW** (Graphics Library Framework) i **GLAD** (GL Loader-Generator) stanowią współczesny standard dla inicjalizacji OpenGL w C++.

```cpp
// Inicjalizacja GLFW
glfwSetErrorCallback(error_callback);
if (!glfwInit()) {
        // Obsługa błędu
}

// Konfiguracja wymagań dla kontekstu OpenGL
glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

// Wymagane dla macOS
#ifdef __APPLE__
glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

// Tworzenie okna
GLFWwindow* window = glfwCreateWindow(1280, 720, "AI Super Resolution Renderer", NULL, NULL);
if (!window) {
        glfwTerminate();
        // Obsługa błędu
}

glfwMakeContextCurrent(window);

// Ładowanie funkcji OpenGL przez GLAD
if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        // Obsługa błędu
}

// Konfiguracja viewportu i callbacków
glViewport(0, 0, 1280, 720);
glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
```

---

### 1.2 Podstawowe Operacje Renderingu

#### 1.2.1 Tworzenie okna i kontekstu renderowania

Proces tworzenia okna i kontekstu renderowania w GLFW/GLAD stanowi fundament każdej aplikacji OpenGL. Dla projektu AI-Enhanced Renderer istnieją jednak dodatkowe kwestie do rozważenia.

> **Rozdzielczość okna** (*window resolution*) i **rozdzielczość renderowania** (*render resolution*) to dwie odrębne wartości. Okno może mieć rozdzielczość 1920×1080 (Full HD), podczas gdy scena jest renderowana do FBO o rozdzielczości 1280×720 (HD) lub nawet 960×540 (qHD), a następnie upscalowana przez AI do 1920×1080.

#### 1.2.2 Vertex Array Objects (VAO) i Vertex Buffer Objects (VBO)

Szczegółowa implementacja VAO i VBO dla typowych przypadków użycia w projekcie super-resolution:

```cpp
// Dla geometrii 3D sceny:
struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoord;
        // Opcjonalnie: tangent, bitangent dla normal mapping
};

// Generowanie i konfiguracja
GLuint vao, vbo, ebo;
glGenVertexArrays(1, &vao);
glGenBuffers(1, &vbo);
glGenBuffers(1, &ebo);

glBindVertexArray(vao);

// VBO z danymi wierzchołków
glBindBuffer(GL_ARRAY_BUFFER, vbo);
glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), 
                         vertices.data(), GL_STATIC_DRAW);

// EBO z indeksami
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint),
                         indices.data(), GL_STATIC_DRAW);

// Konfiguracja atrybutów
// Position (location = 0)
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
                                            (void*)offsetof(Vertex, position));
glEnableVertexAttribArray(0);

// Normal (location = 1)
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                            (void*)offsetof(Vertex, normal));
glEnableVertexAttribArray(1);

// TexCoord (location = 2)
glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                            (void*)offsetof(Vertex, texCoord));
glEnableVertexAttribArray(2);

glBindVertexArray(0);
```

**Full-screen quad** — fundamentalny element projektu, używany do wyświetlania wyniku renderowania z FBO, aplikowania filtrów upscalingu oraz wyświetlania wyniku AI super-resolution:

```cpp
float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
        
        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
};
```

#### 1.2.3 Kompilacja i linkowanie shaderów (GLSL)

Proces tworzenia programu shaderowego wymaga wielu kroków z obsługą błędów na każdym etapie:

```cpp
GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource) {
        // Kompilacja vertex shader
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexSource, NULL);
        glCompileShader(vertexShader);
        
        // Sprawdzenie błędów kompilacji
        GLint success;
        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success) {
                glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
                std::cerr << "Vertex shader compilation failed:\n" << infoLog << std::endl;
                glDeleteShader(vertexShader);
                return 0;
        }
        
        // Analogicznie dla fragment shader
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
        glCompileShader(fragmentShader);
        // ... sprawdzenie błędów ...
        
        // Linkowanie programu
        GLuint program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);
        
        // Sprawdzenie błędów linkowania
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
                glGetProgramInfoLog(program, 512, NULL, infoLog);
                std::cerr << "Program linking failed:\n" << infoLog << std::endl;
                return 0;
        }
        
        // Shadery można usunąć po linkowaniu
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        
        return program;
}
```

Dla projektu super-resolution potrzebne są co najmniej **trzy programy shaderowe**:
1. Shader do renderowania sceny 3D z oświetleniem
2. Shader do tradycyjnego upscalingu (opcjonalnie, dla porównania)
3. Shader do wyświetlania finalnego wyniku jako full-screen quad z opcjonalnym gamma correction i tone mapping

#### 1.2.4 Rysowanie prymitywów (glDrawArrays, glDrawElements)

| Funkcja | Zastosowanie | Parametry kluczowe |
|---------|-------------|-------------------|
| `glDrawArrays(GL_TRIANGLES, first, count)` | Nieindeksowana geometria | `first` – offset w buforze, `count` – liczba wierzchołków |
| `glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, offset)` | Indeksowana geometria | `count` – liczba indeksów, `offset` – offset w EBO |
| `glDrawArraysInstanced` / `glDrawElementsInstanced` | Instancing wielu kopii | `instancecount` – liczba instancji |

`glDrawElements` jest preferowane dla złożonej geometrii 3D ze względu na możliwość współdzielenia wierzchołków między trójkątami, redukując zużycie pamięci o **50–70%**. Dla quadów pełnoekranowych używanych w post-processing, `glDrawArrays` z 6 wierzchołkami jest wystarczający.

---

### 1.3 System Współrzędnych i Przekształcenia

#### 1.3.1 Przestrzenie: modelowa, światowa, widoku, przycięcia, ekranu

| Przestrzeń | Opis | Transformacja | Zakres typowych wartości |
|-----------|------|--------------|------------------------|
| **Modelowa** (Object/Local) | Współrzędne wierzchołków względem centrum obiektu | — | Zależne od modelu, często -1 do 1 |
| **Światowa** (World) | Pozycja obiektu w globalnym układzie sceny | Macierz modelu (**M**) | Dowolne, zależne od skali sceny |
| **Widoku** (View/Camera/Eye) | Współrzędne względem kamery | Macierz widoku (**V**) | Zależne od odległości od kamery |
| **Przycięcia** (Clip) | Po zastosowaniu projekcji, przed dzieleniem perspektywicznym | Macierz projekcji (**P**) | Homogeniczne (x, y, z, w) |
| **NDC** (Normalized Device Coordinates) | Po podzieleniu przez w | Dzielenie perspektywiczne | x, y ∈ [-1, 1]; z ∈ [-1, 1] lub [0, 1] |
| **Ekranu** (Screen/Window) | Pikselowe współrzędne viewportu | Viewport transform | x ∈ [0, width-1]; y ∈ [0, height-1] |

Transformacja z przestrzeni modelowej do NDC wykonywana jest przez vertex shader:

```glsl
gl_Position = projection * view * model * vec4(position, 1.0);
```

> **⚠️ Uwaga:** Kolejność mnożenia jest krytyczna – OpenGL używa kolumnowego układu macierzy, więc transformacje stosowane są od prawej do lewej.

#### 1.3.2 Macierze transformacji i biblioteka GLM

**GLM** (OpenGL Mathematics) to header-only biblioteka C++ stanowiąca de facto standard dla operacji matematycznych w OpenGL.

```cpp
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Typy i inicjalizacja
glm::vec3 position(1.0f, 2.0f, 3.0f);
glm::mat4 model = glm::mat4(1.0f); // macierz jednostkowa

// Budowanie macierzy transformacji
model = glm::translate(model, glm::vec3(5.0f, 0.0f, 0.0f));
model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));

// Macierz widoku (kamera w pozycji (0,0,5), patrząca na (0,0,0))
glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 5.0f),  // eye
        glm::vec3(0.0f, 0.0f, 0.0f),  // center
        glm::vec3(0.0f, 1.0f, 0.0f)   // up
);

// Macierz projekcji perspektywicznej
glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),  // FOV w pionie
        1280.0f / 720.0f,     // aspect ratio
        0.1f,                 // near plane
        100.0f                // far plane
);

// Kombinacja MVP
glm::mat4 mvp = projection * view * model;

// Przekazanie do shadera
glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "mvp"), 
                                     1, GL_FALSE, glm::value_ptr(mvp));
```

Dla projektu super-resolution, GLM jest niezbędna nie tylko dla standardowego renderowania 3D, ale również dla obliczania współrzędnych tekstur przy wyświetlaniu wyników, gdzie precyzyjne mapowanie jest krytyczne dla jakości.

#### 1.3.3 Uniform Buffer Objects (UBO) dla efektywnego przekazywania danych

**Uniform Buffer Objects (UBO)**, dostępne od OpenGL 3.1, pozwalają na przechowywanie uniformów w buforach GPU zamiast indywidualnych wywołań `glUniform*`.

**Zalety:**
- Redukcja overheadu przy wielu uniformach
- Współdzielenie tych samych danych między wieloma programami shaderowymi
- Lepsza organizacja kodu

Dla projektu z wieloma etapami renderowania (scena, post-processing, wyświetlanie), UBO mogą przechowywać wspólne dane: macierze kamery, parametry czasu, ustawienia jakości.

---

## 2. Zaawansowane Techniki Renderingu dla AI Super Resolution

### 2.1 Framebuffer Objects (FBO) – Klucz do Dynamic Resolution

#### 2.1.1 Tworzenie i konfiguracja FBO

Framebuffer Objects stanowią techniczną podstawę dla całej koncepcji AI-enhanced rendering. Bez FBO niemożliwe byłoby renderowanie sceny w niższej rozdzielczości niż wyświetlacz.

```cpp
struct Framebuffer {
        GLuint fbo;
        GLuint colorTexture;
        GLuint depthTexture; // lub depthRenderbuffer
        int width, height;
};

Framebuffer createFramebuffer(int width, int height, bool useDepth = true) {
        Framebuffer fb;
        fb.width = width;
        fb.height = height;
        
        // Generowanie FBO
        glGenFramebuffers(1, &fb.fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fb.fbo);
        
        // Tworzenie tekstury koloru
        glGenTextures(1, &fb.colorTexture);
        glBindTexture(GL_TEXTURE_2D, fb.colorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
                                 GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        // Dołączenie tekstury koloru do FBO
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                                    GL_TEXTURE_2D, fb.colorTexture, 0);
        
        // Opcjonalnie: bufor głębokości
        if (useDepth) {
                glGenTextures(1, &fb.depthTexture);
                glBindTexture(GL_TEXTURE_2D, fb.depthTexture);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0,
                                         GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                                            GL_TEXTURE_2D, fb.depthTexture, 0);
        }
        
        // Sprawdzenie kompletności
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                std::cerr << "Framebuffer incomplete!" << std::endl;
        }
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return fb;
}
```

> `GL_LINEAR` dla filtrów zapewnia bilinearną interpolację (istotne gdy tekstura jest próbkowana w innym rozmiarze). `GL_CLAMP_TO_EDGE` zapobiega artefaktom na brzegach.

#### 2.1.2 Attachment points: color, depth, stencil buffers

| Attachment | Cel | Typowe formaty | Uwagi |
|-----------|-----|---------------|-------|
| `GL_COLOR_ATTACHMENT0`–`7` | Bufor koloru | `GL_RGBA8`, `GL_RGBA16F`, `GL_RGBA32F`, `GL_R11F_G11F_B10F` | Do 8 równocześnie w MRT |
| `GL_DEPTH_ATTACHMENT` | Bufor głębokości | `GL_DEPTH_COMPONENT16`, `GL_DEPTH_COMPONENT24`, `GL_DEPTH_COMPONENT32F` | Tekstura lub renderbuffer |
| `GL_STENCIL_ATTACHMENT` | Bufor szablonu | `GL_STENCIL_INDEX8` | Rzadko używany samodzielnie |
| `GL_DEPTH_STENCIL_ATTACHMENT` | Połączony bufor | `GL_DEPTH24_STENCIL8`, `GL_DEPTH32F_STENCIL8` | Efektywniejsze niż osobne attachmenty |

Dla projektu super-resolution:
- **`GL_COLOR_ATTACHMENT0`** — wymagany do przechwycenia renderowanej sceny
- **`GL_DEPTH_ATTACHMENT`** — potrzebny jeśli scena 3D używa testu głębokości
- Wybór między **teksturą a renderbufferem** dla głębokości: renderbuffer jest bardziej efektywny jeśli głębokość nie jest potrzebna w post-processingu; tekstura wymagana dla SSAO, depth of field itp.

#### 2.1.3 Renderowanie do tekstury (render-to-texture)

```cpp
void renderSceneToFramebuffer(const Framebuffer& fb, GLuint sceneShader, 
                                                             GLuint sceneVAO, const glm::mat4& mvp) {
        // Związanie FBO jako celu renderowania
        glBindFramebuffer(GL_FRAMEBUFFER, fb.fbo);
        glViewport(0, 0, fb.width, fb.height);
        
        // Czyszczenie
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Renderowanie sceny
        glUseProgram(sceneShader);
        glUniformMatrix4fv(glGetUniformLocation(sceneShader, "mvp"), 
                                             1, GL_FALSE, glm::value_ptr(mvp));
        glBindVertexArray(sceneVAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        
        // Przywrócenie domyślnego framebuffera
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
```

Po tym wywołaniu, `fb.colorTexture` zawiera wyrenderowaną scenę i może być użyta jako:
- **(a)** Dane wejściowe dla tradycyjnego upscalingu w shaderze
- **(b)** Dane do przesłania do CPU dla procesowania AI
- **(c)** Dane wejściowe dla compute shadera implementującego AI

#### 2.1.4 Sprawdzanie kompletności framebuffera

| Status | Znaczenie | Typowa przyczyna |
|--------|-----------|-----------------|
| `GL_FRAMEBUFFER_COMPLETE` | ✅ Sukces | — |
| `GL_FRAMEBUFFER_UNDEFINED` | Domyślny framebuffer nie istnieje | Niezainicjalizowany kontekst |
| `GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT` | Nieprawidłowy attachment | Niezainicjalizowana tekstura |
| `GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT` | Brak attachmentu koloru | Zapomniane `glFramebufferTexture2D` |
| `GL_FRAMEBUFFER_UNSUPPORTED` | Niekompatybilne formaty | Problematyczna kombinacja formatów na danym GPU |
| `GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS` | Różne rozmiary attachmentów | Niedopasowane `glTexImage2D` |

> Weryfikacja powinna być wykonywana po każdej zmianie konfiguracji FBO, ze szczególnym uwzględnieniem dynamicznej zmiany rozdzielczości.

#### 2.1.5 Multiple Render Targets (MRT) dla zaawansowanych efektów

MRT umożliwiają jednoczesne zapisywanie do wielu tekstur koloru w jednym przebiegu renderowania. Dla projektu super-resolution, MRT może generować dodatkowe bufory informacyjne:

- Normalne w przestrzeni ekranu
- Pozycje w przestrzeni świata
- Identyfikatory materiałów

Te dane mogą wspomagać *perceptualne* super-resolution, gdzie model AI otrzymuje nie tylko kolor, ale również informacje o geometrii i materiałach.

Konfiguracja MRT wymaga:
1. Dołączenia wielu tekstur koloru do różnych `GL_COLOR_ATTACHMENT`
2. Ustawienia `glDrawBuffers` z listą aktywnych attachmentów
3. Modyfikacji fragment shadera: `layout(location = n) out vec4 colorN;`

---

### 2.2 Dynamic Resolution Rendering

#### 2.2.1 Renderowanie sceny w niższej rozdzielczości

Dynamic Resolution Rendering (DRR) to strategia adaptacyjnej zmiany rozdzielczości renderingu w odpowiedzi na obciążenie GPU, z kompensacją jakości poprzez upscaling.

| Skala liniowa | Skala powierzchni | Dla 1080p docelowej | Redukcja pikseli | Zastosowanie |
|:---:|:---:|:---:|:---:|---|
| 0.50× | 0.25× | 960×540 | 75% | Maksymalna wydajność, szybka AI |
| 0.67× | 0.44× | 1280×720 | 56% | Zbalansowane ustawienie |
| 0.75× | 0.56× | 1440×810 | 44% | Wysoka jakość, wolniejsza AI |
| 0.90× | 0.81× | 1728×972 | 19% | Minimalny gain, anti-aliasing |

> **💡 Kluczowa obserwacja:** Redukcja jest kwadratowa względem wymiaru liniowego: 0.5× rozdzielczość liniowa = 0.25× pikseli. Oszczędzony czas GPU jest "inwestowany" w przetwarzanie AI.

```cpp
class DynamicResolutionRenderer {
public:
        int displayWidth, displayHeight;
        float resolutionScale = 0.67f;
        Framebuffer lowResFB;
        
        void setResolutionScale(float scale) {
                resolutionScale = glm::clamp(scale, 0.25f, 1.0f);
                int newWidth = static_cast<int>(displayWidth * resolutionScale);
                int newHeight = static_cast<int>(displayHeight * resolutionScale);
                
                // Upewnienie się, że wymiary są parzyste (wymaganie niektórych modeli AI)
                newWidth = (newWidth / 2) * 2;
                newHeight = (newHeight / 2) * 2;
                
                // Przearanżowanie FBO jeśli rozmiar się zmienił
                if (newWidth != lowResFB.width || newHeight != lowResFB.height) {
                        // Destroy old framebuffer and create new one
                }
        }
};
```

#### 2.2.2 Skalowanie w górę – tradycyjne metody (bilinear, bicubic)

| Metoda | Jakość | Wydajność | Implementacja | Artefakty |
|--------|--------|-----------|--------------|-----------|
| Bilinearna | Niska | Bardzo wysoka | Sprzętowa (`GL_LINEAR`) | Rozmyte krawędzie, utrata detali |
| Bikubicna | Średnia | Średnia | Shader (16 próbek) | Halo artifacts, ringing |
| Lanczos | Wysoka | Niska | Shader (więcej próbek) | Ringing przy ostrych krawędziach |
| **AI Super-Resolution** | **Bardzo wysoka** | Zmienna | CPU (OpenCV/ONNX) lub GPU | Rzadkie, specyficzne dla modelu |

Te tradycyjne metody stanowią **baseline** dla oceny AI super-resolution – model neuronowy powinien znacząco je przewyższać pod względem jakości percepcyjnej.

#### 2.2.3 Proporcje aspektowe i obsługa różnych rozdzielczości

Dynamic resolution musi zachować prawidłowe proporcje aspektowe. Typowo skalujemy oba wymiary proporcjonalnie: dla 16:9 i targetu 1080p, 0.5× DRR daje 960×540.

Dla AI super-resolution, model jest zazwyczaj trenowany na kwadratowych patchach lub proporcjach 1:1, więc nieproporcjonalne dane mogą wymagać preprocessingu.

#### 2.2.4 Pomiar i adaptacja jakości w czasie rzeczywistym

System DRR wymaga feedback loop: mierzymy czas renderowania klatki, dostosowujemy rozdzielczość dla utrzymania targetu FPS. Dla AI-enhanced, musimy uwzględnić czas inferencji – jeśli model AI działa na CPU, możemy pozwolić GPU na niższą rozdzielczość, "inwestując" oszczędność w jakość AI.

---

### 2.3 Post-processing i Full-Screen Effects

#### 2.3.1 Renderowanie quadów pełnoekranowych

Full-screen quad to technika renderowania prostokąta pokrywającego cały viewport. Vertex shader przepuszcza pozycje bez transformacji, fragment shader próbkuje teksturę źródłową i aplikuje efekt.

Dla AI super-resolution ten sam mechanizm służy do:
- Wyświetlania wyniku upscalingu
- Opcjonalnego post-processingu (gamma, tone mapping)
- W wersji zaawansowanej – samej inferencji neuronowej w shaderze

#### 2.3.2 Gamma correction i tone mapping

Gamma correction kompensuje nieliniowość percepcji ludzkiego wzroku:

```glsl
corrected = pow(linear, 1.0 / 2.2);
```

| Operator Tone Mapping | Charakterystyka | Zastosowanie |
|----------------------|----------------|-------------|
| **Reinhard** | Prosty, może produkować szare obrazy | Szybkie prototypowanie |
| **Filmic** | Bardziej kontrastowy, inspirowany filmem | Gry, cinematic look |
| **ACES** | Standard przemysłowy, najbardziej zbalansowany | Produkcja profesjonalna |

> **⚠️ Uwaga:** Modele SR są zazwyczaj trenowane na danych w przestrzeni gamma (sRGB), więc konwersja między przestrzeniami musi być starannie zarządzana.

#### 2.3.3 Podstawowe filtry konwolucyjne w fragment shaderach

Konwolucje w fragment shaderach implementują próbkowanie sąsiednich pikseli. Kernele 3×3 wymagają 9 próbek; separowalne kernele (np. Gaussian) redukują do 2×N. Te operacje są fundamentalne dla zrozumienia, jak działają sieci konwolucyjne używane w super-resolution – warstwy `conv2d` w CNN to **nauczalne konwolucje**.

---

## 3. Compute Shaders – Obliczenia GPU dla AI

### 3.1 Wprowadzenie do Compute Shaders (OpenGL 4.3+)

#### 3.1.1 Architektura wykonania: work groups, local/global invocation ID

Compute shadery pozwalają na wykonywanie ogólnych obliczeń równoległych bez konieczności tworzenia geometrii, rasterizacji czy renderowania do framebuffera. Dla projektu AI-Enhanced Renderer otwierają możliwość implementacji **inferencji neuronowej bezpośrednio na GPU**.

Model wykonania jest zorganizowany hierarchicznie:

| Poziom | Opis | Identyfikator w GLSL |
|--------|------|---------------------|
| **Dispatch** | Wywołanie `glDispatchCompute(num_groups_x, y, z)` | — |
| **Work group** | Kolekcja invocationów, współdzieląca pamięć | `gl_WorkGroupID` |
| **Local invocation** | Pojedynczy wątek w work group | `gl_LocalInvocationID`, `gl_LocalInvocationIndex` |
| **Global invocation** | Unikalny identyfikator w całym dispatchu | `gl_GlobalInvocationID` |

> Dla maksymalnej wydajności zaleca się wielokrotności **32** lub **64** (rozmiary warpów/wavefrontów). Popularna konfiguracja: `layout(local_size_x = 16, local_size_y = 16)` → 256 invocationów.

#### 3.1.2 Deklaracja i dispatch compute shaderów

```glsl
#version 430 core
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(rgba8, binding = 0) uniform readonly image2D inputImage;
layout(rgba8, binding = 1) uniform writeonly image2D outputImage;

void main() {
        ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
        // ... przetwarzanie ...
        imageStore(outputImage, coords, result);
}
```

Dispatch w C++:

```cpp
glDispatchCompute(width / 16 + 1, height / 16 + 1, 1);
```

#### 3.1.3 Memory barriers i synchronizacja

| Funkcja | Zakres synchronizacji | Zastosowanie |
|---------|----------------------|-------------|
| `barrier()` | Wszystkie invocationy w work group | Po załadowaniu do shared memory |
| `memoryBarrier()` | Globalna pamięć | Gwarancja widoczności zapisów |
| `memoryBarrierShared()` | Tylko shared memory | Optymalizacja wewnątrz work group |
| `memoryBarrierImage()` | Obrazki (image load/store) | Między dispatchami obrazków |
| `glMemoryBarrier()` (CPU) | Między dispatchami | Po `glDispatchCompute`, przed kolejnym użyciem |

Dla AI super-resolution z wieloma warstwami, prawidłowe użycie barier jest krytyczne:

```
dispatch etapu 1 → glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT) → dispatch etapu 2
```

#### 3.1.4 Shared memory dla optymalizacji dostępu do danych

Shared memory to ultra-szybka pamięć dostępna dla wszystkich invocationów w work group:

```glsl
shared float tile[18][18];
```

**Typowe użycie (tiling):** każdy invocation ładuje jeden element do shared memory → `barrier()` → wszystkie invocationy operują na lokalnych danych. Dla konwolucji 3×3, zamiast 9 globalnych odczytów na piksel, shared memory z halo regionem redukuje odczyty globalne do ~1 na piksel (amortyzowane).

> Ograniczenie: typowo **16–64 KB** na work group.

---

### 3.2 Przetwarzanie Tekstur w Compute Shaders

#### 3.2.1 imageLoad/imageStore – bezpośredni dostęp do texeli

W przeciwieństwie do `texture()` w fragment shaderach, compute shadery używają `imageLoad`/`imageStore`:

```cpp
// C++ - bindowanie image unit
glBindImageTexture(unit, texture, level, layered, layer, access, format);
```

```glsl
// GLSL
layout(rgba32f, binding = 0) uniform image2D inputImage;
```

**Kluczowa zaleta:** możliwość read-modify-write oraz *scatter writes* – zapisy do dowolnych lokacji.

#### 3.2.2 Formaty tekstur dla obliczeń

| Format | Precyzja | Pamięć | Zastosowanie |
|--------|---------|--------|-------------|
| `RGBA32F` | Pełna (32-bit float) | 128 bit/piksel | Wagi modelu, aktywacje w głębokich sieciach |
| `RGBA16F` | Połówkowa (16-bit float) | 64 bit/piksel | Aktywacje po kwantyzacji, kompromis jakość/pamięć |
| `R32F` / `R16F` | Jednokanałowa | 32/16 bit/piksel | Mapy głębokości, single-channel features |
| `R8` / `RGBA8` | 8-bitowa | 8/32 bit/piksel | Dane we/wy, kwantyzowane modele |

> Dla prototypowania zaleca się zacząć od **FP16**, a optymalizować później.

#### 3.2.3 Konwolucje i operacje sąsiedztwa

Dla głębokich sieci super-resolution (np. EDSR z 32 residual blocks), każda warstwa wymaga osobnego dispatchu, co może prowadzić do setek dispatchów na klatkę.

**Techniki optymalizacji:**
- Fuzja warstw (łączenie `conv` + `relu` w jeden shader)
- Redukcja precyzji
- Użycie tekstur z mipmapping dla wczesnych warstw

#### 3.2.4 Porównanie wydajności: compute vs fragment shaders

| Aspekt | Compute Shadery | Fragment Shadery |
|--------|----------------|-----------------|
| Kontrola nad tilingiem | Pełna przez shared memory | Ograniczona |
| Scatter operations | ✅ Tak | ❌ Nie |
| Overhead rasterizacji | Brak | Obecny |
| Integracja z pipeline | Wymaga barier | Bezpośrednia |
| Optymalizacja sterownika | Mniej dojrzała | Bardziej dojrzała |
| **Najlepsze dla** | Złożone operacje AI, wiele warstw | Prosty post-processing, finalne wyświetlanie |

> **💡 Rekomendacja:** Zacząć od fragment shaderów (prostsze), migrować do compute shaderów dla wąskich gardeł.

---

## 4. Modele AI dla Super Resolution – Wybór i Przygotowanie

### 4.1 Przegląd Architektur Super Resolution

| Architektura | Jakość | Prędkość | Wymagania pamięciowe | Najlepsze zastosowanie |
|:---:|:---:|:---:|:---:|---|
| **EDSR** | ⭐⭐⭐⭐⭐ | 🐌 Wolna | Wysokie | Statyczne obrazy, maksymalna jakość |
| **ESPCN** | ⭐⭐⭐ | 🚀 Bardzo szybka | Niskie | Real-time, ograniczone zasoby |
| **FSRCNN** | ⭐⭐⭐⭐ | ⚡ Szybka | Średnie | **Rekomendowane dla początkujących** |
| **LapSRN** | ⭐⭐⭐⭐ | 🏃 Średnia | Średnie | Progresywne upscaling, wieloskalowe |

#### 4.1.1 EDSR (Enhanced Deep Residual Networks)

EDSR (*Lim et al., 2017*) maksymalizuje jakość rekonstrukcji. Architektura opiera się na głębokich sieciach residualnych z usuniętymi warstwami batch normalization.

**Kluczowe cechy:**
- Znaczna głębokość (typowo 32 bloki residualne z 256 kanałami)
- Brak downsamplingu wewnątrz sieci
- Rozbudowane połączenia skip

Jakość jest najwyższa spośród dostępnych opcji, ale EDSR jest najbardziej zasobożerny. W module OpenCV `dnn_superres` dostępny ze współczynnikami **2×**, **3×** i **4×**.

#### 4.1.2 ESPCN (Efficient Sub-Pixel Convolutional Network)

ESPCN (*Shi et al., 2016*) wprowadza innowacyjną technikę **sub-pixel convolution** – zamiast powiększać feature maps przed konwolucjami, wykonuje konwolucje w niskiej rozdzielczości, a następnie rearranges kanały wyjściowe. To czyni ESPCN **najszybszą opcją** w OpenCV `dnn_superres`.

Kompromisem jest jakość – mniej ostre krawędzie przy większych współczynnikach skalowania.

#### 4.1.3 FSRCNN (Fast Super-Resolution CNN) — rekomendowany

FSRCNN oferuje najlepszy stosunek jakości do prędkości:
- Zmniejszony rozmiar filtrów (3×3 zamiast 9×9)
- Większa głębokość z mniejszą szerokością
- Zaawansowana technika *shrinking* i *expanding*

> **✅ Rekomendacja:** FSRCNN to najlepszy wybór dla początkujących – oferuje balans między łatwością implementacji, jakością wyników i wymaganiami wydajnościowymi. Modele są szeroko dostępne w pre-trenowanej formie.

#### 4.1.4 LapSRN (Laplacian Pyramid Super-Resolution Network)

LapSRN wykorzystuje piramidę Laplace'a do progresywnego upscalingu, generując wyniki pośrednie przy różnych skalach. Główna zaleta: wiele skal (2×, 4×, 8×) z jednego przejścia. Jednak dla pojedynczego, ustalonego współczynnika skalowania, może być mniej efektywny.

#### 4.1.5 Real-ESRGAN – zaawansowana architektura

Real-ESRGAN wykorzystuje zaawansowane modele degradacji obejmujące różnorodne artefakty spotykane w praktyce. Oferuje wyjątkową jakość ale wymaga sekund na klatkę nawet na potężnym sprzęcie.

> Dla projektu real-time: opcja jakościowa dla statycznych scen lub screenshotów, z fallbackiem do szybszych modeli.

---

### 4.2 Przygotowanie Modelu do Wdrożenia

#### 4.2.1 Eksport modelu do ONNX

```python
import torch
import torch.onnx

# Załadowanie wytrenowanego modelu
model = torch.load('fsrcnn_x2.pth')
model.eval()

# Przykładowe wejście
dummy_input = torch.randn(1, 3, 256, 256)

# Eksport do ONNX
torch.onnx.export(
        model, 
        dummy_input, 
        'fsrcnn_x2.onnx',
        export_params=True,
        opset_version=11,
        do_constant_folding=True,
        input_names=['input'],
        output_names=['output'],
        dynamic_axes={
                'input':  {0: 'batch_size', 2: 'height', 3: 'width'},
                'output': {0: 'batch_size', 2: 'height', 3: 'width'}
        }
)
```

#### 4.2.2 Kwantyzacja i optymalizacja

```python
from onnxruntime.quantization import quantize_dynamic, QuantType

quantize_dynamic('fsrcnn_x2.onnx', 'fsrcnn_x2_int8.onnx', weight_type=QuantType.QInt8)
```

Dodatkowe techniki: *pruning* (usuwanie nieistotnych wag), *knowledge distillation* (trenowanie mniejszego modelu na wyjściach większego).

#### 4.2.3 Konwersja do formatów mobilnych

| Format | Zalety | Najlepsze dla |
|--------|-------|--------------|
| **TensorFlow Lite** | Optymalizacja dla ARM, delegaty GPU/NPU | Android, iOS, embedded Linux |
| **ncnn** | Wysoka wydajność na mobilnych GPU, brak zależności | Mobile GPU, Vulkan-capable devices |
| **ONNX Runtime Mobile** | Zgodność z desktopowym ONNX | Cross-platform mobile |

#### 4.2.4 Weryfikacja jakości

| Metryka | Opis | Typowe wartości | Ograniczenia |
|---------|------|----------------|-------------|
| **PSNR** | Peak Signal-to-Noise Ratio | >30 dB dobre, >40 dB bardzo dobre | Niekoreluje z percepcją |
| **SSIM** | Structural Similarity Index | >0.9 dobre, >0.95 bardzo dobre | Lepiej koreluje z percepcją |
| **LPIPS** | Learned Perceptual Image Patch Similarity | Niższe = lepsze | Najlepiej koreluje z percepcją |

> Weryfikacja powinna obejmować zarówno metryki, jak i **subiektywną ocenę** na różnorodnych typach zawartości.

---

## 5. Integracja AI z Pipeline'm Renderingu OpenGL

### 5.1 Strategie Integracji – Porównanie Podejść

| Podejście | Złożoność | Wydajność | Jakość | Rekomendacja |
|-----------|:---------:|:---------:|:------:|-------------|
| **CPU: OpenCV + ONNX Runtime** | 🟢 Niska | 🟡 Średnia (transfer CPU↔GPU) | 🟢 Wysoka | ✅ **Dla początkujących** |
| **GPU: ShaderNN (GLSL)** | 🔴 Bardzo wysoka | 🟢 Wysoka (zero-copy) | 🟡 Zależna | Zaawansowani, specyficzne przypadki |
| **Hybrydowe: Vulkan/NCNN** | 🟠 Wysoka | 🟢 Bardzo wysoka | 🟢 Wysoka | Produkcja, cross-platform |

#### 5.1.1 Podejście CPU: OpenCV + ONNX Runtime

Oferuje największą elastyczność i dojrzałe ekosystemy. Kluczowe wyzwanie: transfer danych GPU ↔ CPU.

##### 5.1.1.1 Przechwytywanie ramki: glReadPixels vs PBO

Tradycyjne `glReadPixels` bez PBO blokuje CPU. **Pixel Buffer Objects (PBO)** umożliwiają asynchroniczny transfer:

```cpp
// Tworzenie PBO
GLuint pbo;
glGenBuffers(1, &pbo);
glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
glBufferData(GL_PIXEL_PACK_BUFFER, width * height * 4, NULL, GL_STREAM_READ);

// Asynchroniczny readback
glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, 0);
// CPU może kontynuować pracę...

// Później, gdy dane są potrzebne:
void* ptr = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
// Kopiowanie do cv::Mat lub bezpośrednie użycie
glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
```

> **💡 Tip:** Double lub triple buffering PBO pozwala na overlapping operacji GPU i CPU.

##### 5.1.1.2 Konwersja danych OpenGL do cv::Mat

```cpp
cv::Mat inputRGBA(height, width, CV_8UC4, pboData);
cv::Mat inputRGB;
cv::cvtColor(inputRGBA, inputRGB, cv::COLOR_RGBA2RGB);

cv::Mat inputFloat;
inputRGB.convertTo(inputFloat, CV_32F, 1.0 / 255.0);
```

##### 5.1.1.3 Inferencja przez OpenCV dnn_superres lub ONNX Runtime

**OpenCV dnn_superres** — uproszczony interfejs:

```cpp
cv::dnn_superres::DnnSuperResImpl sr;
sr.readModel("FSRCNN_x2.pb");
sr.setModel("fsrcnn", 2);

cv::Mat output;
sr.upsample(modelInput, output); // output ma 2x rozmiar wejścia
```

**ONNX Runtime** — większa elastyczność:

```cpp
Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "SuperRes");
Ort::SessionOptions sessionOptions;
sessionOptions.SetIntraOpNumThreads(4);
sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

Ort::Session session(env, "fsrcnn_x2.onnx", sessionOptions);

// Przygotowanie input tensor
std::vector<int64_t> inputShape = {1, 3, height, width};
Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
        memoryInfo, modelInput.ptr<float>(), inputSize, 
        inputShape.data(), inputShape.size());

// Inferencja
auto outputTensors = session.Run(Ort::RunOptions{nullptr}, 
                                                                    inputNames, &inputTensor, 1,
                                                                    outputNames, 1);

float* outputData = outputTensors[0].GetTensorMutableData<float>();
```

##### 5.1.1.4 Upload wyniku z powrotem do GPU

```cpp
cv::Mat outputUInt8;
output.convertTo(outputUInt8, CV_8U, 255.0);

cv::Mat outputRGBA;
cv::cvtColor(outputUInt8, outputRGBA, cv::COLOR_RGB2RGBA);

glBindTexture(GL_TEXTURE_2D, resultTexture);
glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, outputWidth, outputHeight,
                                GL_RGBA, GL_UNSIGNED_BYTE, outputRGBA.data());
```

##### 5.1.1.5 Optymalizacje: asynchroniczny readback, double buffering

| Technika | Implementacja | Oczekiwany gain |
|---------|--------------|----------------|
| Double buffering PBO | Dwa PBO, naprzemienne użycie | 20–40% redukcji latency |
| Triple buffering | Trzy PBO, overlapping render/AI/display | 30–50% redukcji latency |
| Asynchroniczna inferencja | ONNX Runtime async API | Overlapping CPU AI z GPU render |
| Pinned memory | `cudaHostAlloc` / `VirtualLock` | Szybszy transfer CPU↔GPU |

#### 5.1.2 Podejście GPU: ShaderNN i Inferencja w GLSL

**ShaderNN** (OPPO) implementuje inferencję CNN bezpośrednio w GLSL z zero-copy data flow.

| Komponent | Zastosowanie | Przykłady operacji |
|-----------|-------------|-------------------|
| Fragment shaders | Proste operacje, regularny dostęp | Konwolucje 3×3, aktywacje, element-wise |
| Compute shaders | Złożone operacje, shared memory | Duże kernele, redukcje, atomics |

##### Implementacja warstwy conv w GLSL

```glsl
#version 330 core
uniform sampler2D inputTexture;
uniform sampler2D weights;
uniform vec2 inputSize;
uniform vec2 outputSize;

out vec4 fragColor;

void main() {
        vec2 texCoord = gl_FragCoord.xy / outputSize;
        vec2 inputTexel = 1.0 / inputSize;
        
        vec4 result = vec4(0.0);
        for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                        vec2 offset = vec2(float(kx), float(ky)) * inputTexel;
                        vec4 sample = texture(inputTexture, texCoord + offset);
                        float weight = texture(weights, 
                                vec2((kx+1 + (ky+1)*3) / 9.0, 0.0)).r;
                        result += sample * weight;
                }
        }
        fragColor = max(result, 0.0); // ReLU
}
```

**Ograniczenia:** złożoność konwersji modelu, ograniczona precyzja GLSL, brak wsparcia dla wszystkich operacji.

#### 5.1.3 Podejście Hybrydowe: Vulkan/NCNN

**Real-ESRGAN-ncnn-vulkan** demonstruje produkcyjną jakość: 100–300ms dla obrazów 720p na współczesnych GPU mobilnych. Wymaga interoperacyjności OpenGL ↔ Vulkan przez `VK_KHR_external_memory`.

> Warto rozważyć pełne przejście na Vulkan gdy: projekt jest na wczesnym etapie, wymagana jest maksymalna wydajność, lub planowany jest ray tracing.

---

### 5.2 Szczegółowa Implementacja Podejścia CPU (Rekomendowana dla Początkujących)

#### 5.2.1 Konfiguracja Środowiska

**Ubuntu/Debian:**
```bash
sudo apt-get install libopencv-dev libopencv-contrib-dev
```

**Windows (vcpkg):**
```bash
vcpkg install opencv4[contrib,dnn]
```

**CMakeLists.txt:**

```cmake
cmake_minimum_required(VERSION 3.16)
project(AISuperResolutionRenderer)

find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(OpenCV REQUIRED COMPONENTS core imgproc dnn dnn_superres)

# GLAD jako podprojekt
add_subdirectory(third_party/glad)

add_executable(renderer main.cpp)
target_link_libraries(renderer 
        OpenGL::GL 
        glfw 
        glad
        ${OpenCV_LIBS}
)
```

#### 5.2.2 Struktura Aplikacji

##### Główna pętla renderowania

```cpp
while (!glfwWindowShouldClose(window)) {
        // Etap 1: Renderowanie sceny do FBO w niskiej rozdzielczości
        renderer.renderSceneToFBO(scene, lowResFB);
        
        // Etap 2: Asynchroniczny readback do PBO
        frameCapture.beginReadback(lowResFB.colorTexture);
        
        // Etap 3: Jeśli poprzednia klatka gotowa, inferencja AI
        if (frameCapture.isPreviousFrameReady()) {
                cv::Mat lowResFrame = frameCapture.getFrame();
                cv::Mat highResFrame = aiSuperResolver.upsample(lowResFrame);
                resultTexture.upload(highResFrame);
        }
        
        // Etap 4: Wyświetlenie wyniku
        renderer.displayResult(resultTexture);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
}
```

##### Kluczowe klasy

```cpp
class FrameCapture {
public:
        void initialize(int width, int height);
        void beginReadback(GLuint texture);
        bool isPreviousFrameReady();
        cv::Mat getFrame();
private:
        GLuint pbo[2]; // double buffering
        int currentPBO = 0;
        cv::Mat stagingBuffer;
};

class AISuperResolver {
public:
        bool loadModel(const std::string& modelPath, 
                                     const std::string& modelName, int scale);
        cv::Mat upsample(const cv::Mat& input);
private:
        cv::dnn_superres::DnnSuperResImpl sr;
};

class RenderTarget {
public:
        void create(int width, int height, bool useDepth = true);
        void destroy();
        void bind();
        void unbind();
        void resize(int newWidth, int newHeight);
        GLuint getColorTexture() const { return colorTexture; }
private:
        Framebuffer fb;
};
```

#### 5.2.3 Przepływ Danych w Czasie Rzeczywistym

| Etap | Operacja | Rozdzielczość | Format | Czas typowy |
|:----:|---------|:------------:|:------:|:----------:|
| 1 | Renderowanie sceny do FBO | 960×540 | RGBA8 | 2–5 ms |
| 2 | Asynchroniczny readback PBO | 960×540 | RGBA8 | 0.5–1 ms |
| 3 | Preprocessing (RGBA→RGB, float) | 960×540 | RGB32F | 0.5–1 ms |
| 4 | **Inferencja AI (FSRCNN 2×)** | 540→1080 | RGB32F | **10–50 ms** |
| 5 | Postprocessing (denormalizacja) | 1920×1080 | RGBA8 | 0.5–1 ms |
| 6 | Upload tekstury i wyświetlenie | 1920×1080 | RGBA8 | 0.5–1 ms |

**Łączny czas:** 15–60 ms (**17–67 FPS**), zależnie od sprzętu i modelu.

---

### 5.3 Optymalizacje Wydajnościowe

#### 5.3.1 Redukcja Opóźnień (Latency)

**Triple buffering:** bufor A renderowany przez GPU, bufor B przetwarzany przez AI na CPU, bufor C wyświetlany. Gdy GPU kończy A, zaczyna nowy rendering zamiast czekać na zwolnienie B.

**Overlapping:** podczas gdy CPU przetwarza klatkę N przez AI, GPU renderuje klatkę N+1.

**Dynamiczna adaptacja:** system monitoruje czas renderingu + inferencji i dostosowuje `resolutionScale`.

#### 5.3.2 Jakość vs Prędkość – Parametryzacja

| Scenariusz | Rekomendowany model | Rozdzielczość renderingu | Oczekiwany FPS |
|-----------|:-------------------:|:------------------------:|:--------------:|
| Wysokie obciążenie GPU | ESPCN | 540p | 60+ |
| Zbalansowane | FSRCNN | 720p | 30–60 |
| Niskie obciążenie, wysoka jakość | EDSR | 810p | ~30 |

**Region-of-interest processing** — upscaluj ważne obszary pełnym modelem, resztę tradycyjnymi metodami.

**Temporalne stabilizowanie** — wykorzystanie informacji z poprzednich klatek redukuje flickering i poprawia spójność czasową.

---

## 6. Zaawansowane Techniki i Rozszerzenia

### 6.1 Per-Object AI Enhancement

- **Identyfikacja obiektów:** object detection (YOLO), semantic segmentation (U-Net), lub heurystyki (głębokość, ruch, rozmiar)
- **Selektywny upscaling:** maska binarna/wagowa do blendowania AI-upscalowanego z tradycyjnie upscalowanym obrazem
- **Integracja z deferred rendering:** G-buffer (pozycja, normalna, albedo) wspomaga AI w rekonstrukcji krawędzi i adaptacyjnym upscalingu

### 6.2 Temporalne Super Resolution (TSR)

- **Akumulacja informacji czasowej:** efektywne zwiększanie "próbkowania" bez zwiększania rozdzielczości
- **Motion vectors:** projekcja poprzednich klatek do aktualnej przestrzeni ekranu
- **Redukcja ghostingu:** rejection pikseli zablokowanych okluzją, clamping kolorów, history validation

### 6.3 Integracja z Istniejącymi Frameworkami

| Framework | Dostępność | Platformy | Kluczowe cechy | Kiedy użyć |
|-----------|-----------|-----------|----------------|-----------|
| **NVIDIA DLSS** | Proprietary, wymaga RTX | Windows, Linux (Proton) | Najwyższa jakość i wydajność | Produkcja komercyjna |
| **AMD FSR** | Open source | Wszystkie z Vulkan/DX12 | Dobra jakość, łatwa integracja | Cross-platform, open source |
| **Intel XeSS** | Cross-platform | Intel Arc, inne przez DP4a | Konkurencyjna jakość | Systemy Intel |

---

## 7. Roadmapa Projektu – Od Początku do Końca

### Faza 1: Podstawowy Renderer OpenGL (2–3 tygodnie)

| Tydzień | Zadania | Deliverable |
|:-------:|---------|------------|
| 1 | Setup CMake, GLFW, GLAD, GLM; okno i kontekst | Kompilujący się szkielet |
| 1–2 | VAO/VBO, podstawowe shadery, rysowanie trójkąta | Kolorowy trójkąt w 3D |
| 2 | Macierze transformacji, kamera, prosta scena | Interaktywna scena 3D |
| 2–3 | Oświetlenie Phonga, teksturowanie, materiały | Teksturowany model z oświetleniem |
| 3 | FBO, render-to-texture, podstawowy post-process | Scena renderowana do tekstury |

### Faza 2: Dynamic Resolution i Upscaling (1–2 tygodnie)

| Tydzień | Zadania | Deliverable |
|:-------:|---------|------------|
| 3–4 | Zmienna rozdzielczość FBO, pomiar wydajności | Renderer z dynamicznym DRR |
| 4 | Bilinearny i bikubicny upscaling w shaderze | Porównanie jakości metod tradycyjnych |
| 4 | Full-screen quad, gamma correction, tone mapping | Polerowany pipeline post-processingu |

### Faza 3: Integracja AI Super Resolution (3–4 tygodnie)

| Tydzień | Zadania | Deliverable |
|:-------:|---------|------------|
| 5 | Instalacja OpenCV dnn_superres, wybór modelu | FSRCNN działający na statycznych obrazach |
| 5–6 | PBO readback, konwersja danych OpenCV↔OpenGL | Asynchroniczny transfer ramki |
| 6–7 | Integracja w pętli renderowania, debugowanie | AI upscaling w czasie rzeczywistym |
| 7–8 | Optymalizacje: double/triple buffering, threading | Płynne 30+ FPS z AI |
| 8 | Porównanie modeli, tuning parametrów | Konfigurowalny system jakość/wydajność |

### Faza 4: Polerowanie i Zaawansowane Funkcje (2–3 tygodnie)

| Tydzień | Zadania | Deliverable |
|:-------:|---------|------------|
| 8–9 | ImGui dla konfiguracji w czasie rzeczywistym | GUI z suwakami jakości, wyboru modelu |
| 9–10 | Porównanie A/B, metryki jakości | Systematyczna ocena konfiguracji |
| 10–11 | Opcjonalnie: compute shadery, temporal SR, per-object | Zaawansowane funkcje prototypowe |
| 11 | Dokumentacja, cleanup, release | ✅ Gotowy projekt do portfolio |

---

## 8. Zasoby i Materiały Uzupełniające

### 8.1 Kluczowe Tutoriale i Dokumentacja

| Zasób | Opis | Link |
|-------|------|------|
| **LearnOpenGL** | Kompletny kurs Modern OpenGL | [learnopengl.com](https://learnopengl.com) |
| **OpenGL 4.3 Spec** | Oficjalna specyfikacja compute shaderów | [khronos.org/registry/OpenGL](https://www.khronos.org/registry/OpenGL/) |
| **OpenCV dnn_superres** | Tutorial i dokumentacja modułu | [docs.opencv.org](https://docs.opencv.org) |

### 8.2 Repozytoria Referencyjne

| Projekt | Opis | Link |
|---------|------|------|
| **ShaderNN** | Inferencja CNN w OpenGL/GLSL | [github.com/OPPO-Mente-Lab/ShaderNN](https://github.com/nicktian007/ShaderNN) |
| **Real-ESRGAN-ncnn-vulkan** | Produkcyjna jakość SR z ncnn | [github.com/xinntao/Real-ESRGAN-ncnn-vulkan](https://github.com/xinntao/Real-ESRGAN-ncnn-vulkan) |
| **waifu2x-ncnn-vulkan** | Szybki upscaling anime/stylizowanych obrazów | [github.com/nihui/waifu2x-ncnn-vulkan](https://github.com/nihui/waifu2x-ncnn-vulkan) |

### 8.3 Modele Pre-trenowane

| Źródło | Modele | Formaty |
|--------|--------|---------|
| **OpenCV Model Zoo** | FSRCNN, ESPCN, EDSR, LapSRN | Caffe, TensorFlow |
| **ONNX Model Zoo** | Różne architektury SR | ONNX |
| **TensorFlow Hub** | EDSR, SRGAN, inne | TFLite po konwersji |

---

<div align="center">

**2024 AI-Enhanced Real-Time Renderer Guide**

Projekt opracowany dla programistów wizualizacji 3D i gier wideo.

</div>