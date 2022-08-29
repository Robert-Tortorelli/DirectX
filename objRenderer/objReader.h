// Wavefront .obj file I/O (objReader function) header file
// Version 3.1
//
// Description
// This header file declares external global variables that support Wavefront .obj file I/O via the objReader function.
// Include it in all source files that reference these external global variables, and always in the objReader function source file.
// Then define these external global variables, and optionally initialize them, in one and only one source file, in this case the objReader function source file.
// An objReader function prototype is declared in any source file that calls the objReader function, but need not be declared in the objReader function itself. Therefore, an objReader function prototype is not included in this header file, because this header file is always included in the objReader function source file.

// #pragma once is a non-standard but widely supported preprocessor directive designed to cause the current source file to be included only once in a single compilation. Thus, #pragma once serves the same purpose as include guards, but with several advantages, including: less code, avoidance of name clashes, and sometimes improvement in compilation speed. On the other hand, #pragma once is not necessarily available in all compilers and its implementation is tricky and might not always be reliable (Wikipedia).
#pragma once												// Automatically included by Visual Studio 2022 when creating a header file.

// Declare the VERTEX structure with a member list representing the vertex elements of one vertex of an object.
struct VERTEX
{
	float X, Y, Z;
	float Color[4];
};

// Declare global variables external by declaring them 'extern' here. Otherwise building the project results in Linker Tools Error LNK2005 (symbol already defined in object).
//
// Declare the vertex buffer.
// "Vn" is computed at run time to be the number of vertices comprising an object.
// After "Vn" is computed, allocate OurVertices as an array (OurVertices[VertexNo]) of type VERTEX using "OurVertices = new VERTEX[Vn]".
extern int Vn;												// The number of vertices comprising the object.
extern VERTEX* OurVertices;									// The vertex buffer, a pointer to the first element in the array of VERTEX structures, with each structure representing the vertex elements of one vertex of an object.
//
// Declare the index buffer.
// "In" is computed at run time to be the number of triangle primitives comprising an object.
// After "In" is computed, allocate OurIndices as an array (OurIndices[FaceElementNo]) of type DWORD using OurIndices = new DWORD[In * 3]. Three vertex indices (each pointing to a vertex in OurVertices) describe each triangle, and "In" is the number of triangles comprising the object. Therefore In * 3.
extern int In;												// The number of primitives comprising the object. In this program, they are triangle primitives.
extern DWORD* OurIndices;									// The index buffer, a pointer to the first element in the array of vertex indices of an object.