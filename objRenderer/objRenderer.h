// objRenderer/objRenderer.h
// Version 3.3
//
// Description:
// This header file contains all declarations shared by multiple source files in this project.
// Header files should not contain "using declarations" (such as using std::string;) or  "using directives" (such as using namespace std;).
//
// Authorship:
// Robert John Tortorelli

// Pragma Directives.
// Specify that the compiler include this header file only once when compiling source code files.
#pragma once												// Automatically included by Visual Studio 2022 when creating a header file.

// Header Files.
// Windows Runtime Template Library (WRL) Header File.
#include <wrl.h>											// Needed by the Microsoft::WRL::ComPtr smart pointer.
// Standard Encapsulated Data and Functions for Manipulating String Data Header File.
#include <string>											// String class member functions stof, to_string, etc.
// Vector Container Class Header File.
#include <vector>											// Vector class member functions push_back, pop_back, etc.
// Windows API Header File.
#include <windows.h>										// The Windows API (Win32 API) header file enables you to create 32-bit and 64-bit programs. It includes declarations for both Unicode and ANSI versions of the API. For more information, see Unicode in the Windows API.
// Direct3D 11 Header File.
#include <d3d11.h>											// This header is used by Direct3D 11 Graphics.
// DirectXMath Header File.
#include <directxmath.h>                                    // The DirectXMath API provides SIMD-friendly C++ types and functions for common linear algebra and graphics math operations common to DirectX programs.

//***
// Structure Declarations.
// Structures declarations appear before variable declarations that reference them.
//***

// VERTEX 'named structure' data type.
// The set of vertex attributes of one of the three vertices of a triangle.
// The input element description structure is used to define the input-layout object that describes the VERTEX structure.
// If the VERTEX structure is changed then the input element description structure (defined in objRenderer.cpp) must be changed accordingly.
struct VERTEX {												// Vertex attributes.
	DirectX::XMFLOAT3 GeometricVertex;						// Geometric vertex attribute:			.x, .y, .z	("v " element in the Wavefront .obj file)
	DirectX::XMFLOAT2 VertexTextureCoordinate;				// Vertex texture coordinate attribute:	.x, .y		("vt" element in the Wavefront .obj file)
	DirectX::XMFLOAT3 VertexNormalVector;					// Vertex normal vector attribute:		.x, .y, .z	("vn" element in the Wavefront .obj file)
};

// SUBMESH and OBJECT 'named structure' data types.
//
// SUBMESH 'named structure' data type.
// The set of attributes of one submesh.
// One submesh within one object uses one material, e.g., the same texture image.
//
// OBJECT 'named structure' data type.
// The set of attributes of one object.
// A 3D object is comprised of one or more submeshes.
//
// OurVertices and OurIndices.
// OurVertices (used to initialize the DirectX vertex buffer), a variable containing values formatted for DirectX, is the array of unique sets of vertex attributes of a single 3D object.
// OurIndices  (used to initialize the DirectX index  buffer), a variable containing values formatted for DirectX, is the array of non-unique indices of OurVertices, each pointing to an associated unique set of vertex attributes in OurVertices.
// One unique set of vertex attributes in OurVertices, OurVertices(y), may be referenced more than once by duplicate (non-unique) indices in OurIndices:
// e.g., OurIndices(x1) and OurIndices(x2) = y (y is the duplicate (non-unique) index in OurIndices), both pointing to the same unique set of vertex attributes of OurVertices(y), e.g., OurIndices(x1) = y so OurVertices(OurIndices(x1)) = OurVertices(y).
//
// VertexAttributeSetsTotal.
// A cube's 6 sides				 are each comprised of 4 corners (vertices) so there are 4 unique sets of vertex attributes per side   for a total of 6 x 4 = 24 unique sets of vertex attributes.
// A cube's 8 corners (vertices) are each part		of 3 sides				so there are 3 unique sets of vertex attributes per corner for a total of 8 x 3 = 24 unique sets of vertex attributes.
// By contrast, in a program where (unlike this program) the only vertex attribute is the geometric vertex, there are only 8 unique sets of vertex attributes, i.e., each set contains only a single geometric vertex.
//
// IndicesTotal.
// A cube's 6 sides are comprised of 2 triangle primitives per side, for a total of 6 x 2 = 12 triangle primitives, each triangle primitive comprised of 3 vertices, for a total of 12 x 3 = 36 non-unique indices of OurVertices.
//
// ConstantBuffer.
// Declare the C++ constant buffer structure used to assign values to the HLSL constant buffer structure.
// This structure represents a constant buffer used in the graphics rendering pipeline.
// It contains information that is passed to the vertex shader stage of the pipeline and can be used to transform geometric vertices and calculate lighting effects on them.
//
// The matFinal member  is the 4x4 final transformation matrix that represents the combined world, view, and projection transformations that are applied to the geometric vertices of the geometry being rendered.
//
// The matRotate member is the 4x4 final rotation matrix       that represents a rotation transformation                                that is  applied to the geometric vertices of the geometry being rendered.
// It is a component of the world transformation, and therefore of the matFinal matrix.
// Vertex normal vectors at the geometric vertices, like the geometric vertices comprising the object, also need to be transformed by the rotation matrix to correctly calculate lighting effects.
//
// The LightVector member is a 4D vector that represents the direction of the light source in 3D space.
// This vector can be represented by any nonzero vector and the light will shine in that direction.
//
// The LightColor member is a 4D vector that represents the color and brightness of the light source.
// Any color closer to white is brighter than any color closer to black.
//
// The AmbientColor member is a 4D vector that represents the color and brightness of the ambient light in the scene.
// Ambient light is a type of light that illuminates all objects in a scene equally, regardless of their distance from the light source.
// It is used to add a basic level of illumination to a scene and can be used to simulate global illumination effects.
struct SUBMESH {
	// CPU-side data.
	std::string OurMaterialName;							// The material name of the submesh. In this case the material name of the description of the texture image. It is not used in rendering but for informational purposes only.
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pTextureView; // Smart pointer to a shader resource view interface. A shader resource view interface specifies the subresource a shader can access during rendering. In this case the texture image.
	// OurIndices:	 Holds the data used to initialize the GPU-side index buffer via pIBuffer, the pointer to the index buffer interface.
	std::vector<DWORD> OurIndices;							// The dynamically allocated array of DWORD indices, where each array element points to  a unique set of vertex attributes in an OurVertices array element. Multiple array elements will point to the same OurVertices array element.
	int IndicesTotal = 0;									// The total number of array elements in array variable OurIndices	(OurIndices.size()),  e.g., 36 array elements specify a cube. Manually initialized as type int does not have a default constructor.

	// GPU-side data.
	Microsoft::WRL::ComPtr<ID3D11Buffer> pIBuffer;			// Smart pointer to a buffer interface. A buffer interface accesses a buffer resource, which is unstructured memory. In this case the index buffer.
};

struct OBJECT {
	// CPU-side data.
	std::string OurObjectName;								// The name of the object. It is optional, and not used in rendering but for informational purposes only.
	//   OurVertices:	 Holds the data used to initialize the GPU-side vertex buffer via pVBuffer, the pointer to the vertex buffer interface.
	std::vector<VERTEX> OurVertices;						// The dynamically allocated array of VERTEX structures, where each array element represents a unique set of vertex attributes describing one or more triangle vertices.
	int VertexAttributeSetsTotal = 0;						// The total number of array elements in array variable OurVertices (OurVertices.size()), e.g., 24 array elements specify a cube. Manually initialized as type int does not have a default constructor.
	std::vector<SUBMESH> OurSubMeshes;						// The dynamically allocated array of SUBMESH structures. Each submesh groups faces sharing the same material. Each array element (SUBMESH structure) contains the set of attributes of one submesh, including the texture image and the vertex and index data for that submesh.
	struct {
		DirectX::XMMATRIX matFinal;							// The final transformation matrix.
		DirectX::XMMATRIX matRotate;						// The final rotation matrix.
		DirectX::XMFLOAT4 LightVector;						// Directional light's direction.
		DirectX::XMFLOAT4 LightColor;						// Directional light's color (whiter color == brighter color).
		DirectX::XMFLOAT4 AmbientColor;						// Ambient     light's color (whiter color == brighter color).
	} ConstantBuffer;

	// GPU-side data.
	Microsoft::WRL::ComPtr<ID3D11Buffer> pVBuffer;			// Smart pointer to a buffer interface. A buffer interface accesses a buffer resource, which is unstructured memory. In this case the vertex buffer.
	Microsoft::WRL::ComPtr<ID3D11Buffer> pCBuffer;			// Smart pointer to a buffer interface. A buffer interface accesses a buffer resource, which is unstructured memory. In this case the constant buffer.
};

// End: Structure Declarations.

//***
// External Variable Declarations.
// Declare variables as external by specifying 'extern' here. Otherwise building the project results in Visual Studio Linker Tools Error LNK2005 (symbol already defined in object).
// Define external variables in one and only one source file (not this one) and initialize them as needed.
//***

// OurObjects.
extern std::vector<OBJECT> OurObjects;						// The dynamically allocated array of OBJECT structures, with each array element containing the set of attributes of one named object.

// End: External Variable Declarations.