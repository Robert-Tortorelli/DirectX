// objReaderEnum
// Version 3.3
//
// Description:
// Read and parse all 3D object's descriptive information from their Wavefront .obj files and use it to define the variables needed to render these 3D objects.
//
// Implemented:
//   The following return values are passed from the indicated function to the caller:
//	 RC 0:					(all functions)			Normal termination.
//	 RC 1:					objReader function:		Error opening the Wavefront .obj file.
//	 RC 2:					objReader function:		Error in	  the Wavefront .obj file: Required vertex attributes are missing.
//
// Authorship:
// Robert John Tortorelli

// objReader Header File for Wavefront .obj file I/O.
#include "objReader.h"

#include <print>											// Include the print      header file for input and output operations, such as std::println. This requires C++23 or later.
#include <filesystem>										// Include the filesystem header file for file system	   operations, such as std::filesystem.

// objReaderEnum function: Definition
int objReaderEnum(void)
{
	// Get the path to the current executable and store it in variable exePath.
	std::filesystem::path exePath = std::filesystem::current_path();

	// Iterate over all files in the current directory.
	// The following for statement is a range-based for loop that iterates over each file and directory in the directory specified by exePath.
	//   The auto keyword tells the compiler to automatically deduce the type of the variable entry from its initializer.
	//   In the following statement auto deduces the type of variable entry to be a constant reference to a std::filesystem::directory_entry object,
	//   because directory_iterator yields elements of type std::filesystem::directory_entry.
	//   This allows the code to be more concise and maintainable, especially when dealing with complex or verbose types.
	//   .is_regular_file(), .path().extension(), and .path().filename().string() are member functions of the entry object.
	for (const auto& entry : std::filesystem::directory_iterator(exePath))
	{
		if (entry.is_regular_file() && entry.path().extension() == ".obj")
		{
			// The entry is a regular file and its extension is ".obj".

			// Read and parse one 3D object's descriptive information from one Wavefront .obj file and use it to define the variables needed to render the 3D object.
			//   Call the objReader function and test whether its return value is nonzero, indicating an error.
			if (int objReaderRC = objReader(entry.path().filename().string()); objReaderRC != 0)
			{
				// The objReader function terminated abnormally. Terminate the InitGraphics function with the return value of the objReader function.
				return objReaderRC;
			}
			// The objReader function terminated normally.

			//*TEST* Next pass the return code of the objReader function through this calling program, and update associated comments (such as the various programs Description sections) as needed.
			//*TEST* Next assign the variables needed to render the one 3D object to the new array of all such information, then modify objRenderer to use this new array to render all the 3D objects.

		}
	}

	// Return to the calling program with a return code indicating success.
	return 0;
}