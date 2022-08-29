// objReader function: Definition
// Version 3.1
//
// Description
// This function reads and parses a single 3D object's descriptive information from a Wavefront .obj file and uses it to define the data structures needed to render the 3D object.
// Read and store vertex information in OurVertices, and index information in OurIndices.

using namespace std;

// Standard Encapsulated Data and Functions for Manipulating String Data.
#include <string>

// Standard I/O (Console Screen and Keyboard).
#include <iostream>											// Stream objects cin, cout, cerr, clog, etc.

// File I/O.
#include <fstream>											// Stream object member functions get, close, etc.

// DWORD.
#include <IntSafe.h>										// Required for the DWORD data type. Note the main objRenderer program (with the WinMain function) includes the larger windows.h, containing IntSafe.h, instead.

// Wavefront .obj file I/O (objReader function).
#include "objReader.h"										// Declare external global variables in this header file, and include it in all source files that reference these external global variables.
// Define external global variables in one and only one source file, this one, and initialize them as needed.
// See the associated header file for comments on these global variables.
int Vn = 0;
VERTEX* OurVertices;
int In = 0;
DWORD* OurIndices;

void objReader(void)
{
	// ***
	// 1. Read and store vertex information in OurVertices, and index information in OurIndices.
	// ***

	ifstream obj;											// Declare the file stream object representing the Wavefront .obj file.
	string stringtext;										// Holds one record read from the file stream object representing the Wavefront .obj file.
	size_t nCharCounter = 0;								// Index into a vertex element or face element, that's being read from the Wavefront .obj file.
	size_t nOffset;
	string::iterator it;

	// Three vertex indices, each a face element pointing to a vertex comprised of three vertex elements, describe a face; i.e., a triangle.
	const string v("v ");									// "v " prefix indicating a record of the Wavefront .obj file containing three geometric vertex elements describing one vertex of an object.
	const string f("f ");									// "f " prefix indicating a record of the Wavefront .obj file containing three polygonal face elements   describing one face   of an object.

#define MAX_ELEMENT_SIZE 50								// Define the maximum vertex element and face element size.
	char tempchar[MAX_ELEMENT_SIZE] = { '\0' };				// Holds one complete vertex element or face element, null-terminated by initializing all array elements to the null-terminator.
	int ignoreFlag = 0;										// Only save characters of a face element when ignoreFlag = 0.

	// Variables used to track which vertex and vertex element is being read from the Wavefront .obj file.
	int VertexNo = -1;										// The number of the vertex being read, counting from 0. This corresponds to the vertex index (>= 0), a face element.
	int VertexElementNo;									// The number of the vertex element of the vertex being read: x (VertexElementNo = 1), y (VertexElementNo = 2) or z (VertexElementNo = 3).

	// Variables used to track which face and face element is being read from the Wavefront .obj file.
	int FaceNo = -1;										// The number of the face being read, counting from 0. This is informational, not needed or used. Consider logging it.
	int FaceElementNo = 0;									// The number of the vertex index, a face element, being read, counting from 0. It's counted relative to the first vertex index of the first face, sequentially across all faces. Its final value is one greater than the final value used.

	// Open the Wavefront .obj file for input.
	obj.open("Text.obj", ios::in);
	// Check whether the Wavefront .obj file opened successfully.
	if (!obj)
	{
		// Cannot open the Wavefront .obj file.

		exit(1);
	}
	// Compute "Vn" at run time based on the number of vertices comprising the object as defined in the Wavefront .obj file.
	// Compute "In" at run time based on the number of triangles, a face element, comprising the object as defined in the Wavefront .obj file.
	while (getline(obj, stringtext))						// Read the entire line as a string.
	{
		if (nOffset = stringtext.find(v, 0) != string::npos)// nOffset is 1 if "v " is found in column 1. *** The .find documentation says it should be 0 ***
		{
			// A "v " vertex row in the Wavefront .obj file describing a new vertex has been found.

			++Vn;											// Count the vertex that was found.
		}
		else if (nOffset = stringtext.find(f, 0) != string::npos)
		{
			// A "f " face row in the Wavefront .obj file describing a new triangle has been found.

			++In;											// Count the face that was found.
		}
	}
	OurVertices = new VERTEX[Vn];							// After "Vn" is computed, allocate memory for OurVertices.
	OurIndices = new DWORD[In * 3];							// After "In" is computed, allocate memory for OurIndices. Three vertex indices (each pointing to a vertex in OurVertices) describe each triangle, and "In" is the number of triangles comprising the object. Therefore In * 3.

	// Resetting to the beginning of the previously opened Wavefront .obj file before assigning values to OurVertices and OurIndices.
	obj.clear();											// The Wavefront .obj file has already been read to eof, so the stream's internal error state flags indicate eof. Therefore all subsequent input operations fail unless you first clear the stream's flags to good.
	obj.seekg(0, ios::beg);									// Sets the position of the next character to be read to the stream's beginning.

	// Assign values to OurVertices and OurIndices.
	while (getline(obj, stringtext)) // Read the entire line as a string.
	{
		if (nOffset = stringtext.find(v, 0) != string::npos)
		{
			// A "v " vertex row in the Wavefront .obj file describing a new vertex has been found.

			++VertexNo;										// We are reading a new Vertex.
			VertexElementNo = 1;							// We reading the first vertex element, X, of a new Vertex.

			// Initialize the Vertex's Color using an arbitrary formula:
			// Color[x] is 0 or 1 because (n MOD 2) is 0 or 1, where n is any real number.
			OurVertices[VertexNo].Color[0] = (float)(VertexNo / 1 % 2);
			OurVertices[VertexNo].Color[1] = (float)(VertexNo / 2 % 2);
			OurVertices[VertexNo].Color[2] = (float)(VertexNo / 3 % 2);
			OurVertices[VertexNo].Color[3] = (float)(VertexNo / 4 % 2);

			// Parse the current "v " vertex row in the Wavefront .obj file for vertex elements, and store them.
			// nOffset is incremented to 2 if "v " is found in column 1.
			// The function strtok is an alternative way to split a string into tokens.
			for (it = stringtext.begin() + ++nOffset; it != stringtext.end(); ++it)
			{
				if (*it != ' ')
				{
					// Not ' ' indicates a digit of a vertex element. Initially *it is pointing to the first non-blank value after "v ".

					tempchar[nCharCounter] = *it;
					++nCharCounter;
				}
				else
				{
					// ' ' char detected, which indicates a complete vertex element has been read into "tempchar".

					switch (VertexElementNo)
					{
						// *** case 1 may execute ***
					case 1:	OurVertices[VertexNo].X = stof(tempchar);
						break;
						// *** case 2 may execute ***
					case 2:	OurVertices[VertexNo].Y = stof(tempchar);
						break;
						// *** case 3 will never execute ***
					case 3:	OurVertices[VertexNo].Z = stof(tempchar);
						// Invert each vertices' Z coordinate.
						OurVertices[VertexNo].Z = OurVertices[VertexNo].Z * -1.0f;
						break;
					}
					// Clear "tempchar" with the null-terminator.
					fill(tempchar, tempchar + MAX_ELEMENT_SIZE, '\0');
					// Reset "tempchar's" array element pointer.
					nCharCounter = 0;
					// Prepare to read the next vertex element, y or z.
					++VertexElementNo;
				}
			}
			// End of line reached, which indicates this line's final complete vertex element has been read into "tempchar".
			switch (VertexElementNo)
			{
				// *** case 1 will never execute ***
			case 1:	OurVertices[VertexNo].X = stof(tempchar);
				break;
				// *** case 2 will never execute ***
			case 2:	OurVertices[VertexNo].Y = stof(tempchar);
				break;
				// *** case 3 may execute ***
			case 3:	OurVertices[VertexNo].Z = stof(tempchar);
				// Invert each vertices' Z coordinate.
				OurVertices[VertexNo].Z = OurVertices[VertexNo].Z * -1.0f;
				break;
			}
			// Clear "tempchar" with the null-terminator.
			fill(tempchar, tempchar + MAX_ELEMENT_SIZE, '\0');
			// Reset "tempchar's" array element pointer.
			nCharCounter = 0;
			// Get another line from the while loop.
		}
		else if (nOffset = stringtext.find(f, 0) != string::npos)
		{
			// A "f " face row in the Wavefront .obj file describing a new triangle has been found. Three vertex indices (each pointing to a vertex in OurVertices) describe a triangle.

			++FaceNo;										// We are reading a new Face. This is informational, not needed or used. Consider logging it.

			// Parse the current "f " face row in the Wavefront .obj file for face elements, and store them.
			for (it = stringtext.begin() + ++nOffset; it != stringtext.end(); ++it)
			{
				if (*it != ' ' && *it != '/')
				{
					// Not ' ' AND Not '/' indicates a digit of a face element.

					if (ignoreFlag == 0)
					{
						// Save characters of the face element, as it's a vertex index.

						tempchar[nCharCounter] = *it;
						++nCharCounter;
					}
				}
				else
				{
					// ' ' OR '/' char detected:
					// ' ' demarks a vertex index, or	a vertex Texture Coordinate Index, or		a vertex Normal Index.
					// '/' demarks a vertex index, or	a vertex Texture Coordinate Index, but not	a vertex Normal Index.
					// '/' indicates					a vertex Texture Coordinate Index, or		a vertex Normal Index follows.

					if (*it == ' ')
					{
						// ' ' char detected, which indicates a complete face element, a vertex index, has been read into "tempchar" (ignoreFlag ensures "tempchar" only holds a vertex index).

						// Convert (reverse) the triangle primitive drawing order from counter-clockwise (used by the Wavefront .obj specification) to clockwise (used by DirectX).
						// "OurIndices[FaceElementNo] = strtol(tempchar, NULL, 10);"			// Store the face element, a vertex index, in the index buffer. *** Order of vertices is unchanged ***
						OurIndices[(In * 3) - FaceElementNo - 1] = strtol(tempchar, NULL, 10);	// Store the face element, a vertex index, in the index buffer. *** Order of vertices is reversed  ***
						// --OurIndices[FaceElementNo];											// DirectX index buffer values start with 0, while the equivalent Wavefront .obj face index values start with 1. Decrement it. *** Order of vertices is unchanged ***
						--OurIndices[(In * 3) - FaceElementNo - 1];								// DirectX index buffer values start with 0, while the equivalent Wavefront .obj face index values start with 1. Decrement it. *** Order of vertices is reversed  ***
						fill(tempchar, tempchar + MAX_ELEMENT_SIZE, '\0');						// Clear "tempchar" with the null-terminator.
						nCharCounter = 0;														// Reset "tempchar's" array element pointer.
						ignoreFlag = 0;															// Reset the flag indicator to save characters of the face element, as it's a vertex index.
						++FaceElementNo;														// Prepare to read the next face element.
					}
					else
					{
						// '/' char detected, which indicates a vertex Texture Coordinate Index or a vertex Normal Index follows. We are only interested in a vertex index.

						ignoreFlag = 1;															// Set the flag indicator to not save characters of the face element, because it's not a vertex index.
					}
				}
			}
			// End of line reached, which indicates this line's final complete face element has been read into "tempchar".
			// Convert (reverse) the triangle primitive drawing order from counter-clockwise (used by the Wavefront .obj specification) to clockwise (used by DirectX).
			// "OurIndices[FaceElementNo] = strtol(tempchar, NULL, 10);"						// Store the face element, a vertex index, in the index buffer. *** Order of vertices is unchanged ***
			OurIndices[(In * 3) - FaceElementNo - 1] = strtol(tempchar, NULL, 10);				// Store the face element, a vertex index, in the index buffer. *** Order of vertices is reversed  ***
			// --OurIndices[FaceElementNo];														// DirectX index buffer values start with 0, while the equivalent Wavefront .obj face index values start with 1. Decrement it. Order of vertices is unchanged ***
			--OurIndices[(In * 3) - FaceElementNo - 1];											// DirectX index buffer values start with 0, while the equivalent Wavefront .obj face index values start with 1. Decrement it. Order of vertices is reversed  ***
			fill(tempchar, tempchar + MAX_ELEMENT_SIZE, '\0');									// Clear "tempchar" with the null-terminator.
			nCharCounter = 0;																	// Reset "tempchar's" array element pointer.
			ignoreFlag = 0;																		// Reset the flag indicator to save characters of a face element.
			++FaceElementNo;																	// Prepare to read the next face element. Because it's incremented when the end of the line is reached, its final value is one greater than the final value used.
			// Get another line from the while loop.
		}
	}
	// Close the Wavefront .obj file.
	obj.close();

	// End: 1. Read and store vertex information in OurVertices, and index information in OurIndices.
}