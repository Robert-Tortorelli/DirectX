// The C++ statements derived from the following pseudocode are in the section of code preceding assignments to OurObjects, i.e., preceding objFileProcessor.cpp:245 '} else if (type == "f")' (See line 250's "*TEST*").

if statement EQ "mtllib"
	// A mtllib statement has been found.
	objFileState = 1xxx // Record finding a mtllib statement.
	if objFileState NEQ 1000 // If statements other than mtllib have previously been found the Wavefront .obj file is malformed. All mtllib statements must appear before any other statements.
		write error
		return RC = error
		// Return from the function due to the error.
	// Parse the mtllib statement for one or more MTL library filenames and assign each filename to an element of array variable mtllib.
	//   mtllib[mtllibi] returns a MTL library filename.
	mtllib[++mtllibi] = parsestatement(mtllib statement)

if statement EQ "usemtl"
	// A usemtl statement has been found.
	// A new submesh has been found.
	objFileState = x1xx // Record finding a usemtl statement.
	if objFileState EQ 01xx // If no mtllib statements have previously been found the Wavefront .obj file is malformed. All mtllib statements must appear before any other statements.
		write error
		return RC = error
		// Return from the function due to the error.
	// Parse the usemtl statement for its material description name and assign this name to an element of array variable usemtl.
	//   usemtl[OurSubMeshesi] returns the material description name associated with the current submesh.
	usemtl[++OurSubMeshesi] = parsestatement(usemtl statement) // ++OurSubMeshesi is the index of the new submesh.
	//
	// Parse all material description statements (in all MTL library files), which define the start of a new material description, and add each key/value pair (material description name/map_Kd texture image filename) to the newmtl unordered_map.
	//   newmtl[a material description name] returns the map_Kd texture image filename associated with the material description name. newmtl[usemtl[OurSubMeshesi]] is equivalent to newmtl[a material description name].
	//   Parsing all material description statements (in all MTL library files) is a one-time operation, performed when the first usemtl statement is found in the Wavefront .obj file.
	//   Finding any non-mtllib statement indicates all mtllib statements have been found. A usemtl statement should be the first non-mtllib statement.
	if newmtl[*] is empty // If no newmtl statements have yet been parsed then perform the one-time operation of parsing all material description statements (in all MTL library files).
		Process mtllib[*] // Read the next statement from the next MTL library file until all statements have been read.
			if statement EQ "newmtl"
				// A newmtl statement has been found. Populate the newmtl unordered_map with the material description name and its associated map_Kd texture image filename.
				newmtl[key] = parsestatement(newmtl statement) // Parse the material description name.
				newmtl[value] = parsestatement(map_Kd statement) // Parse the map_Kd texture image filename.
				// Continue processing the next statement in the MTL library file.
		// End of processing the MTL library file.

// The C++ statements derived from the following pseudocode are in the section of code with assignments to OurObjects, i.e., after objFileProcessor.cpp:245 '} else if (type == "f")' (See line 250's "*TEST*").

// Assign the texture image filename for the current submesh.
OurObjects[OurObjectsi].OurSubMeshes[OurSubMeshesi].OurTexture = newmtl[usemtl[OurSubMeshesi]] // Assign the texture image filename associated with the material description name to the current submesh.