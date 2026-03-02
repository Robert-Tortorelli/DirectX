# objRenderer - Copilot Instructions

## Project Overview

**objRenderer** is a DirectX 11 desktop application for Windows that renders 3D OBJ models with real-time lighting, texture mapping, and camera controls. It is built using C++17, Win32 API, and Direct3D 11 with HLSL shaders compiled at runtime using D3DCompile.

## Architecture

### Solution & Project Structure

```
objRenderer/
├── .github/
│   └── copilot-instructions.md       # This file
├── objRenderer/
│   ├── objRenderer.vcxproj           # Visual Studio project file
│   ├── objRenderer.vcxproj.filters   # VS project filters
│   ├── Main.cpp                      # Entry point (WinMain), message loop, window creation
│   ├── Renderer.h / Renderer.cpp     # Core Direct3D 11 rendering engine
│   ├── ObjLoader.h / ObjLoader.cpp   # Wavefront OBJ file parser
│   ├── Camera.h / Camera.cpp         # First-person camera with mouse/keyboard input
│   ├── TextureLoader.h / TextureLoader.cpp  # BMP texture file loader
│   ├── InputManager.h / InputManager.cpp    # Keyboard/mouse input state tracking
│   ├── ObjRendererUtils.h / ObjRendererUtils.cpp  # String/path utility functions
│   ├── resource.h                    # Resource identifiers
│   ├── objRenderer.rc                # Resource script (menus, dialogs, icons)
│   ├── shaders/
│   │   ├── VertexShader.hlsl         # Vertex shader (world/view/proj transforms, lighting prep)
│   │   └── PixelShader.hlsl          # Pixel shader (Phong lighting, texturing, fog)
│   ├── models/                       # Sample OBJ model files
│   └── textures/                     # Sample BMP texture files
├── objRenderer.sln                   # Visual Studio solution file
├── CMakeLists.txt                    # CMake build support
├── LICENSE
└── README.md
```

### Core Components

#### 1. Main.cpp — Application Entry Point
- Contains `WinMain` and the Win32 message pump.
- Creates the application window using Win32 `RegisterClassEx` / `CreateWindowEx`.
- Instantiates `Renderer`, `Camera`, and `InputManager`.
- Processes Win32 messages (`WM_KEYDOWN`, `WM_MOUSEMOVE`, `WM_SIZE`, `WM_COMMAND`, etc.).
- Manages the main render loop: processes input → updates camera → calls `Renderer::Render()`.
- Implements menu commands for loading OBJ files, toggling wireframe, toggling textures, and changing lighting modes.
- Uses `WM_COMMAND` with resource IDs defined in `resource.h` for menu handling.

#### 2. Renderer (Renderer.h / Renderer.cpp) — Direct3D 11 Rendering Engine
- Manages the entire Direct3D 11 pipeline lifecycle:
  - Device, device context, and swap chain creation (`D3D11CreateDeviceAndSwapChain`).
  - Render target view and depth-stencil view creation.
  - Viewport configuration.
  - Vertex and pixel shader compilation from HLSL files at runtime using `D3DCompileFromFile`.
  - Input layout creation matching the `Vertex` struct.
  - Constant buffer management for per-frame data (matrices, lighting parameters).
  - Rasterizer states (solid and wireframe).
  - Sampler state and texture (shader resource view) management.
- Key methods:
  - `Initialize(HWND, int, int)` — Full D3D11 setup.
  - `Render(const Camera&)` — Per-frame rendering (clear, set state, draw, present).
  - `LoadModel(const std::string&)` — Loads OBJ via `ObjLoader`, creates vertex/index buffers.
  - `LoadTexture(const std::string&)` — Loads BMP texture via `TextureLoader`.
  - `Resize(int, int)` — Handles window resize, recreates swap chain buffers.
  - `SetWireframe(bool)` — Toggles wireframe rendering.
  - `SetLightingMode(int)` — Switches between lighting models.
  - `Cleanup()` — Releases all COM objects.
- Uses `ComPtr<T>` (`Microsoft::WRL::ComPtr`) for COM resource management.
- Constant buffer structure (`ConstantBuffer`) contains:
  - World, View, Projection matrices (`XMMATRIX`).
  - Light direction, light color, ambient color (`XMFLOAT4`).
  - Camera position, lighting mode, texture toggle flags.
  - Fog parameters (start, end, color).

#### 3. Camera (Camera.h / Camera.cpp) — First-Person Camera
- Stores position (`XMFLOAT3`), yaw, and pitch angles.
- `Update(const InputManager&, float deltaTime)` — Reads keyboard (WASD + Q/E for vertical) and mouse delta to update position and orientation.
- `GetViewMatrix()` — Computes and returns the view matrix using `XMMatrixLookAtLH`.
- `GetPosition()` — Returns current camera world position.
- Mouse sensitivity and movement speed are configurable constants.
- Uses left-handed coordinate system consistent with DirectX conventions.

#### 4. ObjLoader (ObjLoader.h / ObjLoader.cpp) — Wavefront OBJ Parser
- Parses `.obj` files supporting:
  - Vertex positions (`v`), texture coordinates (`vt`), normals (`vn`).
  - Faces (`f`) with format `v/vt/vn` (triangulated; handles quads by splitting into two triangles).
  - Material library references (`mtllib`) and material usage (`usemtl`).
- Parses associated `.mtl` files for material properties (diffuse color, texture filenames).
- Outputs a `std::vector<Vertex>` and `std::vector<uint32_t>` (index buffer).
- The `Vertex` struct is defined as:
  ```cpp
  struct Vertex {
      XMFLOAT3 position;
      XMFLOAT3 normal;
      XMFLOAT2 texcoord;
  };
  ```
- Computes normals if not present in the OBJ file.
- Returns material/texture path information for the renderer to load.

#### 5. TextureLoader (TextureLoader.h / TextureLoader.cpp) — BMP Texture Loader
- Loads 24-bit and 32-bit uncompressed BMP files.
- Converts pixel data to `DXGI_FORMAT_R8G8B8A8_UNORM` format.
- Creates a `ID3D11Texture2D` and `ID3D11ShaderResourceView` from the loaded data.
- Handles bottom-up BMP row ordering (flips rows).
- Returns the SRV for binding to the pixel shader.

#### 6. InputManager (InputManager.h / InputManager.cpp) — Input State Tracker
- Tracks key press/release states via `bool` array indexed by virtual key codes.
- Tracks mouse position and delta movement.
- Methods: `KeyDown(UINT)`, `KeyUp(UINT)`, `IsKeyDown(UINT)`, `UpdateMousePosition(int, int)`, `GetMouseDelta()`.
- Updated from the Win32 message handler in `Main.cpp`.
- Consumed by `Camera::Update()` each frame.

#### 7. ObjRendererUtils (ObjRendererUtils.h / ObjRendererUtils.cpp) — Utilities
- String conversion helpers (`std::string` ↔ `std::wstring`).
- File path utilities (extract directory, combine paths, get file extension).
- Used by ObjLoader and TextureLoader for resolving relative paths to model/texture assets.

#### 8. HLSL Shaders (shaders/)
- **VertexShader.hlsl**: Transforms vertices by World × View × Projection matrices. Passes world-space position, normal, and texture coordinates to the pixel shader.
- **PixelShader.hlsl**: Implements Phong lighting (ambient + diffuse + specular). Supports:
  - Multiple lighting modes (selectable via constant buffer).
  - Texture sampling with a toggle to fall back to material/vertex color.
  - Distance fog (linear fog based on camera distance).
  - Uses `cbuffer` matching the C++ `ConstantBuffer` struct layout.

## Key Patterns & Conventions

### COM Resource Management
- All Direct3D COM interfaces are managed with `Microsoft::WRL::ComPtr<T>`.
- Raw `Release()` calls are avoided; rely on `ComPtr` RAII semantics.
- `Cleanup()` methods reset ComPtrs explicitly for deterministic shutdown order.

### Coordinate System
- **Left-handed** coordinate system (DirectX standard).
- Matrices use `DirectX::XM*` math library functions (`XMMatrixLookAtLH`, `XMMatrixPerspectiveFovLH`).

### Shader Compilation
- Shaders are compiled at runtime from `.hlsl` files using `D3DCompileFromFile`.
- Shader files are located in `objRenderer/shaders/` and referenced by relative path.
- The working directory must be set so that `shaders/VertexShader.hlsl` and `shaders/PixelShader.hlsl` are accessible at runtime.

### Constant Buffer Layout
- A single constant buffer is used, updated per-frame via `Map`/`Unmap` with `D3D11_MAP_WRITE_DISCARD`.
- The C++ struct and HLSL `cbuffer` must be kept in **exact layout parity** (field order, sizes, and 16-byte alignment padding with `XMMATRIX` and `XMFLOAT4`).
- When modifying the constant buffer, **always update both** `Renderer.h` (C++ struct) and both HLSL shader files.

### Error Handling
- DirectX API calls are checked with `FAILED(hr)` macro.
- Shader compilation errors are logged by reading the `ID3DBlob` error message.
- File loading failures (OBJ, BMP) return error codes or empty results and are reported via `MessageBox` or debug output.

### Naming Conventions
- **Classes**: PascalCase (`Renderer`, `ObjLoader`, `Camera`, `InputManager`).
- **Methods**: PascalCase (`Initialize`, `LoadModel`, `GetViewMatrix`).
- **Member variables**: `m_` prefix with camelCase (`m_device`, `m_swapChain`, `m_vertexBuffer`).
- **Local variables**: camelCase (`viewMatrix`, `deltaTime`).
- **Constants/Defines**: ALL_CAPS for preprocessor defines and resource IDs (`IDM_FILE_OPEN`, `ID_TOGGLE_WIREFRAME`).
- **Shaders**: PascalCase filenames (`VertexShader.hlsl`, `PixelShader.hlsl`).
- **Structs**: PascalCase (`Vertex`, `ConstantBuffer`, `Material`).

### File Organization
- Each class has a `.h` / `.cpp` pair.
- Headers use `#pragma once` for include guards.
- System/library headers are included before project headers.
- DirectX headers: `<d3d11.h>`, `<d3dcompiler.h>`, `<DirectXMath.h>`, `<wrl/client.h>`.

### Memory & Resource Lifecycle
- D3D11 resources (buffers, textures, views) are created once and reused until the model/texture changes.
- On model reload, old vertex/index buffers are released before creating new ones.
- On window resize, back buffer and depth-stencil are recreated.

## Build Instructions

### Prerequisites
- **Visual Studio 2022** (or later) with the "Desktop development with C++" workload.
- **Windows SDK** (10.0 or later) — includes DirectX 11 headers and libraries.
- **C++17** standard or later.

### Building with Visual Studio
1. Open `objRenderer.sln` in Visual Studio.
2. Select the desired configuration (`Debug` or `Release`) and platform (`x64`).
3. Build the solution (`Ctrl+Shift+B` or Build → Build Solution).
4. The output executable will be in `objRenderer/<Configuration>/` (e.g., `objRenderer/x64/Debug/objRenderer.exe`).

### Building with CMake
```bash
cd d:\Users\rober\OneDrive\source\repos\DirectX\objRenderer
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### Running
- Set the working directory to the project folder (`objRenderer/objRenderer/`) so that shader files and sample assets are found at their relative paths.
- In Visual Studio, this is configured in Project Properties → Debugging → Working Directory (set to `$(ProjectDir)`).
- The application opens a window and renders a default scene. Use File → Open to load an OBJ model.

### Linked Libraries
The project links against:
- `d3d11.lib` — Direct3D 11 runtime.
- `d3dcompiler.lib` — Runtime shader compilation.
- `dxgi.lib` — DXGI (swap chain support).
- Standard Win32 libraries (`user32.lib`, `gdi32.lib`, etc., linked automatically).

## Controls

| Input | Action |
|-------|--------|
| W/A/S/D | Move camera forward/left/backward/right |
| Q/E | Move camera down/up |
| Mouse movement | Rotate camera (yaw/pitch) |
| Menu → File → Open | Load OBJ model via file dialog |
| Menu → Toggle Wireframe | Switch between solid and wireframe rendering |
| Menu → Toggle Texture | Enable/disable texture mapping |
| Menu → Lighting Mode | Cycle through lighting modes |

## Guidelines for AI-Assisted Development

1. **Shader changes**: When modifying the constant buffer, update the C++ `ConstantBuffer` struct in `Renderer.h` AND the `cbuffer` in both `VertexShader.hlsl` and `PixelShader.hlsl` to maintain layout parity. Respect 16-byte alignment rules.
2. **COM objects**: Always use `ComPtr<T>`. Never use raw `new`/`delete` for COM objects.
3. **DirectX Math**: Use `DirectX::XMFLOAT3`, `DirectX::XMMATRIX`, etc. Load/store with `XMLoadFloat3`/`XMStoreFloat3` as DirectX math requires aligned types for SIMD operations.
4. **Error checking**: All `HRESULT`-returning functions must be checked with `FAILED(hr)` and handled appropriately.
5. **Resource creation**: D3D11 resources should follow the create-once, bind-many pattern. Avoid creating resources per-frame.
6. **Window messages**: New input or UI features should be handled in the `WndProc` in `Main.cpp` and dispatched to the appropriate manager class.
7. **New file additions**: Follow the existing pattern of `.h`/`.cpp` pairs, `#pragma once`, PascalCase class names, `m_` prefixed members.
8. **Platform**: Target Windows x64. Use Win32 API for windowing. Do not introduce cross-platform abstractions unless specifically requested.