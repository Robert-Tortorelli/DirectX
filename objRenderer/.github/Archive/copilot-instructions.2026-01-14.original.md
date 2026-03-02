# Copilot Instructions for objRenderer

## Project Overview

objRenderer is a DirectX 11 Win32 Desktop application (Version 3.3) that parses Wavefront `.obj` files and renders 3D objects with lighting and texturing. It integrates Direct3D 11, Direct2D, and DirectWrite APIs.

## Architecture

### Core Components

- **[objRenderer.cpp](../objRenderer.cpp)** - Main entry point (`WinMain`), window management, DirectX initialization, and render loop
- **[objReaderParser.cpp](../objReaderParser.cpp)** - Wavefront `.obj` file parser (`objReader` + `objParser` functions)
- **[objRenderer.h](../objRenderer.h)** - Shared declarations: `VERTEX` struct (vertex attributes), `OBJECT` struct (per-object data including GPU buffers)
- **[shaders.hlsl](../shaders.hlsl)** - HLSL vertex/pixel shaders with Lambertian lighting model

### Data Flow

1. `objReader()` scans current directory for `.obj` files
2. `objParser()` extracts vertices (`v`), texture coords (`vt`), normals (`vn`), and faces (`f`)
3. Data stored in `OurObjects` vector (each `OBJECT` has `OurVertices`, `OurIndices`, GPU buffers)
4. `InitGraphics()` uploads to GPU via vertex/index buffers
5. `RenderFrame()` transforms and renders each object per frame

### Coordinate System Conversions

**Critical**: Wavefront `.obj` uses different conventions than DirectX:
- Z-coordinates: Inverted (`z * -1.0f`)
- Texture V-coordinate: Inverted (`1.0f - v`)
- Winding order: Counter-clockwise → Clockwise (swap indices 1 and 2)

## Build & Run

- **IDE**: Visual Studio 2022 (C++ Desktop Development workload)
- **Dependencies**: DirectX 11 SDK, DirectXTK (`wictextureloader.h`)
- **Build**: Open `objRenderer.vcxproj`, build Debug/Release x64
- **Run**: Place `.obj` files + `Wood.png` texture in same directory as executable

## Key Patterns

### ComPtr Smart Pointers

All DirectX COM objects use `Microsoft::WRL::ComPtr<>`:
```cpp
ComPtr<ID3D11Device> dev;
dev->CreateBuffer(...);           // Use -> for method calls
dev.GetAddressOf()                // Use for **ppOut parameters
dev.Get()                         // Use for *pIn parameters
```

### Graphics Pipeline Flow

Initialization order in `InitD3D()`:
1. Device/SwapChain creation → 2. Depth buffer → 3. Render target → 4. Viewport → 5. Pipeline (`InitPipeline`) → 6. Graphics data (`InitGraphics`)

### Constant Buffer Pattern

CPU-side struct in `OBJECT::ConstantBuffer` mirrors HLSL `cbuffer`:
```cpp
// C++ (objRenderer.h)              // HLSL (shaders.hlsl)
struct { XMMATRIX matFinal; ... }   cbuffer ConstantBuffer { float4x4 matFinal; ... }
```
Update via `UpdateSubresource()` before each draw call.

### Return Codes

| RC | Source | Meaning |
|----|--------|---------|
| 0 | All | Success |
| 1 | `objParser` | Cannot open `.obj` file |
| 2 | `objParser` | Missing required vertex attributes (v, vt, or vn) |
| 3 | `objReader` | No `.obj` files found |

## Wavefront .obj Requirements

Files must contain ALL of:
- Geometric vertices: `v x y z`
- Texture coordinates: `vt u v`
- Vertex normals: `vn x y z`
- Triangulated faces: `f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3`

**Not supported**: Multiple objects per file, line continuations (`\`), spaces around `/`

## Runtime Controls

- **W/S**: Move object +/- X axis
- **A/D**: Move object +/- Y axis
- **I/K**: Move camera +/- Z axis
- **Menu**: File > Exit, File > Enter Text, Help > About (displays diagnostics)

## Common Modifications

### Adding vertex attributes
1. Update `VERTEX` struct in [objRenderer.h](../objRenderer.h)
2. Update input element description array in `InitPipeline()`
3. Update HLSL `VShader` parameters in [shaders.hlsl](../shaders.hlsl)

### Adding new objects
Place `.obj` files in `Wavefront .obj file repository/` or executable directory. Parser auto-discovers all `.obj` files.
