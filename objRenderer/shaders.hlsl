// Shader Functions
// Version 3.1
//
// Description
// Shader functions are small, low-level, programs that are compiled by the CPU and then run by the GPU at specific stages in the graphics pipeline. They automatically receive data from shader functions in previous stages, and return data to shader functions in successive stages, of the graphics pipeline.
// Shader functions execute when the ID3D11DeviceContext::DrawIndexed member function, or a similar member function, is called from a DirectX application.
//
// A semantic is an all caps string attached to a shader function's input or output that conveys information about the intended use of a parameter on the GPU. Semantics are required on all variables passed between shader stages.
//   A semantic specified after a shader function's parameter list is used to convey information about the intended use of the shader function's return value, e.g., 
//     In "float4 VShader(float4 position3D : POSITION, float4 normal : NORMAL, float2 texcoord : TEXCOORD) : SV_POSITION" the semantic SV_POSITION applies to the simple float4 return value.
//   For a shader function to return multiple variables, it returns a struct containing multiple members, just as in a C++ program. Each structure member must specify its associated semantic.
//     In such a case the shader function no longer specifies its own semantic, i.e., SV_POSITION is removed, and the simple return type is replaced by the struct name, e.g., 
//     "float4 VShader(float4 position3D : POSITION, float4 normal : NORMAL, float2 texcoord : TEXCOORD) : SV_POSITION"
//      becomes:
//     "VOut   VShader(float4 position3D : POSITION, float4 normal : NORMAL, float2 texcoord : TEXCOORD)"
// Semantics used in the shader functions defined here are described below.
//
// Authorship
// This program, except for the objReader function that parses the Wavefront.obj file, is based on "DirectX 11 Win32 Desktop: Direct3D: Moving to 3D: Lesson 3: Simple Modeling" and earlier lessons by Chris Hanson (http://DirectXTutorial.com).
// All defects in this program are solely the responsibility of Robert John Tortorelli.

// Declarations: Start

// All variables and functions coded in HLSL (.hlsl files) are processed in GPU memory.
//   In HLSL, the value of a local variable does not persist between shader function calls, unless it is declared type static, just as in a C++ program.
// All variables and functions coded in C++  (.cpp  files) are processed in CPU memory.

// Vector variables store between one and four scalar values and are declared by placing a number at the end of the type specification, 
// e.g., a vector variable named "color"  containing four float scaler values can be declared and defined using 
// float4 color = (1.0f, 2.0f, 3.0f, 4.0f);
// color = (1.0f, 2.0f, 3.0f, 4.0f); // This definition is valid after "color" is declared.
//
// Matrix variables store between one and sixteen scaler values and are declared by placing the number of rows and columns at the end of the type specification, 
// e.g., a matrix variable named "matrix" containing four float scaler values can be declared and defined using 
// float2x2 matrix = (1.0f, 2.0f,	 // row 1
//                    3.0f, 4.0f);	 // row 2
// matrix = (1.0f, 2.0f,
//           3.0f, 4.0f);			 // This definition is valid after "matrix" is declared.
//
// HLSL variables may be larger than the corresponding C++ variables used to indirectly (by way of the vertex buffer) assign them data.
// For example, a HLSL float4 variable, such as "position3D" in "float4 position3D : POSITION;" is assigned values from the vertex buffer, that were originally copied to the vertex buffer (using C++), from the smaller C++ variable OurVertices.
// OurVertices only specifies three floats for geometric vertex position, and they are assigned to the first three floats of "position3D", leaving the last float of "position3D" undefined.

// Declare the constant buffer.
// Note this is defined using the type cbuffer, not the type struct.
// See the C++ constant buffer structure declaration for an explanation of these constant buffer members.
cbuffer ConstantBuffer
{
	float4x4 matFinal;
	float4x4 matRotate;
	float4 LightVector;
	float4 LightColor;
	float4 AmbientColor;
}

// Declare the struct of return values output by the vertex shader function. It is sometimes also used as the input struct for the pixel shader function.
//
// For a shader function to return multiple variables, it returns a struct containing multiple members, just as in a C++ program. Each structure member must specify its associated semantic.
// The order of these struct members determines the order of the input parameters passed to the pixel shader function (see the definitions of the shader functions).
//
// Semantics:
// COLOR:       Pixel color.                                                                                    -> Vertex shader | Vertex shader -> Pixel shader
// TEXCOORD:	Texture Coordinates.																			-> Vertex shader | Vertex shader -> Pixel shader
// SV_POSITION: Vertex position in screen space (2D space: Between 1 and -1 on the X and Y axes).                                  Vertex shader -> Pixel shader
struct VOut
{
	float4 color : COLOR;
	float2 texcoord : TEXCOORD;
	float4 position2D : SV_POSITION;
};

// Declare the texture object.
Texture2D Texture;											// A 2D texture object.
// Declare the sampler type, a set of properties that define how to sample the texture object.
SamplerState ss;											// A SamplerState sampler type.

// Declarations: End

// VShader function: Definition
// This function is the vertex shader function.
// A vertex shader function executes once for each vertex, e.g., for a single triangle, it is called by the graphics driver three times, once per vertex.
// A vertex shader function always operates on a single input vertex and produces a single output vertex.
// A vertex shader function may return multiple variables. In such case it returns a struct, e.g., structure VOut with its member variables, and consequently it specifies the associated semantics in that struct's definition.
// A vertex shader function must return the geometric vertex's 2D position. Vertex color, vertex lighting, vertex texture coordinates, fog factors, and other characteristics of a single vertex, are optional.
//
// The number of times a vertex shader function has been executed can be queried from the CPU using the VSInvocations pipeline statistic.
// 
// The vertex shader function's input parameters may be specified in any order. However, when passing multiple variables between shader functions (e.g., return values output by the vertex shader function -> input parameters of the pixel shader function), they must be passed in the same order.
//   The C++ input element description array of structures is used to define the input-layout object that describes the input elements of the C++ VERTEX structure definition. These input element descriptions agree in number and type with the vertex shader function's input parameters.
//   Because each C++ input element description specifies the HLSL semantic name associated with each input element of the C++ VERTEX structure, the vertex shader function's input parameters, which include the HLSL semantic name, may be specified in any order.
//
// Semantics:
// POSITION:    Vertex position in 3D space.                                                                    -> Vertex shader
// NORMAL:      Normal vector.                                                                                  -> Vertex shader
// TEXCOORD:	Texture Coordinates.																			-> Vertex shader | Vertex shader -> Pixel shader
VOut VShader(float4 position3D : POSITION, float4 normal : NORMAL, float2 texcoord : TEXCOORD)
{
	VOut output;

	// Calculate the geometric vertex's 2D position (screen position) from its 3D position.
	output.position2D = mul(matFinal, position3D);						// output.position2D with semantic SV_POSITION = f(constant buffer's matFinal, VShader parameter position3D with semantic POSITION)

	// Calculate changes in color based on the level of light.
	// normalVector is necessary because VShader parameter "normal" cannot be modified.
	float4 normalVector = normalize(mul(matRotate, normal));			// Rotate the vertex normal vector to match the rotation of the object.																   The "normalize" intrinsic scales the calculated value of a vector to make its length equal to 1.0.
	float diffusebrightness = saturate(dot(normalVector, LightVector)); // Calculate the amount of light at the vertex (diffuse brightness) as the dot product of the vertex normal vector and the light vector. The "saturate" intrinsic clamps the calculated values between 0 and 1.
	output.color = AmbientColor + (LightColor * diffusebrightness);		// output.color with semantic COLOR = f(constant buffer's ambient light's color, constant buffer's light's color, calculated diffuse brightness)
	output.texcoord = texcoord;											// Set the texture coordinates, unmodified.

	return output;
}

// PShader function: Definition
// This function is the pixel shader function.
// A pixel shader function is also known as a fragment shader function. The term pixel shader is commonly used in the context of DirectX, while the term fragment shader is more commonly used in the context of OpenGL.
// A pixel shader function executes once for each pixel. Based on the output clip positions, the rasterizer identifies the pixels covered by each triangle, perspectively interpolates the vertex attributes at those pixels, and then calls the pixel shader function for each pixel.
// A pixel shader function's inputs are interpolated from the vertex attributes of the primitive being rasterized, based on the interpolation mode declared.
// A pixel shader function returns a single variable, color. Therefore it does not return a struct, and consequently it specifies the associated semantic in the pixel shader function's definition, i.e., SV_TARGET.
//
// The number of times a pixel shader function has been executed can be queried from the CPU using the PSInvocations pipeline statistic.
//
// When passing multiple variables between shader functions (e.g., return values output by the vertex shader function -> input parameters of the pixel shader function), they must be passed in the same order, e.g., COLOR first; TEXCOORD second; SV_POSITION third as specified by the order of members in the structure VOut returned by the vertex shader function.
// The input parameters of the pixel shader function must be a continuous subset of the return values output by the vertex shader function, where this subset starts with the first return value of the vertex shader function. SV_POSITION, which is not needed by the pixel shader function, is therefore placed as the third member of the structure VOut so it need not be passed as input to the pixel shader function.
// It is possible for the pixel shader function to have fewer input parameters than the return values of the vertex shader function, as the pixel shader function only needs to know the interpolated data for each pixel and not the full set of data for the entire model.
//   For example:
//   If Vertex shader output is: a, b, c then:
//   Pixel shader input: a, b, c (valid)
//   Pixel shader input: a, b    (valid)
//   Pixel shader input: a       (valid)
//   Pixel shader input: b, c    (invalid)
//
// Semantics:
// COLOR:       Pixel color.                                                                                    -> Vertex shader | Vertex shader -> Pixel shader
// TEXCOORD:	Texture Coordinates.																			-> Vertex shader | Vertex shader -> Pixel shader
// SV_TARGET:   Final color of the pixel of the render target.                                    Pixel shader  -> (Output-Merger Stage)
float4 PShader(float4 color : COLOR, float2 texcoord : TEXCOORD) : SV_TARGET
{
	// Return color with semantic SV_TARGET = f(PShader parameter color with semantic COLOR, sampled texture (= f(PShader parameter "texcoord" with semantic TEXCOORD)))
	// texture-Object.Sample member function:
	//   Sample a texture object.
	//   texture-Object.Sample( sampler_state S, float Location [, int Offset] );
	return color * Texture.Sample(ss,						// The sampler state (sampler type).
								  texcoord);				// The texture coordinates.
}