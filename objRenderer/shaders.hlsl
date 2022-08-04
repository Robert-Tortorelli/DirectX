// Shader Functions
// Version 3.1
//
// Description
// Shader functions are small, low-level, programs that are compiled by the CPU and then run by the GPU at specific stages in the graphics pipeline. They automatically receive data from shader functions in previous stages, and return data to shader functions in successive stages, of the graphics pipeline.
// Shader functions execute when the ID3D11DeviceContext::DrawIndexed member function, or a similar member function, is called from a DirectX 11 Direct3D Win32 desktop application.
// The shader functions defined here provide the following functionality:
//   The vertex shader function uses a constant buffer to change the position of each vertex in the render target to a new position, while also mapping each vertex from 3D space to 2D space, i.e., SV_POSITION = f(constant buffer, POSITION)
//   The pixel shader function does nothing.
//   These shader functions leave the color of each pixel unchanged.
//
// A semantic is an all caps string attached to a shader function's input or output that conveys information about the intended use of a parameter on the GPU. Semantics are required on all variables passed between shader stages.
//   A semantic specified after a shader function's parameter list is used to convey information about the intended use of the shader function's return value, e.g., In 'float4 PShader(VOut input) : SV_TARGET' the SV_TARGET semantic applies to the float4 return value.
//   For a shader function to return multiple variables, it returns a struct containing multiple members, just as a C program would. Each structure member must specify a semantic.
//     In such a case the shader function does not specify its own semantic, i.e., SV_POSITION is removed (and the type float4 is replaced by the struct name) from 'float4 VShader(float4 position : POSITION, float4 color : COLOR) : SV_POSITION'
// Semantics used in the shader functions defined here are described below. The relationship of these semantics is as follows:
//   POSITION of each pixel in 3D space & COLOR of each pixel -> Vertex shader -> SV_POSITION of each pixel in 2D space & COLOR of each pixel -> Pixel shader -> SV_TARGET (final color) of each pixel of the render target
//
// The vertex shader function executes once for each vertex in the scene. For a single triangle, it is called by the graphics driver three times, once per vertex.
// The pixel shader function executes once for each pixel in the render target. Based on the output clip positions, the rasterizer identifies the pixels covered by each triangle, perspectively interpolates the vertex attributes at those pixels, and then calls the pixel shader for each pixel.
//
// Authorship
// This program, except for the objReader function that parses the Wavefront.obj file, is based on "DirectX 11 Win32 Desktop: Direct3D: Moving to 3D: Lesson 3: Simple Modeling" and earlier lessons by Chris Hanson (https://DirectXTutorial.com).
// All defects in this program are solely the responsibility of Robert John Tortorelli.

// Declarations: Start

// All variables and functions coded in HLSL (.hlsl files) are stored in GPU memory.
// All variables and functions coded in C++ (.cpp files) are stored in CPU memory.

// Vector variables store between one and four    scalar values and are declared by placing a   number                     at the end of the type, 
// e.g., a vector variable named "color"  containing four float scaler values can be declared and defined using 
// float4 color = {1.0f, 2.0f, 3.0f, 4.0f};
// OR
// float4 color = float4(1.0f, 2.0f, 3.0f, 4.0f);
//
// Matrix variables store between one and sixteen scaler values and are declared by placing the number of rows and columns at the end of the type, 
// e.g., a matrix variable named "matrix" containing four float scaler values can be declared and defined using 
// float2x2 matrix = { 1.0f, 2.0f, // row 1
//                     3.0f, 4.0f  // row 2
//                   };
// OR
// float2x2 matrix = float2x2(1.0f, 2.0f,  // row 1
//                            3.0f, 4.0f); // row 2

// Declare the constant buffer structure.
cbuffer ConstantBuffer
{
	float4x4 matFinal;                                      // A float4x4 is 16 (4x4) floats (each 4 bytes) for a total of 64 bytes.
}

// Declare the Vertex shader output struct. It is also used as the input struct for the Pixel shader.
// The program using these shaders specifies a vertex buffer containing both position and color values. Therefore multiple variables, position and color, are returned by the vertex shader function.
//
// For the vertex shader function to return these multiple variables, it returns a struct containing multiple members, just as a C program would.
//
// Semantics:
// SV_POSITION: Vertex position in screen space (2D space: Between 1 and -1 on the X and Y axes).                                  Vertex shader -> Pixel shader
// COLOR:       Pixel color.                                                                                    -> Vertex shader | Vertex shader -> Pixel shader
struct VOut
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

// Declarations: End

// VShader function: Definition
// This function is the vertex shader function.
// This shader is executed for each vertex in the scene.
// This shader operates on vertex buffer elements provided to it by the calling program and at a minimum returns a 4-component position vector that will be rasterized into a pixel position.
// Optionally, this shader can output texture coordinates, vertex color, vertex lighting, fog factors, and other characteristics of a single vertex.
//
// The number of times the vertex shader has been executed can be queried from the CPU using the VSInvocations pipeline statistic.
//
// When passing multiple variables between shader functions, they must be passed in the same order, e.g., position first; color second.
//
// The vertex shader function returns multiple variables, position and color.                  Therefore it does return a struct, but it does not specify its own semantic, i.e., SV_POSITION.
//
// Semantics:
// COLOR:       Pixel color.                                                                                    -> Vertex shader | Vertex shader -> Pixel shader
// POSITION:    Vertex position in 3D space.                                                                    -> Vertex shader
// SV_POSITION: Vertex position in screen space (2D space: Between 1 and -1 on the X and Y axes).                                  Vertex shader -> Pixel shader
VOut VShader(float4 position : POSITION, float4 color : COLOR)
{
    VOut output;

    output.position = mul(matFinal, position);              // output.position is SV_POSITION = f(constant buffer, POSITION) (Position of pixel in 2D space is a function of the constant buffer and the position of the pixel in 3D space)
    output.color = color;                                   // output.color    is COLOR       = f(COLOR) (Pixel color (unchanged in this shader function))

    return output;
}

// PShader function: Definition
// This function is the pixel shader function.
// This shader is also known as a fragment shader.
// This shader is executed for each pixel (fragment) in the render target.
// This shader receives rasterized coordinates from previous shader stages (in the simplest pipelines, this would be the vertex shader) and at a minimum returns a color (or other 4-component value) for that pixel position, which is then written into a render target.
// Optionally, this shader can output the color, brightness, contrast, and other characteristics of a single pixel.
//
// The number of times the pixel shader has been executed can be queried from the CPU using the PSInvocations pipeline statistic.
//
// When passing multiple variables between shader functions, they must be passed in the same order, e.g., position first; color second.
//
// The pixel shader function returns a single variable, position or color, in this case color. Therefore it does not return a struct, and it does specify its own semantic, i.e., SV_TARGET.
//
// Semantics:
// SV_POSITION: Vertex position in screen space (2D space: Between 1 and -1 on the X and Y axes).                                  Vertex shader -> Pixel shader
// COLOR:       Pixel color.                                                                                    -> Vertex shader | Vertex shader -> Pixel shader
// SV_TARGET:   Final color of the pixel of the render target.                                    Pixel shader  ->
//
// The following syntax was provided by DirectXTutorial.com:
// float4 PShader(float4 position : SV_POSITION, float4 color : COLOR) : SV_TARGET
// {
//     return color;                                           // color           is COLOR       = Pixel color (unchanged in this shader function)
// }
// The following alternative syntax is also valid: It was suggested by GitHub Copilot.
// This syntax explicitly shows that the return value of the vertex shader function is being passed to the pixel shader function.
float4 PShader(VOut input) : SV_TARGET
{
    return input.color;                                  // input.color     is COLOR       = Pixel color (unchanged in this shader function)
}