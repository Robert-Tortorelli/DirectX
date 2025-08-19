// objReader Header File
// Version 3.3
//
// Description:
// objReader Header File for Wavefront .obj file I/O.
//
// This header file contains all global declarations, including external variables, functions, structures, and classes.
// Include it in all source files that reference these global declarations.
// The only variables declared here are external. The objReader function source file defines the external variables declared here.
// Internal variables are declared in the source files that define them.
//
// Header files should not contain "using directives" (such as using namespace std;) or "using declarations" (such as using std::cout;).
//
// Authorship:
// Robert John Tortorelli

//***
// Declarations.
//***

// Pragma Directives.
// Specify that the compiler include this header file only once when compiling source code files.
#pragma once												// Automatically included by Visual Studio 2022 when creating a header file.

// Standard Encapsulated Data and Functions for Manipulating String Data.
#include <string>											// String class member functions stof, to_string, etc.

// Vector Container Class.
#include <vector>											// Vector class member functions push_back, pop_back, etc.

// Windows API Header File.
#include <windows.h>										// The Windows API (Win32 API) header file enables you to create 32-bit and 64-bit programs. It includes declarations for both Unicode and ANSI versions of the API. For more information, see Unicode in the Windows API.

// Direct3D Header Files.
#include <d3d11.h>											// This header is used by Direct3D 11 Graphics.

// DirectXMath Header File.
#include <directxmath.h>                                    // The DirectXMath API provides SIMD-friendly C++ types and functions for common linear algebra and graphics math operations common to DirectX programs.

//***
// Function Declarations.
// Function prototypes for functions (e.g., objReader) called by programs (e.g., objRenderer) that include this header file. They are optional in the functions named here (e.g., objReader).
//***

int objReaderEnum(void);
int objReader(const std::string& filename = "Text.obj");

// End: Function Declarations.

//***
// External Variables.
//***

//***
// Structure Declarations for External Variables.
// Structures must be declared before the variables they are used to define, e.g., OurVertices and OurObjects.
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

// OBJECT 'named structure' data type.
// The set of attributes of one object.
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
// The matFinal member is the 4x4 final transformation matrix that represents the combined world, view, and projection transformations that are applied to the geometric vertices of the geometry being rendered.
//
// The matRotate member is the 4x4 final rotation matrix that represents a rotation transformation that is applied to the geometric vertices of the geometry being rendered.
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
struct OBJECT {
	// CPU-side buffer data.
	std::string OurName;									// The name of the object.

	std::vector<VERTEX> OurVertices;						// The dynamically allocated array of VERTEX structures, where each array element (VERTEX structure) represents a unique set of vertex attributes. Each array element (VERTEX structure) may describe one or more triangle vertices and is referenced via the indices in array variable OurIndices.
	int VertexAttributeSetsTotal = 0;						// The total number of array elements in array variable OurVertices (OurVertices.size()), e.g., 24 array elements specify a cube. Manually initialized as type int does not have a default constructor.

	std::vector<DWORD> OurIndices;							// The dynamically allocated array of DWORD indices, with each array element (index) pointing to the corresponding unique set of vertex attributes (for one of the three vertices of a triangle) in an OurVertices array element (VERTEX structure). Multiple array elements (indices) can point to the same OurVertices array element.
	int IndicesTotal = 0;									// The total number of array elements in array variable OurIndices	(OurIndices.size()),  e.g., 36 array elements specify a cube. Manually initialized as type int does not have a default constructor.

	struct {
		DirectX::XMMATRIX matFinal;							// The final transformation matrix.
		DirectX::XMMATRIX matRotate;						// The final rotation matrix.
		DirectX::XMFLOAT4 LightVector;						// Directional light's direction.
		DirectX::XMFLOAT4 LightColor;						// Directional light's color (whiter color == brighter color).
		DirectX::XMFLOAT4 AmbientColor;						// Ambient     light's color (whiter color == brighter color).
	} ConstantBuffer;

	// GPU-side buffer data.
	ID3D11Buffer* pVBuffer = nullptr;						// Pointer to a buffer interface. A buffer interface accesses a buffer resource, which is unstructured memory. In this case the vertex buffer.

	ID3D11Buffer* pIBuffer = nullptr;						// Pointer to a buffer interface. A buffer interface accesses a buffer resource, which is unstructured memory. In this case the index buffer.

	ID3D11Buffer* pCBuffer = nullptr;						// Pointer to a buffer interface. A buffer interface accesses a buffer resource, which is unstructured memory. In this case the constant buffer.
};

// End: Structure Declarations for External Variables.

//***
// External Variable Declarations.
// Declare variables as external by specifying 'extern' here. Otherwise building the project results in Visual Studio Linker Tools Error LNK2005 (symbol already defined in object).
// Define external variables in one and only one source file (not this one) and initialize them as needed.
//***

// OurObjects.
extern std::vector<OBJECT> OurObjects;						// The dynamically allocated array of OBJECT structures, with each array element containing the set of attributes of one named object.
// OurObjects Supplemental Variables.
extern int OurObjectsi;										// The index variable OurObjectsi of array variable OurObjects[OurObjectsi].

// OurVertices Supplemental Variables.
extern int OurVerticesi;									// The index variable OurVerticesi of array variable OurVertices[OurVerticesi].

// OurIndices Supplemental Variables.
extern int OurIndicesi;										// The index variable OurIndicesi of array variable OurIndices[OurIndicesi].

// End: External Variable Declarations.

// End: External Variables.

// End: Declarations.