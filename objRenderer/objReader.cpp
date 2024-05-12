// objReader
// Version 3.2
//
// Description
// This function parses a Wavefront .obj file for a single 3D object's descriptive information and assigns it to the external global variables used by the calling program when rendering the object.
//
// Authorship
// Robert John Tortorelli

//***
// Global Declarations.
// Declaring a pointer variable at the global scope (outside any function) automatically initializes it to nullptr if you don't provide an initial value.
// This ensures it does not point to any memory location until it is explicitly initialized.
//***

// objReader Header File for Wavefront .obj file I/O.
#include "objReader.h"

// Standard Encapsulated Data and Functions for Manipulating String Data.
#include <string>											// String class member functions stof, to_string, etc.

// File Stream Functions.
#include <fstream>											// File stream class member functions get, close, etc.

// String stream class member functions.
#include <sstream>											// String stream class member functions getline, etc.

// Using Declarations and Directives.
// Using declarations such as using std::string;   bring one identifier	 in the named namespace into scope.
// Using directives	  such as using namespace std; bring all identifiers in the named namespace into scope.
// Using declarations are preferred to using directives.
// Using declarations and directives must appear after their respective header file includes.
using std::fill;
using std::ifstream;
using std::ios;
using std::stof;
using std::string;
using std::vector;
using std::istringstream;

//***
// External Variable Global Definitions.
//***

// Define external variables in one and only one source file (this one) and initialize them as needed.
// See the associated header file for declarations and descriptions of these external variables.
vector<VERTEX> OurVertices;	int OurVerticesi = -1;	int VertexAttributeSetsTotal = 0;
vector<DWORD> OurIndices;	int OurIndicesi = -1;	int PrimitivesTotal = 0;

// End: External Variable Global Definitions.

// Declare variables used to parse the Wavefront .obj file.
// Intermediate arrays to temporarily store all vertex attributes before they are copied to the array variable OurVertices:
//   Each intermediate array is a one-dimensional array of structures, where each array element (each structure) contains vertex attributes of a given type for one vertex.
//   v  is the intermediate array of structures for geometric vertices,			where each structure contains three floating-point values (x, y, z).
//   vt is the intermediate array of structures for vertex texture coordinates, where each structure contains two   floating-point values (x, y; usually named U, V in computer graphics).
//   vn is the intermediate array of structures for vertex normal vectors,		where each structure contains three floating-point values (x, y, z).
//   Each intermediate array is indexed by a variable, vi, vni, or vti, initialized to -1, that is incremented by 1 each time a new vertex attribute is stored in the intermediate array.
// In this source code file fully qualified identifiers (e.g., DirectX::XMFLOAT3 instead of using namespace DirectX;) are used to define these intermediate arrays.
vector<DirectX::XMFLOAT3> v;  int vi = -1;					// Geometric vertices		  dynamically allocated intermediate array, and index (v[vi]).
vector<DirectX::XMFLOAT2> vt; int vti = -1;					// Vertex texture coordinates dynamically allocated intermediate array, and index (vt[vti]).
vector<DirectX::XMFLOAT3> vn; int vni = -1;					// Vertex normal vectors	  dynamically allocated intermediate array, and index (vn[vni]).
// Intermediate array variable OurIndicesFaceTriplet temporarily stores three indices of array variable OurVertices. Each index is derived from one of the three face element triplets in a face element statement (e.g., v1/vt1/vn1) that represent the set of vertex attributes for one of the three vertices of a triangle.
// The three indices are stored in the counter-clockwise drawing order specified by the order of face element statements in the Wavefront .obj file. These indices will be converted to the clockwise drawing order used by DirectX, and then stored in the array variable OurIndices.
int OurIndicesFaceTriplet[3];
int OurIndicesFaceTripleti;									// The index variable OurIndicesFaceTripleti of intermediate array variable OurIndicesFaceTriplet[OurIndicesFaceTripleti].

// End: Global Declarations.

//***
// Function Definitions.
//***

// objReader function: Definition
int objReader(void)
{
	ifstream obj;											// Declare the input file stream object representing the Wavefront .obj file.
	string stringtext;										// Holds one statement of the input file stream object representing the Wavefront .obj file.

	// Wavefront .obj file format requirements:
	// - Supported and required statements are:
	//   Vertex attribute statements: geometric vertex statements (v x y z), vertex texture coordinate statements (vt u v), and vertex normal vector statements (vn x y z).
	//
	//   Face element statements: f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3
	//   These specify three triplets, one triplet (e.g., v1/vt1/vn1) for each of the three vertices of a triangle, where v1, v2, v3 are geometric vertex indices, vt1, vt2, vt3 are vertex texture coordinate indices, and vn1, vn2, vn3 are vertex normal vector indices.
	//   These indices are positive numbers referring to vertex attribute statements by the order in which the vertex attribute statements appear in the Wavefront .obj file.
	//
	//   Vertex attribute statements are listed first, and will therefore be parsed before any face element statements.
	//   The order of the face element statements determines the order in which the triangles must be drawn. This order is important when dealing with overlapping triangles, as the later triangles will be drawn on top of the earlier ones. Face element statements are parsed in this order.
	//   All other statements are ignored.
	// - No spaces are permitted before or after a slash ('/').
	// - Statements can start in any column.
	// - Statements can be logically joined with the line continuation character ( \ ) at the end of a line. (This is *not* supported by this program)
	//
	// Open the Wavefront .obj file for input.
	obj.open("Text.obj", ios::in);
	// Check whether the Wavefront .obj file opened successfully. (!obj), (!obj.is_open()), and (obj.fail()) all indicate an error opening the file.
	if (!obj)												// If not (!) successful (obj) then:
	{
		// Cannot open the Wavefront .obj file.

		// Terminate this function with a return code indicating an error.
		return 1;
	}

	// Parse the Wavefront .obj file.
	while (getline(obj, stringtext))						// Read an entire statement, from the input file stream object obj, into the string variable stringtext. At eof getline becomes false and the while loop is exited.
	{
		istringstream lineStream(stringtext);				// Declaring object lineStream inside the loop ensures that it is reset and ready to parse the next line of the file in each iteration of the loop.
		string type;										// Wavefront .obj file statement type: #, v, vn, vt, f, etc.
															// Declaring variable "type" inside the while loop ensures that it is reset at the start of each iteration, which is generally what you want when reading a new line from the file.
		lineStream >> type;									// The >> operator extracts the next value from the lineStream input stream object and stores it in the variable "type". At this point, the value is the first word of the statement: #, v, vn, vt, f, etc.

		if (type == "v")
		{
			// The statement read is a geometric vertex statement. Parse and store it in variable v.
			vi++;												// Increment the geometric vertex index.
			v.emplace_back();									// Add a new element to this dynamic intermediate array.
			lineStream >> v[vi].x >> v[vi].y >> v[vi].z;		// The >> operator extracts the next three values from the lineStream input stream object and stores them in the intermediate array variables v[vi].x, then v[vi].y, then v[vi].z.
		} else if (type == "vn")
		{
			// The statement read is a vertex normal vector statement. Parse and store it in variable vn.
			vni++;												// Increment the vertex normal vector index.
			vn.emplace_back();									// Add a new element to this dynamic intermediate array.
			lineStream >> vn[vni].x >> vn[vni].y >> vn[vni].z;	// The >> operator extracts the next three values from the lineStream input stream object and stores them in the intermediate array variables vn[vni].x, then vn[vni].y, then vn[vni].z.
		} else if (type == "vt")
		{
			// The statement read is a vertex texture coordinate statement. Parse and store it in variable vt.
			vti++;												// Increment the vertex texture coordinate index.
			vt.emplace_back();									// Add a new element to this dynamic intermediate array.
			lineStream >> vt[vti].x >> vt[vti].y;				// The >> operator extracts the next two values from the lineStream input stream object and stores them in the intermediate array variables vt[vti].x, then vt[vti].y.
		} else if (type == "f")
		{
			// The statement read is a face element statement, therefore all vertex attribute statements in the Wavefront .obj file have previously been read, parsed, and stored in the array variables v, vt, and vn.
			// Now parse the face element statement: f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3
			//   Each of the three triplets (e.g., v1/vt1/vn1) in the face element statement consists of three positive numbers referring to three vertex attribute statements specifying one triangle vertex, which together comprise one set of vertex attributes. This new set of vertex attributes, if unique, is stored in the next sequential element of the array variable OurVertices, which is pointed to by array variable OurIndices.
			//   This results in storing sets of vertex attributes in array variable OurVertices in the order that the face element statements appear in the Wavefront .obj file, which is the order in which the triangles must be drawn.
			//   Note: A set of vertex attributes is only stored in array variable OurVertices if it is unique, i.e., if it has not been previously found in the Wavefront .obj file.

			// Check if any of the dynamic intermediate arrays v, vt, or vn is empty, i.e., if geometric vertex statements, vertex texture coordinate statements, or vertex normal vector statements are not specified in the Wavefront .obj file.
			// This can occur when the Wavefront .obj file is valid but only contains geometric vertex statements and face element statements, but not vertex texture coordinate statements and vertex normal vectors statements.
			if (vi == -1 || vti == -1 || vni == -1)			// If vi or vti or vni is -1, then the Wavefront .obj file does not contain the required vertex attribute statements.
			{
				// Cannot process the Wavefront .obj file.

				// Terminate this function with a return code indicating an error.
				return 2;
			}

			// Parse each of the three triplets in the face element statement.
			OurIndicesFaceTripleti = -1;					// Reset the index variable OurIndicesFaceTripleti of intermediate array variable OurIndicesFaceTriplet[OurIndicesFaceTripleti] to -1 for each new face element statement.
			for (int i = 0; i <= 2; i++)
			{
				// Face element vertex attribute indices of the current face element triplet.
				int fv;										// Geometric vertex index (v1, or v2, or v3).
				int fvt;									// Vertex texture coordinate index (vt1, or vt2, or vt3).
				int fvn;									// Vertex normal vector index (vn1, or vn2, or vn3).
				char slash = '/';							// The slash character ('/') is a delimiter used to parse the face element.

				// Parse the current triplet (v/vt/vn) in the face element statement, storing the face element indices.
				lineStream >> fv >> slash >> fvt >> slash >> fvn;

				// Decrement the face element indices by 1 to adjust them from the Wavefront .obj file format to the *C++* format.
				// The face element indices are decremented by 1 because the C++ index variables vi, vti, and vni of the vertex attribute intermediate array variables v[vi], vt[vti], and vn[vn1] start with 0.
				//
				// (Face element indices in the Wavefront .obj file format start with 1, while those in the DirectX format start with 0. However, this program does not use the face element indices directly, so an adjustment *for DirectX* is not necessary.
				// Regardless, making an adjustment for DirectX effectively makes the face element indices start with 0, which is consistent with the C++ format)
				fv--; fvt--; fvn--;

				// Create a new element of dynamic array variable OurIndices.
				OurIndicesFaceTripleti++;					// Update the index variable OurIndicesFaceTripleti of the 3 element intermediate array variable OurIndicesFaceTriplet[OurIndicesFaceTripleti].
				OurIndicesi++;								// Update the index variable OurIndicesi of array variable OurIndices[OurIndicesi].
				OurIndices.emplace_back();					// The only OurIndices.emplace_back() statement, executed once for each face element triplet in all face element statements. Each face element triplet corresponds to a set of vertex attributes.

				// The candidate set of vertex attributes is comprised of v[fv], vt[fvt], and vn[fvn].
				// Test if the candidate set of vertex attributes is unique, i.e., has this set of vertex attributes been previously found in the Wavefront .obj file and stored in array variable OurVertices?
				// This is done to avoid storing duplicate (non-unique) sets of vertex attributes in array variable OurVertices.
				//
				// The uniqueness of a given candidate set of these vertex attributes can alternatively be determined by whether the associated face element triplet is unique. This is possible because although all geometric vertices are unique, and all vertex texture coordinates are unique, and all vertex normal vertices are unique in a Wavefront .obj file, a combination of these three vertex attributes can be non-unique. (This is not done in this program)
				// (If geometric vertices could be non-unique, or vertex texture coordinates could be non-unique, or vertex normal vertices could be non-unique, then two face element triplets comprised of different indices might mistakenly appear to be different but could still point to an identical (non-unique) set of vertex attributes)
				bool unique = true;																// Define a semaphore to indicate whether the candidate set of vertex attributes is unique.
				for (int j = 0; j <= OurVerticesi; j++)
				{
					if (OurVertices[j].GeometricVertex.x ==				  v[fv].x		   &&
						OurVertices[j].GeometricVertex.y ==				  v[fv].y		   &&
						OurVertices[j].GeometricVertex.z ==				  v[fv].z * -1.0f  &&	// Invert the geometric vertex's Z coordinate, to adjust it from the Wavefront .obj file format to the DirectX format.

						OurVertices[j].VertexTextureCoordinate.x ==		  vt[fvt].x		   &&
						OurVertices[j].VertexTextureCoordinate.y ==		  1.0f - vt[fvt].y &&	// Invert the vertex texture coordinate's V coordinate, to adjust it from the Wavefront .obj file format to the DirectX format.

						OurVertices[j].VertexNormalVector.x ==			  vn[fvn].x		   &&
						OurVertices[j].VertexNormalVector.y ==			  vn[fvn].y		   &&
						OurVertices[j].VertexNormalVector.z ==			  vn[fvn].z * -1.0f)	// Invert the vertex normal vector's Z coordinate, to adjust it the Wavefront .obj file format to the DirectX format.
					{
						// The candidate set of vertex attributes is non-unique, so no new set of vertex attributes is created and stored in the array variable OurVertices.
						// No  new element of array variable OurVertices						is created, as an existing set of vertex attributes is reused.
						// The new element of intermediate array variable OurIndicesFaceTriplet is assigned the index (the value of variable j) of the element of array variable OurVertices[j], which contains the set of vertex attributes previously found.
						unique = false;
						OurIndicesFaceTriplet[OurIndicesFaceTripleti] = j;
						break;
					}
				}
				if (unique)
				{
					// The candidate set of vertex attributes is unique, so a new set of vertex attributes is created and stored in the array variable OurVertices.
					// A   new element of dynamic array variable OurVertices is created, and the candidate set of vertex attributes is assigned to it.
					// The new element of array variable OurIndicesFaceTriplet is assigned.
					OurVerticesi++;																// Update the index variable OurVerticesi of array variable OurVertices[OurVerticesi].
					OurVertices.emplace_back();													// The only OurVertices.emplace_back() statement, executed once for each unique set of vertex attributes in all face element statements.

					OurVertices[OurVerticesi].GeometricVertex.x =		  v[fv].x;
					OurVertices[OurVerticesi].GeometricVertex.y =		  v[fv].y;
					OurVertices[OurVerticesi].GeometricVertex.z =		  v[fv].z * -1.0f;		// Invert the geometric vertex's Z coordinate, to adjust it from the Wavefront .obj file format to the DirectX format.

					OurVertices[OurVerticesi].VertexTextureCoordinate.x = vt[fvt].x;
					OurVertices[OurVerticesi].VertexTextureCoordinate.y = 1.0f - vt[fvt].y;		// Invert the vertex texture coordinate's V coordinate, to adjust it from the Wavefront .obj file format to the DirectX format.

					OurVertices[OurVerticesi].VertexNormalVector.x =	  vn[fvn].x;
					OurVertices[OurVerticesi].VertexNormalVector.y =	  vn[fvn].y;
					OurVertices[OurVerticesi].VertexNormalVector.z =	  vn[fvn].z * -1.0f;	// Invert the vertex normal vector's Z coordinate, to adjust it the Wavefront .obj file format to the DirectX format.

					OurIndicesFaceTriplet[OurIndicesFaceTripleti] = OurVerticesi;				// The next element of intermediate array variable OurIndicesFaceTriplet is assigned the index of the next element of array variable OurVertices.
																								// At this point the drawing order of triangle vertices is still counter-clockwise (Wavefront .obj file) and must be converted to clockwise (DirectX).
				}
			}
			// Convert the drawing order of triangle vertices in the current face element statement from counter-clockwise (Wavefront .obj file) to clockwise (DirectX).
			// When rendering an object with DirectX, the drawing order of vertices is determined by the values of the sequential elements of the array variable OurIndices.
			// The values of these elements are the indices of array variable OurVertices, and would also be sequential, were it not for,
			// - The need to avoid storing duplicate (non-unique) sets of vertex attributes in array variable OurVertices as discussed above.
			// - The need to convert the drawing order to clockwise.
			//
			// At this point, the three face element triplets in the current face element statement have been processed:
			// - The associated three sets of vertex attributes				are stored in array variable OurVertices.
			// - The associated three indices of array variable OurVertices are stored in intermediate array variable OurIndicesFaceTriplet.
			//   The drawing order of these three indices is counter-clockwise (Wavefront .obj file) in intermediate array variable OurIndicesFaceTriplet, and must be converted to clockwise (DirectX) in array variable OurIndices.
			// This can be done by reversing the order of processing the second and third set of vertex attributes of each triangle's three vertices, which is equivalent to changing the order by which the elements of array variable OurVertices are referenced by the elements of array variable OurIndices:
			// - OurVertices[OurVerticesi] is OurVertices[OurIndices[OurIndicesi]], where instead of OurIndices[OurIndicesi] = 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 etc. the drawing order is reversed by reordering OurIndices[OurIndicesi] = 0, 2, 1, 3, 5, 4, 6, 8, 7, 9, 11, 10, etc. (This is done in this program)
			// - Alternatively, the elements of the array variable OurVertices could be reordered while keeping the elements of the array variable OurIndices[OurIndicesi] = 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 etc. (This is not done in this program)
			// At this point, OurIndicesi is the index of the last element of array variable OurIndices.
			// The last three elements of the array variable OurIndices are assigned the values of the elements of the intermediate array variable OurIndicesFaceTriplet, converted to clockwise (DirectX).
			OurIndices[OurIndicesi - 2] = OurIndicesFaceTriplet[0];								// OurIndicesi - 2 corresponds to the index of the first  element of the three element intermediate array variable OurIndicesFaceTriplet.
			OurIndices[OurIndicesi - 1] = OurIndicesFaceTriplet[2];								// OurIndicesi - 1 corresponds to the index of the third  element of the three element intermediate array variable OurIndicesFaceTriplet.
			OurIndices[OurIndicesi]     = OurIndicesFaceTriplet[1];								// OurIndicesi     corresponds to the index of the second element of the three element intermediate array variable OurIndicesFaceTriplet.
		} else continue;																		// The statement read is not a geometric vertex, vertex texture coordinate, vertex normal vector, or face element statement. Ignore it and continue.
	}
	// End of the while loop. The entire Wavefront .obj file has been read and parsed.

	// Close the Wavefront .obj file.
	obj.close();

	// Assign the total number of unique sets of vertex attributes in array variable OurVertices to the external global variable VertexAttributeSetsTotal.
	VertexAttributeSetsTotal = OurVerticesi + 1;
	// Assign the total number of triangle primitives (the total number of array elements in OurIndices / 3) to the external global variable PrimitivesTotal.
	PrimitivesTotal = (OurIndicesi + 1) / 3;

	// Return to the calling program with a return code indicating success.
	return 0;
}