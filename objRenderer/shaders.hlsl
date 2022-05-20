// DirectX 11  Direct3D Win32 Desktop
cbuffer ConstantBuffer
{
	// The name of the constant buffer variable (a HLSL structure) declared by the HLSL statements does not have to be match the name of the analogous final transform matrix variable (a C++ structure) declared by the C++ statements. However, they have been named the same for clarity.
	// The order and size of these structure's members must match in both the HLSL statements and the C++ statements.
	float4x4 matFinal;                                      // A float4x4 is 16 (4x4) floats (each 4 bytes) for a total of 64 bytes.
}

struct VOut
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

VOut VShader(float4 position : POSITION, float4 color : COLOR)
{
    VOut output;

    output.position = mul(matFinal, position);
    output.color = color;

    return output;
}


float4 PShader(float4 position : SV_POSITION, float4 color : COLOR) : SV_TARGET
{
    return color;
}
