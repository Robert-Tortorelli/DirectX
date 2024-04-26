// objReader Header File
// Version 3.1
//
// Description
// Wavefront .obj file I/O Header File
// Version 3.1
//
// Description
// This header file contains all global declarations, including external variables, functions, structures, and classes.
// Include it in all source files that reference these global declarations.
// The only variables declared here are external. The objReader function source file defines the external variables declared here.
// Internal variables are declared in the source files that define them.
//
// Header files should not contain "using directives" (such as "using namespace std") or "using declarations" (such as "using std::cout").
//
// Authorship
// Robert John Tortorelli

//***
// Global Declarations.
// Declaring a pointer variable at the global scope (outside any function) automatically initializes it to nullptr if you don't provide an initial value.
// This ensures it does not point to any memory location until it is explicitly initialized.
//***

// Pragma Directives.
// Specify that the compiler include this header file only once when compiling source code files.
#pragma once												// Automatically included by Visual Studio 2022 when creating a header file.

// Vector Container Class.
#include <vector>											// Vector class member functions push_back, pop_back, etc.

// DWORD Header File.
#include <intsafe.h>										// Required for the DWORD data type. Note the main objRenderer program (with the WinMain function) includes the larger windows.h, containing IntSafe.h, instead.

//***
// DirectX Global Declarations.
//***

// DirectXMath Header File.									// The DirectXMath API provides SIMD-friendly C++ types and functions for common linear algebra and graphics math operations common to DirectX programs.
#include <directxmath.h>                                    // Used with the namespace DirectX.

// Using Declarations and Directives.
// Using declarations such as using std::string;   bring one identifier	 in the named namespace into scope.
// Using directives	  such as using namespace std; bring all identifiers in the named namespace into scope.
// Using declarations are preferred to using directives.
// Using declarations and directives must appear after their respective header file includes.
using namespace DirectX;									// The DirectXMath API is in the DirectX namespace.

// End: DirectX Global Declarations.

//***
// Structure Declarations.
// Structures used to define new data types that can contain multiple members of different types.
// They must be declared before the variables they are used to define, i.e., OurVertices.
//***

// Declare the VERTEX 'named structure' data type, a dynamic array of structures.
// The input element description structure is used to define the input-layout object that describes the VERTEX structure.
// If the VERTEX structure is changed then the input element description structure (defined in objRenderer.cpp) must be changed accordingly.
struct VERTEX {												// Vertex attributes.
	XMFLOAT3 GeometricVertex;								// Geometric vertex attribute:			.x, .y, .z	("v " element in the Wavefront .obj file)
	XMFLOAT3 VertexNormalVector;							// Vertex normal vector attribute:		.x, .y, .z	("vn" element in the Wavefront .obj file)
	XMFLOAT2 VertexTextureCoordinate;						// Vertex texture coordinate attribute:	.x, .y		("vt" element in the Wavefront .obj file)
};

// End: Structure Declarations.

//***
// External Variable Global Declarations.
//***

// Declare variables as external by specifying 'extern' here. Otherwise building the project results in Visual Studio Linker Tools Error LNK2005 (symbol already defined in object).
// Define external variables in one and only one source file (not this one) and initialize them as needed.
//
// OurVertices (used to initialize the DirectX vertex buffer), a variable containing values formatted for DirectX, is the array of unique sets of vertex attributes of a single 3D object.
extern std::vector<VERTEX> OurVertices;						// Dynamically allocated array of VERTEX structures, with each array element containing the vertex attributes of one of the three vertices of a triangle.
extern int OurVerticesi;									// The index variable OurVerticesi of array variable OurVertices[OurVerticesi].
// A cube's 6 sides				 are each comprised of 4 corners (vertices) so there are 4 unique sets of vertex attributes per side   for a total of 6 x 4 = 24 unique sets of vertex attributes.
// A cube's 8 corners (vertices) are each part		of 3 sides				so there are 3 unique sets of vertex attributes per corner for a total of 8 x 3 = 24 unique sets of vertex attributes.
// By contrast, in a program where (unlike this program) the only vertex attribute is the geometric vertex, there are only 8 unique sets of vertex attributes, i.e., each set contains only a single geometric vertex.
extern int VertexAttributeSetsTotal;						// The total number of array elements in OurVertices, e.g., 24 array elements specify a cube.
//
// OurIndices (used to initialize the DirectX index buffer), a variable containing values formatted for DirectX, is an array of non-unique indices of OurVertices, each pointing to an associated unique set of vertex attributes in OurVertices.
// One unique set of vertex attributes in OurVertices, OurVertices(y), may be referenced more than once by duplicate (non-unique) indices in OurIndices:
// e.g., OurIndices(x1) and OurIndices(x2) = y (y is the duplicate (non-unique) index in OurIndices), both pointing to the same unique set of vertex attributes of OurVertices(y), e.g., OurIndices(x1) = y so OurVertices(OurIndices(x1)) = OurVertices(y).
extern std::vector<DWORD> OurIndices;						// Dynamically allocated array of DWORD indices, with each array element pointing to the corresponding set of vertex attributes (for one of the three vertices of a triangle) in an OurVertices array element.
extern int OurIndicesi;										// The index variable OurIndicesi of array variable OurIndices[OurIndicesi].
// A cube's 6 sides are comprised of 2 triangle primitives per side, for a total of 6 x 2 = 12 triangle primitives, each triangle primitive comprised of 3 vertices, for a total of 12 x 3 = 36 non-unique geometric vertex indices.
extern int PrimitivesTotal;									// The total number of triangle primitives comprising a single 3D object, e.g., 12 triangle primitives specify a cube and the total number of array elements in OurIndices is PrimitivesTotal * 3 = 36.

// End: External Variable Global Declarations.

//***
// Global Function Declarations.
// Function prototypes for functions (e.g., objReader) called by programs (e.g., objRenderer) that include this header file. They are optional in the functions named here (e.g., objReader).
//***

int objReader(void);										// The objReader function parses a single 3D object's Wavefront .obj file and uses it to populate the external global variables OurVertices and OurIndices.

// End: Global Function Declarations.

// End: Global Declarations.