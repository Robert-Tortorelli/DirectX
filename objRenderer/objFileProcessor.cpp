// objRenderer/objFileProcessor.cpp
// Version 3.3
//
// Description:
// This source file reads and parses one or more Wavefront .obj files to define the variables specifying the single 3D object in each file.
//
// Authorship:
// Robert John Tortorelli

// Header Files.
// objRenderer Header File.
#include "objRenderer.h"
// File System Functions.
#include <filesystem>										// File system operations; i.e., manipulate and retrieve information about paths, directories, and files.
// File Stream Functions.
#include <fstream>											// File stream operations; i.e., input/output on file based streams.
// Unordered Map Functions.
#include <unordered_map>									// Unordered map operations; i.e., unordered_map container, which is a hash table-based associative container.

// Using Declarations and Directives.
// Using declarations such as using std::string;   bring one identifier	 in the named namespace into scope.
// Using directives	  such as using namespace std; bring all identifiers in the named namespace into scope.
// Using declarations are preferred to using directives.
// Using declarations and directives must appear after their respective header file includes.
// Consider grouping using statements into a namespace if they're used across multiple files.
using std::fill;
using std::ifstream;
using std::ofstream;
using std::ios;
using std::stof;
using std::string;
using std::vector;
using std::istringstream;
using std::unordered_map;

//***
// External Variable Definitions.
//***

// Define external variables in one and only one source file (this one) and initialize them as needed.
// See the associated header file for declarations and descriptions of these external variables.
vector<OBJECT> OurObjects;

// End: External Variable Definitions.

//***
// Other Declarations.
//***

// OurObjectsi must be global because it is incremented in successive calls to the objParser function to process multiple 3D objects.
int OurObjectsi = -1;										// The index variable OurObjectsi of array variable OurObjects[OurObjectsi].

// WideIndexKey is a type declaration for a packed face element triplet (fv, fvt, fvn) used as a key, variable "key", in the unordered map to deduplicate face element triplets to ensure only unique vertex attribute sets are added to array variable OurVertices when populating it.
// WideIndexKey must be declared 1) before the unordered map is declared (because the unordered map uses WideIndexKey as a key type) and 2) before the function prototypes that reference it.
struct WideIndexKey	{										// Structure WideIndexKey: packs variables fv and fvt into one uint64_t (32 bits each); packs variable fvn into another uint64_t (32 bits).
	std::uint64_t lo;										// Variable fv	in the first 32 bits, variable fvt in the second 32 bits.
	std::uint64_t hi;										// Variable fvn in the first 32 bits, second 32 bits are zero.
};

// End: Other Declarations.

// Function Prototypes.
// Functions called in this source file and defined in another source file.
// N/A
// Functions called in this source file before being defined in this source file.
//   The following functions are called asynchronously. They may be called in any order, and multiple times, or not at all, depending on user interaction and other factors.
//   N/A
//   The following functions are called synchronously.
static int objParser(const string& filename);
WideIndexKey packWideIndexKey(int fv, int fvt, int fvn);
static std::size_t hashWideIndexKey(const WideIndexKey& k);
static bool wideIndexKeysEqual(const WideIndexKey& a, const WideIndexKey& b);

//***
// Function Definitions.
//***

// objFileFinder function: Definition
//   This function finds all Wavefront .obj files in the current directory, and calls the objParser function to open, read, and parse each one.
//   It is called by the WinMain function, and runs one time.
int objFileFinder(void)
{
	// Iterate over all files in the current directory.
	std::filesystem::path exePath = std::filesystem::current_path(); // Get the path to the current executable and store it in variable exePath.
	// The following for statement is a range-based for loop that iterates over each file and directory in the directory specified by exePath.
	//   Type "auto" deduces the type of variable "entry" to be filesystem::directory_entry& (a const reference to a directory entry object) because filesystem::directory_iterator yields elements of type filesystem::directory_entry.
	//   .is_regular_file(), .path().extension(), and .path().filename().string() are member functions of the filesystem::directory_entry object.
	for (const auto& entry : std::filesystem::directory_iterator(exePath))
	{
		if (entry.is_regular_file() && entry.path().extension() == ".obj")
		{
			// A candidate Wavefront.obj file was found in the current directory: The directory entry object is a regular file and its extension is ".obj".

			// Attempt to parse one 3D object's descriptive information from the candidate Wavefront .obj file and use it to define the variables needed to render the 3D object.
			//   Call the objParser function and test whether its return value is zero, indicating success.
			if (int objParserRC = objParser(entry.path().filename().string()); objParserRC == 0)
			{
				// The objParser function successfully parsed one Wavefront .obj file.
				// Examine the next file in the current directory.
			}
			else
			{
				// The objParser function unsuccessfully parsed one Wavefront .obj file.
				// Terminate the objFileFinder function and return to the calling function with a return value indicating an error.
				return objParserRC;
			}
		}
	}
	// End of the for loop. None (error), one (success) or more (success) Wavefront .obj files have been found and parsed successfully.

	// Test whether at least one Wavefront .obj file was found.
	if (!OurObjects.empty())
	{
		// At least one Wavefront .obj file was found.

		// For the current just-parsed Wavefront .obj file(s), reduce the capacity of the dynamic array variables to fit their current size, freeing any unused memory.
		OurObjects.shrink_to_fit();

		// Terminate the objFileFinder function and return to the calling function with a return value indicating success.
		return 0;
	}
	else
	{
		// No Wavefront .obj file was found.
		// Terminate the objFileFinder function and return to the calling function with a return value indicating an error.
		return 3;
	}

	// End: objFileFinder function
}

// objParser function: Definition
//   This function opens, reads, and parses one Wavefront .obj file to define the variables needed to render the 3D object specified in the file.
//   It is called by the objFileFinder function, and runs one time for each Wavefront .obj file found by the objFileFinder function.
static int objParser(const string& filename)
{
	// Declare index variables.
	int OurSubMeshesi = 0;									// The index variable OurSubMeshesi of array variable OurObjects[OurObjectsi].OurSubMeshes[OurSubMeshesi]. *TEST* (Not incremented in this program, so not -1; but will be incremented when parsing multiple submeshes is supported in a single Wavefront .obj file)
	int OurVerticesi = -1;									// The index variable OurVerticesi of array variable  OurObjects[OurObjectsi].OurVertices[OurVerticesi].
	int OurIndicesi = -1;									// The index variable OurIndicesi of array variable   OurObjects[OurObjectsi].OurSubMeshes[OurSubMeshesi].OurIndices[OurIndicesi].

	// Declare variables used to parse the Wavefront .obj file.
	// Intermediate arrays to temporarily store all of the vertex attributes found in the Wavefront .obj file, in preparation for using these vertex attributes to populate array variable OurVertices:
	//   Each intermediate array is a one-dimensional array of structures, where each array element (each structure) contains vertex attributes of a given type, v, vt, or vn, for one vertex.
	//   v  is the intermediate array of structures for geometric vertices,			where each structure contains three floating-point values (x, y, z).
	//   vt is the intermediate array of structures for vertex texture coordinates, where each structure contains two   floating-point values (x, y; usually named U, V in computer graphics).
	//   vn is the intermediate array of structures for vertex normal vectors,		where each structure contains three floating-point values (x, y, z).
	//   Each intermediate array is indexed by a variable, vi, vni, or vti, initialized to -1, that is incremented by 1 each time a new vertex attribute is stored in the intermediate array.
	vector<DirectX::XMFLOAT3> v;  int vi = -1;				// Geometric vertices		  dynamically allocated intermediate array, and index (v[vi]).
	vector<DirectX::XMFLOAT2> vt; int vti = -1;				// Vertex texture coordinates dynamically allocated intermediate array, and index (vt[vti]).
	vector<DirectX::XMFLOAT3> vn; int vni = -1;				// Vertex normal vectors	  dynamically allocated intermediate array, and index (vn[vni]).

	// Declare the face element vertex attribute indices, and the slash character, of the current face element triplet.
	int fv;													// Geometric vertex index (v1, or v2, or v3).
	int fvt;												// Vertex texture coordinate index (vt1, or vt2, or vt3).
	int fvn;												// Vertex normal vector index (vn1, or vn2, or vn3).
	char slash = '/';										// The slash character ('/') is a delimiter used to parse the face element.

	// unordered_map placed inside function objParser supports per-object (per-Wavefront .obj file) deduplication of face element triplets to ensure only unique vertex attribute sets are added to array variable OurVertices when populating it.
	// unordered_map placed globally				  would mix face element triplets from different objects that, in this program, do not share face element triplets nor vertex attribute sets, thereby causing rendering errors.
	unordered_map<											// Maps a packed face element triplet (variable of type WideIndexKey) to the corresponding index (variable of type DWORD) in OurVertices.
		WideIndexKey,										// Key type:	WideIndexKey represents a packed face element triplet.
		DWORD,												// Value type:	DWORD represents the index of array variable OurVertices.
		std::size_t(*)(const WideIndexKey&),				// Function pointer type for the key hash function.		Equivalent to decltype(&hashWideIndexKey)	as &hashWideIndexKey	is a pointer to that function, and decltype(&hashWideIndexKey)	 is the function pointer type.
		bool(*)(const WideIndexKey&, const WideIndexKey&)	// Function pointer type for the key-equality function. Equivalent to decltype(&wideIndexKeysEqual)	as &wideIndexKeysEqual	is a pointer to that function, and decltype(&wideIndexKeysEqual) is the function pointer type.
	> faceElementTripletToIndex(							// The unordered_map object. It invokes the functions hashWideIndexKey and wideIndexKeysEqual in statements such as faceElementTripletToIndex.find() and faceElementTripletToIndex.operator[](key)) during parsing.
		// faceElementTripletToIndex.reserve(value) can be used after the unordered_map is defined to refine the initial bucket count to optimize memory allocation.
		// Refining the initial bucket count eliminates the costly CPU overhead of dynamic array reallocations and element migrations during insertions.
		// To optimize for a specific dataset size, the integer value should be based on the number of unique face element triplets in the Wavefront .obj file currently being parsed.
		// See the OneNote section C++ page 'reserve() Storage Efficiently' for details.
		24,													// Initial bucket count for the unordered_map. 24 is the number of unique face element triplets in a cube, and the number of elements in the deduplicated dynamic array variable OurVertices.
		hashWideIndexKey,									// Key hash function.
		wideIndexKeysEqual);								// key-equality function.

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
	// - Vertex attribute statements are listed first, and will therefore be parsed before any face element statements.
	//   The order of the face element statements determines the order in which the triangles must be drawn. This order is important when dealing with overlapping triangles, as the later triangles will be drawn on top of the earlier ones. Face element statements are parsed in this order.
	//   All other statements are ignored.
	//
	// - Multiple objects are not supported by the objParser function within a single Wavefront .obj file.
	//   This is because the objParser function as written requires that no vertex attributes follow face elements, and this happens when one object is defined after another.
	//
	// - No spaces are permitted before or after a slash ('/').
	//
	// - Statements can start in any column.
	//
	// - Statements can be logically joined with the line continuation character ( \ ) at the end of a line. (This is *not* supported by this program)
	//
	// Open the Wavefront .obj file for input.
	obj.open(filename, ios::in);
	// Check whether the Wavefront .obj file opened successfully. (!obj), (!obj.is_open()), and (obj.fail()) all indicate an error opening the file.
	if (!obj)												// If not (!) successful (obj) then:
	{
		// Cannot open the Wavefront .obj file.

		// Terminate the objParser function and return to the calling function with a return value indicating an error.
		return 1;
	}
	// The Wavefront .obj file opened successfully.

	// Prepare for the new object in the Wavefront .obj file.
	OurObjects.emplace_back();								// Add a new element to the dynamic array variable OurObjects.
	++OurObjectsi;											// Increment for each Wavefront .obj file.
	// Prepare for the first submesh in the Wavefront .obj file. *TEST* Processing will change when parsing multiple submeshes is supported in a single Wavefront .obj file.
	OurObjects[OurObjectsi].OurSubMeshes.emplace_back();	// Add a new element to the dynamic array variable OurSubMeshes.

	// Parse the Wavefront .obj file.
	while (getline(obj, stringtext))						// Read an entire statement, from the input file stream object obj, into the string variable stringtext. At eof getline becomes false and the while loop is exited.
	{
		// Declare variables used to parse the current statement of the Wavefront .obj file.
		// Declaring variables inside the while loop ensures they are reset at the start of each iteration, which is necessary when reading a new line from the file.
		istringstream lineStream(stringtext);				// Input stream object lineStream of class istringstream supports parsing the current statement of the Wavefront .obj file using the >> operator, which extracts formatted data from the object lineStream.
		string type;										// Variable type contains the Wavefront .obj file statement type: #, v, vt, vn, f, usemtl.

		lineStream >> type;									// The >> operator extracts the next value from the lineStream input stream object and stores it in the variable type. Here, the next value is the first word of the statement, which indicates the type of the statement.

		if (type == "v")
		{
			// The statement read is a geometric vertex statement. Parse and store it in dynamically allocated intermediate array variable v.
			vi++;											// Increment the geometric vertex index.
			v.emplace_back();								// Add a new element to array variable v.
			lineStream >> v[vi].x >> v[vi].y >> v[vi].z;	// The >> operator extracts the next three values from the lineStream input stream object and stores them in the intermediate array variables v[vi].x, then v[vi].y, then v[vi].z.
		} else if (type == "vt")
		{
			// The statement read is a vertex texture coordinate statement. Parse and store it in dynamically allocated intermediate array variable vt.
			vti++;											// Increment the vertex texture coordinate index.
			vt.emplace_back();								// Add a new element to array variable vt.
			lineStream >> vt[vti].x >> vt[vti].y;			// The >> operator extracts the next two values from the lineStream input stream object and stores them in the intermediate array variables vt[vti].x, then vt[vti].y.
		} else if (type == "vn")
		{
			// The statement read is a vertex normal vector statement. Parse and store it in dynamically allocated intermediate array variable vn.
			vni++;											// Increment the vertex normal vector index.
			vn.emplace_back();								// Add a new element to array variable vn.
			lineStream >> vn[vni].x >> vn[vni].y >> vn[vni].z;	// The >> operator extracts the next three values from the lineStream input stream object and stores them in the intermediate array variables vn[vni].x, then vn[vni].y, then vn[vni].z.
		} else if (type == "f")
		{
			// The statement read is a face element statement, therefore all vertex attribute statements in the Wavefront .obj file have previously been parsed and stored in the intermediate array variables v, vt, and vn.
			// This program requires that all three types of vertex attribute statements be specified in the Wavefront .obj file.
			// Check if any of the intermediate arrays v, vt, or vn is empty, i.e., if geometric vertex statements, vertex texture coordinate statements, or vertex normal vector statements are not specified in the Wavefront .obj file.
			// This can occur when the Wavefront .obj file is valid but only contains geometric vertex statements and face element statements, but not vertex texture coordinate statements and vertex normal vector statements.
			if (vi == -1 || vti == -1 || vni == -1)			// If vi or vti or vni is -1, then the Wavefront .obj file does not contain the required vertex attribute statements.
			{
				// Cannot process the Wavefront .obj file.

				// Terminate the objParser function and return to the calling function with a return value indicating an error.
				return 2;
			}
			// The Wavefront .obj file does contain the required vertex attribute statements.

			// Assemble the three vertex attribute sets for each triangle specified by the current face element statement, and store them in three elements of array variable OurVertices, while also populating three elements of array variable OurIndices with the indices of array variable OurVertices corresponding to the three face element triplets in the current face element statement.
			// This is accomplished by the following steps, which are performed for each face element statement. These steps include, as described, adjusting the vertex attributes from the Wavefront .obj file format to the DirectX format, adjusting the face element vertex attribute indices from the Wavefront .obj file format to the C++ array standard, and adjusting the drawing order of triangle vertices from counter-clockwise (Wavefront .obj file format) to clockwise (DirectX format).

			// Parse each of the three triplets (e.g., triplet v1/vt1/vn1) in the face element statement (f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3).
			//   Each of the three triplets (e.g., triplet v1/vt1/vn1) in the face element statement consists of three positive numbers referring to three vertex attribute statements specifying one triangle vertex, which together comprise one vertex attribute set.
			//   This vertex attribute set, if unique (new; not previously seen), is stored in the next sequential element of array variable OurVertices.
			//   This results in copying unique sets of vertex attributes, from the intermediate array variables v, vt, and vn to array variable OurVertices, in the order that the face element triplets are first encountered across face element statements in the Wavefront .obj file, after adjusting each vertex attribute from the Wavefront .obj file format to the DirectX format.
			//
			// Explanation:
			// Vertex attributes have been stored in the intermediate arrays v, vt, and vn in the same order in which they appear in the Wavefront .obj file.
			// Therefore, the first geometric vertex attribute (v) in the Wavefront .obj file is stored in v[0].
			// Each triplet in a face element statement (f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3), for example 3/2/2 in f 3/2/2 8/4/2 4/5/2, refers to one vertex attribute set for one vertex of a triangle, and consists of three positive numbers referring to the order of vertex attribute statements in the Wavefront .obj file:
			// - The first	number	(e.g., 3) is the geometric vertex index				(v1,  or v2,  or v3)	that refers to the order of geometric vertex statements				in the Wavefront .obj file. For example, 3 refers to the third 	geometric vertex statement			in the Wavefront .obj file, which is stored in v[2]		because of the decrementing by 1 described below.
			// - The second number	(e.g., 2) is the vertex texture coordinate index	(vt1, or vt2, or vt3)	that refers to the order of vertex texture coordinate statements	in the Wavefront .obj file. For example, 2 refers to the second vertex texture coordinate statement	in the Wavefront .obj file, which is stored in vt[1]	because of the decrementing by 1 described below.
			// - The third	number	(e.g., 2) is the vertex normal vector index			(vn1, or vn2, or vn3)	that refers to the order of vertex normal vector statements			in the Wavefront .obj file. For example, 2 refers to the second vertex normal vector statement		in the Wavefront .obj file, which is stored in vn[1]	because of the decrementing by 1 described below.
			// Each triplet in a face element statement (f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3), for example 3/2/2 in f 3/2/2 8/4/2 4/5/2, is parsed into the index variables fv (= 3), fvt (= 2), and fvn (= 2) and then immediately decremented by 1, making 0 the first index as is standard for C++ arrays.
			// fv, fvt, and fvn are the face element vertex attribute indices used to access the intermediate arrays v[fv], vt[fvt], and vn[fvn].
			//
			// As a result, v[fv], vt[fvt], and vn[fvn] store the corresponding vertex attributes referenced by the current face triplet:
			// Specifically, the (fv + 1)-th v statement, the (fvt + 1)-th vt statement, and the (fvn + 1)-th vn statement in the Wavefront .obj file.

			// Add three new elements to the dynamic array variable OurIndices, which will be populated with the indices of array variable OurVertices corresponding to the three face element triplets in the current face element statement.
			OurObjects[OurObjectsi].OurSubMeshes[OurSubMeshesi].OurIndices.resize(OurObjects[OurObjectsi].OurSubMeshes[OurSubMeshesi].OurIndices.size() + 3);	// Resize the dynamic array variable OurIndices to add three new elements to it. This is the only OurIndices.resize() statement, executed once for each face element triplet in all face element statements.
			OurIndicesi += 3;																																	// Update the index variable OurIndicesi of array variable OurIndices[OurIndicesi].

			for (int i = 0; i <= 2; i++)																														// Parse each of the three triplets.
			{
				// Parse the current triplet (v/vt/vn) in the face element statement, storing the face element vertex attribute indices.
				lineStream >> fv >> slash >> fvt >> slash >> fvn;																								// The >> operator extracts the next three values, delimited by the slash character ('/'), from the lineStream input stream object and stores them in the variables fv, then fvt, then fvn.

				// Decrement the face element vertex attribute indices by 1 to adjust them from the Wavefront .obj file format to the C++ array standard, which uses 0 as the first index.
				// The face element vertex attribute indices are decremented by 1 because in C++ index variables vi, vti, and vni of the vertex attribute intermediate array variables v[vi], vt[vti], and vn[vni] index from 0, while the Wavefront .obj file format indices index from 1.
				fv--; fvt--; fvn--;

				// The candidate vertex attribute set is comprised of v[fv], vt[fvt], and vn[fvn].
				// Test if the candidate vertex attribute set is unique, i.e., has this vertex attribute set been previously found in the Wavefront .obj file and stored in array variable OurVertices?
				// This is done to avoid storing duplicate (non-unique) sets of vertex attributes in array variable OurVertices.
				//
				// The uniqueness of a given candidate vertex attribute set can be determined directly by comparing a candidate vertex attribute set with existing vertex attribute sets in the array variable OurVertices, or by whether the associated face element triplet is unique.
				// This is possible because although all geometric vertices are unique, and all vertex texture coordinates are unique, and all vertex normal vertices are unique in a Wavefront .obj file, a combination (set) of these three vertex attributes can be non-unique.
				// (If geometric vertices could be non-unique, or vertex texture coordinates could be non-unique, or vertex normal vertices could be non-unique, then two face element triplets comprised of different indices might mistakenly appear to be different but could still point to an identical (non-unique) vertex attribute set)
				// In this program, the uniqueness of the candidate vertex attribute set is determined using an unordered_map keyed by a packed face element triplet value (struct WideIndexKey). The map hashes that key internally to locate a bucket, then compares the candidate key with existing keys in that bucket to determine whether the candidate key is unique.
				//
				// Adjust the drawing order of triangle vertices in the current face element statement from counter-clockwise (Wavefront .obj file format) to clockwise (DirectX format).
				// When rendering an object with DirectX, the drawing order of the vertices (each defined by a vertex attribute set) that are stored in each element of array variable OurVertices is determined by the values of the sequential elements of array variable OurIndices.
				// The values of these elements are the indices of array variable OurVertices, which would also be sequential, were it not for,
				// - The need to avoid storing duplicate (non-unique) sets of vertex attributes in array variable OurVertices as discussed above.
				// - The need to adjust the drawing order from counter-clockwise (Wavefront .obj file format) to clockwise (DirectX format).
				//   Adjusting the drawing order can be done by reversing the order of processing the second and third vertex attribute set of each triangle's three vertices, which is equivalent to changing the order by which the elements of array variable OurVertices are referenced by the elements of array variable OurIndices:
				//   - OurVertices[OurVerticesi] is OurVertices[OurIndices[OurIndicesi]], where instead of OurIndices[OurIndicesi] = 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 etc. the drawing order is reversed by reordering OurIndices[OurIndicesi] = 0, 2, 1, 3, 5, 4, 6, 8, 7, 9, 11, 10, etc. (This is done in this program)
				//   - Alternatively, the elements of array variable OurVertices could be reordered while keeping the elements of array variable OurIndices[OurIndicesi] = 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 etc. (This is not done in this program)
				// At this point in this program, OurIndicesi is the index of the last of 3 new elements of array variable OurIndices.
				// Variable VertexDrawOrderAdjust is used to reverse the order of processing the second and third vertex attribute set of each triangle's three vertices.
				// Index [OurIndicesi - VertexDrawOrderAdjust] in OurIndices[OurIndicesi - VertexDrawOrderAdjust] calculates the adjusted index of array variable OurIndices that is used to store the index of array variable OurVertices. During rendering, the GPU processes the elements of array variable OurIndices (as the index buffer) sequentially, thereby referencing the elements of array variable OurVertices in the adjusted order, and drawing triangle vertices in clockwise (DirectX format) rather than counter-clockwise (Wavefront .obj file format) order.
                // A ternary conditional expression, i.e., (condition) ? (value_if_true) : (value_if_false) is used to calculate the value of variable VertexDrawOrderAdjust.
				// In the three iterations of this "for i loop" VertexDrawOrderAdjust = (i == 0) ? 2 : (i - 1) successively evaluates to:                                2,           0 (= 1 - 1), 1 (= 2 - 1).
				// Before each entry to this "for i loop" index variable OurIndicesi is incremented by 3 to become 2, 5, 8, etc.
				// Therefore, in the first entry to this "for i loop" (OurIndicesi = 2) its three iterations adjust the indices of array variable OurIndices to 0 (= 2 - 2), 2 (= 2 - 0), 1 (= 2 - 1).
				int VertexDrawOrderAdjust = (i == 0) ? 2 : (i - 1);
				//
				// Determine the uniqueness of the candidate vertex attribute set.
				// Build the unordered_map key, variable "key", for the current candidate vertex attribute set using the current face element triplet.
				const WideIndexKey key = packWideIndexKey(fv, fvt, fvn);
				// Use variable "key", to look for the current face element triplet in the map to see if it already exists.
				auto it = faceElementTripletToIndex.find(key); // Variable "it" is an iterator pointing to the matching key/value pair found in the map, or to end() if not found, when attempting to find the search key (variable "key").
				DWORD index;								   // Variable "index" will store the index of an existing vertex attribute set in variable OurVertices, if the current face element triplet is found in the map.
				if (it != faceElementTripletToIndex.end())
				{
					// The current face element triplet already exists in the map.
					// Therefore, the associated index of an existing vertex attribute set in variable OurVertices is available to be retrieved.
					// Because the associative container stores key/value pairs, 'first' (the term referencing the first stored value in the pair) is the key and 'second' (the term referencing the second stored value in the pair) is the associated index of the existing vertex attribute set in variable OurVertices.
					// "index = it->second" copies to variable "index" the associated index of the existing vertex attribute set in variable OurVertices.
					index = it->second;						// The index of the existing vertex attribute set in variable OurVertices is now stored in variable "index".
					// The element of array variable OurIndices is adjusted by VertexDrawOrderAdjust to adjust the drawing order of triangle vertices from counter-clockwise (Wavefront .obj file format) to clockwise (DirectX format).
     				OurObjects[OurObjectsi].OurSubMeshes[OurSubMeshesi].OurIndices[OurIndicesi - VertexDrawOrderAdjust] = index;
				}
				else
				{
					// The candidate vertex attribute set is unique, so a new vertex attribute set is created and stored in array variable OurVertices.
					//
					// Create a new element of dynamic array variable OurVertices, and assign the candidate vertex attribute set to it.
					OurObjects[OurObjectsi].OurVertices.emplace_back();													// Add a new element to dynamic array variable OurVertices. This is the only OurVertices.emplace_back() statement, executed once for each unique vertex attribute set in all face element statements.
					OurVerticesi++;																						// Update the index variable OurVerticesi of array variable OurVertices[OurVerticesi].

					// As the candidate vertex attribute set is unique, its associated key/value pair does not exist in the map and is added to the map.
					// If the same candidate vertex attribute set is encountered again in a subsequent face element triplet, it will be recognized as non-unique and the existing vertex attribute set in variable OurVertices will be used instead of creating a new one.
					// The key/value pair is comprised of the packed face element triplet (the key) and the index of the associated vertex attribute set in variable OurVertices (the value).
					faceElementTripletToIndex.operator[](key) = static_cast<DWORD>(OurVerticesi);						// operator[] returns a reference to the value, a DWORD, that is mapped to key. Because the candidate vertex attribute set is unique the key does not exist in the map, and operator[] inserts a new map entry in the map bucket, containing a new key/value pair, using key with value initialized to a DWORD of 0. The assignment "=" then stores variable OurVerticesi as the mapped value, replacing the initial value of 0.

					OurObjects[OurObjectsi].OurVertices[OurVerticesi].GeometricVertex.x =		  v[fv].x;
					OurObjects[OurObjectsi].OurVertices[OurVerticesi].GeometricVertex.y =		  v[fv].y;
					OurObjects[OurObjectsi].OurVertices[OurVerticesi].GeometricVertex.z =		  v[fv].z * -1.0f;		// Invert the geometric vertex's Z coordinate,			to adjust it from the Wavefront .obj file format to the DirectX format.

					OurObjects[OurObjectsi].OurVertices[OurVerticesi].VertexTextureCoordinate.x = vt[fvt].x;
					OurObjects[OurObjectsi].OurVertices[OurVerticesi].VertexTextureCoordinate.y = 1.0f - vt[fvt].y;		// Invert the vertex texture coordinate's V coordinate, to adjust it from the Wavefront .obj file format to the DirectX format.

					OurObjects[OurObjectsi].OurVertices[OurVerticesi].VertexNormalVector.x =	  vn[fvn].x;
					OurObjects[OurObjectsi].OurVertices[OurVerticesi].VertexNormalVector.y =	  vn[fvn].y;
					OurObjects[OurObjectsi].OurVertices[OurVerticesi].VertexNormalVector.z =	  vn[fvn].z * -1.0f;	// Invert the vertex normal vector's Z coordinate,		to adjust it from the Wavefront .obj file format to the DirectX format.

					// One of the three new elements of array variable OurIndices is assigned the index (the value of variable OurVerticesi) of the element of array variable OurVertices[OurVerticesi] which contains the new vertex attribute set stored in array variable OurVertices.
					OurObjects[OurObjectsi].OurSubMeshes[OurSubMeshesi].OurIndices[OurIndicesi - VertexDrawOrderAdjust] = OurVerticesi;	// The element of array variable OurIndices is adjusted by VertexDrawOrderAdjust to adjust the drawing order of triangle vertices from counter-clockwise (Wavefront .obj file format) to clockwise (DirectX format).
				}
				// *TEST*
				// The following two lines must be placed inside the loop that processes each submesh, after all indices for the current submesh have been added.
				// For the current just-parsed submesh, reduce the capacity of the dynamic array variables to fit their current size, freeing any unused memory.
				// OurObjects[OurObjectsi].OurSubMeshes[OurSubMeshesi].OurIndices.shrink_to_fit();
			}
		} else continue;																												// The statement read is not a geometric vertex, vertex texture coordinate, vertex normal vector, or face element statement. Ignore it and continue.
	}
	// End of the while loop. The entire Wavefront .obj file specifying one object has been parsed.

	// Close the Wavefront .obj file.
	obj.close();

	// Assign the total number of array elements in array variable OurVertices to the variable VertexAttributeSetsTotal.
	OurObjects[OurObjectsi].VertexAttributeSetsTotal = static_cast<int>(OurObjects[OurObjectsi].OurVertices.size());
	// Assign the total number of array elements in array variable OurIndices to the variable IndicesTotal.
	OurObjects[OurObjectsi].OurSubMeshes[OurSubMeshesi].IndicesTotal = static_cast<int>(OurObjects[OurObjectsi].OurSubMeshes[OurSubMeshesi].OurIndices.size());

	// Write a message to the diagnostic log file logging the deduplication ratio for the current object: (variable VertexAttributeSetsTotal / variable IndicesTotal)
	// If the deduplication ratio equals 1 (its maximum value) then either no deduplication has been done or the object is simple and contains no duplicate vertices.
	// Variable IndicesTotal will never be zero, thereby avoiding division by zero when calculating the deduplication ratio.
	ofstream log("objFileDiagnostics.log", ios::trunc);		// Define the output file stream object representing the diagnostic file (ios::trunc replaces existing content).
	if (!log)
	{
    	// The diagnostic log file could not be opened for writing. Continue without logging.
	}
	else
	{
		// The diagnostic log file has been successfully opened for writing.
		log << "Deduplication ratio: "
			<< static_cast<double>(OurObjects[OurObjectsi].VertexAttributeSetsTotal) /
								   OurObjects[OurObjectsi].OurSubMeshes[OurSubMeshesi].IndicesTotal
			<< '\n';
		if (!log)
		{
			// The diagnostic log file could not be written to. Continue without logging.
		}
		// Close the diagnostic log file.
		log.close();
	}

	// For the current just-parsed object, reduce the capacity of the dynamic array variables to fit their current size, freeing any unused memory.
	OurObjects[OurObjectsi].OurVertices.shrink_to_fit();
	OurObjects[OurObjectsi].OurSubMeshes.shrink_to_fit();

	// Terminate the objParser function and return to the calling function with a return value indicating success.
	return 0;

	// End: objParser function
}

// packWideIndexKey function: Definition
//   This function builds the unordered_map key, variable "key".
//   It supports deduplication of face element triplets to ensure only unique vertex attribute sets are added to array variable OurVertices when populating it.
WideIndexKey packWideIndexKey(int fv,
	int fvt,
	int fvn)
{
	return {
		std::uint64_t(std::uint32_t(fv)) | (std::uint64_t(std::uint32_t(fvt)) << 32), // WideIndexKey::hi (first 32 bits are fv,  second 32 bits are fvt)
		std::uint64_t(std::uint32_t(fvn))											  // WideIndexKey::lo (first 32 bits are fvn, second 32 bits are zero)
	};

	// End: packWideIndexKey function
}

// hashWideIndexKey function: Definition
//   This key hash function is used by unordered_map to compute the hash of a key (variable "key") to select a bucket in which to store and find the key and its associated value.
//   It supports deduplication of face element triplets to ensure only unique vertex attribute sets are added to array variable OurVertices when populating it.
static std::size_t hashWideIndexKey(const WideIndexKey& k)
{
	std::size_t seed = 0, h;
	h = std::hash<std::uint64_t>{}(k.lo); seed ^= h + 0x9e3779b9 + (seed<<6) + (seed>>2);
	h = std::hash<std::uint64_t>{}(k.hi); seed ^= h + 0x9e3779b9 + (seed<<6) + (seed>>2);
	return seed;

	// End: hashWideIndexKey function
}

// wideIndexKeysEqual function: Definition
//   This key-equality function is used by unordered_map to compare keys whenever the map needs to determine whether the search key (variable "key") matches any existing stored key in a bucket and to compare keys within a bucket to distinguish them.
//   It supports deduplication of face element triplets to ensure only unique vertex attribute sets are added to array variable OurVertices when populating it.
static bool wideIndexKeysEqual(const WideIndexKey& a,
	const WideIndexKey& b)
{
	return a.lo == b.lo && a.hi == b.hi;

	// End: wideIndexKeysEqual function
}