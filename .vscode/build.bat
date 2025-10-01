:: filepath: .vscode/build.bat
@echo off

:: Set up the Visual Studio 64-bit environment.
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

:: Compile the resource file resource.rc to resource.res (Visual Studio compiles to resource.aps).
rc.exe /fo "D:\Users\rober\OneDrive\source\repos\DirectX\objRenderer\resource.res" "D:\Users\rober\OneDrive\source\repos\DirectX\objRenderer\resource.rc"

:: Compile the project (Debug build).
cl.exe /EHsc /Zi /std:c++17 /DUNICODE /D_UNICODE /D_DEBUG /MDd /Fe:objRenderer.exe ^
D:\Users\rober\OneDrive\source\repos\DirectX\objRenderer\objRenderer.cpp ^
D:\Users\rober\OneDrive\source\repos\DirectX\objRenderer\objReader.cpp ^
D:\Users\rober\OneDrive\source\repos\DirectX\objRenderer\objReaderEnum.cpp ^
/I. ^
/I"D:\Users\rober\OneDrive\source\repos\DirectX\packages\directxtk_desktop_2019.2025.3.21.2\include" ^
/link /MACHINE:X64 ^
/LIBPATH:"D:\Users\rober\OneDrive\source\repos\DirectX\packages\directxtk_desktop_2019.2025.3.21.2\native\lib\x64\Debug" ^
D:\Users\rober\OneDrive\source\repos\DirectX\objRenderer\resource.res ^
d3d11.lib d3dcompiler.lib dxgi.lib d2d1.lib dwrite.lib user32.lib gdi32.lib comdlg32.lib DirectXTK.lib