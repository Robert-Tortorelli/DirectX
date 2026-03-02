# objRenderer - Copilot Instructions

## Project Overview

**objRenderer** is a DirectX 11 desktop application for Windows that renders 3D OBJ models with real-time lighting, texture mapping, and camera controls. It is built using C++23, Win32 API, and Direct3D 11 with HLSL shaders compiled at runtime using D3DCompile.

## Architecture

### Solution & Project Structure

```
objRenderer/
├── .github/
│   └── copilot-instructions.md       # This file
├── objRenderer/
│   ├── objRenderer.vcxproj           # Visual Studio project file
│   ├── objRenderer.vcxproj.filters   # VS project filters
│   ├── objRenderer.cpp               # Entry point (WinMain), message loop, window creation, Core Direct3D 11 rendering engine, First-person camera with keyboard input
│   ├── objFileProcessor              # Wavefront OBJ file parser
│   ├── resource.h                    # Resource identifiers
│   ├── resource.rc                   # Resource script (menus, dialogs, icons)
│   ├── shaders.hlsl                  # vertex shader, pixel shader
├── .\DirectX.sln                     # Visual Studio solution file
├── .\LICENSE
└── .\README.md
```

### Core Components

#### 1. objRenderer.cpp — Application Entry Point, Direct3D 11 Rendering Engine
- Contains `WinMain` and the Win32 message pump.
- Creates the application window using Win32 `RegisterClassEx` / `CreateWindowEx`.
- Processes Win32 messages.
- Manages the main render loop: processes input → updates camera → calls RenderFrame.
- Uses `WM_COMMAND` with resource IDs defined in `resource.h` for menu handling.
- Manages the entire Direct3D 11 pipeline lifecycle:
  - Device, device context, and swap chain creation (`D3D11CreateDeviceAndSwapChain`).
  - Render target view and depth-stencil view creation.
  - Viewport configuration.
  - Vertex and pixel shader compilation from HLSL files at runtime using `D3DCompileFromFile`.
  - Input layout creation matching the `VERTEX` struct.
  - Constant buffer management for per-frame data (matrices, lighting parameters).
  - Texture (shader resource view) management.
- Uses `ComPtr<T>` (`Microsoft::WRL::ComPtr`) for COM resource management.
- Constant buffer structure (`ConstantBuffer`) contains:
  - World, View, Projection matrices (`XMMATRIX`).
  - Light direction, light color, ambient color (`XMFLOAT4`).
  - Camera position, lighting mode.
- Uses left-handed coordinate system consistent with DirectX conventions.

#### 2. objFileProcessor.cpp — Wavefront OBJ Parser
- Parses `.obj` files supporting:
  - Vertex positions (`v`), texture coordinates (`vt`), normals (`vn`).
  - Faces (`f`) with format `v/vt/vn` (triangulated)
- Outputs a `std::vector<VERTEX>` (vertex buffer) and `std::vector<DWORD>` (index buffer).
- The `VERTEX` struct is defined as:
  ```cpp
  struct VERTEX {												       // Vertex attributes.
	  DirectX::XMFLOAT3 GeometricVertex;				 // Geometric vertex attribute:			.x, .y, .z	("v " element in the Wavefront .obj file)
	  DirectX::XMFLOAT2 VertexTextureCoordinate; // Vertex texture coordinate attribute:	.x, .y		("vt" element in the Wavefront .obj file)
	  DirectX::XMFLOAT3 VertexNormalVector;			 // Vertex normal vector attribute:		.x, .y, .z	("vn" element in the Wavefront .obj file)
  };
  ```
- Loads a Windows Imaging Component (WIC)-supported bitmap file from disk, creates a Direct3D 11 resource from it, and a Direct3D 11 shader resource view.
- Converts pixel data to `DXGI_FORMAT_R8G8B8A8_UNORM` format.

#### 3. shaders.hlsl — HLSL Vertex and Pixel Shaders

## Key Patterns & Conventions

### COM Resource Management
- All Direct3D COM interfaces are managed with `Microsoft::WRL::ComPtr<T>`.
- Raw `Release()` calls are avoided where possible; relying instead on `ComPtr`'s automatic reference counting.

### Coordinate System
- **Left-handed** coordinate system (DirectX standard).
- Matrices use `DirectX::XM*` math library functions (`XMMatrixLookAtLH`, `XMMatrixPerspectiveFovLH`).

### Shader Compilation
- Shaders are compiled at runtime from `.hlsl` files using `D3DCompileFromFile`.
- Shader files are located in `objRenderer\`.

### Constant Buffer Layout
- A single constant buffer is used, updated per-frame via `Map`/`Unmap` with `D3D11_MAP_WRITE_DISCARD`.
- The C++ struct and HLSL `cbuffer` must be kept in **exact layout parity** (field order, sizes, and 16-byte alignment padding with `XMMATRIX` and `XMFLOAT4`).
- When modifying the constant buffer, **always update both** `objRenderer.h` (C++ struct) and 'shader.hlsl' (HLSL shader file).

### Error Handling
- File loading failures (.obj files) are reported via function return values.

<!--
Correct and uncomment the following lines. (RJT)
### Naming Conventions
- **Classes**: PascalCase (`Renderer`, `ObjLoader`, `Camera`, `InputManager`).
- **Methods**: PascalCase (`Initialize`, `LoadModel`, `GetViewMatrix`).
- **Member variables**: `m_` prefix with camelCase (`m_device`, `m_swapChain`, `m_vertexBuffer`).
- **Local variables**: camelCase (`viewMatrix`, `deltaTime`).
- **Constants/Defines**: ALL_CAPS for preprocessor defines and resource IDs (`IDM_FILE_OPEN`, `ID_TOGGLE_WIREFRAME`).
- **Shaders**: PascalCase filenames (`VertexShader.hlsl`, `PixelShader.hlsl`).
- **Structs**: PascalCase (`Vertex`, `ConstantBuffer`, `Material`).
-->

### File Organization
- Headers use `#pragma once` for include guards.
- System/library headers are included after project headers.
- DirectX headers: `<d3d11.h>`, `directxmath.h`, `<d3dcompiler.h>`, `wictextureloader.h`, `d2d1.h`, `dwrite.h`, `<wrl/client.h>`.

<!--
Correct and uncomment the following lines. (RJT)
None of these statements are correct. (RJT)
### Memory & Resource Lifecycle
- D3D11 resources (buffers, textures, views) are created once and reused until the model/texture changes.
- On model reload, old vertex/index buffers are released before creating new ones.
- On window resize, back buffer and depth-stencil are recreated.
-->

## Build Instructions

### Prerequisites
- **Visual Studio 2022** (or later) with the "Game Development with C++" workload.
- **Windows SDK** (11.0 or later) — includes DirectX 11 headers and libraries.
- **C++23** standard or later.

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
- The application opens a window and renders a default scene.

### Linked Libraries
The project links against:
- `d3d11.lib` — Direct3D 11 runtime.
- `d3dcompiler.lib` — Runtime shader compilation.
<!-- - `dxgi.lib` — DXGI (swap chain support). Not found in .cpp files (RJT) -->
- Standard Win32 libraries (`user32.lib`, `gdi32.lib`, etc., linked automatically).

## Controls

| Input | Action |
|-------|--------|
| I | Move the camera +z |
| K | Move the camera -z |
| W | Move the object +x |
| S | Move the object -x |
| A | Move the object +y |
| D | Move the object -y |
| Menu → File → Exit | Terminate the application |
| Menu → File → Enter Text | Prompt the user for input and display it |
| Menu → Help → About | Display diagnostic information about the application |

<!--
Correct and uncomment the following lines. (RJT)
Only 8 is correct. 1 - 7 are incorrect. (RJT)
## Guidelines for AI-Assisted Development
1. **Shader changes**: When modifying the constant buffer, update the C++ `ConstantBuffer` struct in `Renderer.h` AND the `cbuffer` in both `VertexShader.hlsl` and `PixelShader.hlsl` to maintain layout parity. Respect 16-byte alignment rules.
2. **COM objects**: Always use `ComPtr<T>`. Never use raw `new`/`delete` for COM objects.
3. **DirectX Math**: Use `DirectX::XMFLOAT3`, `DirectX::XMMATRIX`, etc. Load/store with `XMLoadFloat3`/`XMStoreFloat3` as DirectX math requires aligned types for SIMD operations.
4. **Error checking**: All `HRESULT`-returning functions must be checked with `FAILED(hr)` and handled appropriately.
5. **Resource creation**: D3D11 resources should follow the create-once, bind-many pattern. Avoid creating resources per-frame.
6. **Window messages**: New input or UI features should be handled in the `WndProc` in `Main.cpp` and dispatched to the appropriate manager class.
7. **New file additions**: Follow the existing pattern of `.h`/`.cpp` pairs, `#pragma once`, PascalCase class names, `m_` prefixed members.
8. **Platform**: Target Windows x64. Use Win32 API for windowing. Do not introduce cross-platform abstractions unless specifically requested.
-->