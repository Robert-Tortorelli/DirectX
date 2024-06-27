// objRenderer
// Version 3.2
//
// Description
// The project objRenderer parses a single 3D object's description from a Wavefront .obj file, and renders that object one or more times.
// Implemented:
// - Object geometry
// - Light
// - Texture
// Change Log:
// - Version 3.2 XXXXX XX, 202X
//   Added support in X for X.
//
// This program, objRenderer, renders the object.
// This program is a C++ Windows Desktop program using the Windows (Win32) API and the DirectX 11 API.
//
// All variables and functions coded in HLSL (.hlsl files) are stored in GPU memory.
// All variables and functions coded in C++ (.cpp files) are stored in CPU memory.
// 
// Return codes
// RC 0: All functions:		 Normal
// RC 1: objReader function: Abnormal: Error opening the Wavefront .obj file.
// RC 2: objReader function: Abnormal: Error in		 the Wavefront .obj file: Required vertex attributes are missing.
//
// Authorship
// This program is based on "DirectX 11 Win32 Desktop: Direct3D: Moving to 3D: Lesson 3: Simple Modeling" and earlier lessons by Chris Hanson (http://DirectXTutorial.com).
// All defects in this program are solely the responsibility of Robert John Tortorelli.

//***
// Global Declarations.
//***

// Terminology
//   Textures:
//   The word texture has two distinct meanings in DirectX programming. Both meanings are used in this program.
//
//   1. The terms texture, texture image, and image texture refers to a computer graphics image applied to the surface of a 3D object.
//      A texel (a texture element or texture pixel) is the fundamental unit of a texture map.
//      Textures are represented by arrays of texels representing the texture space, just as other images are represented by arrays of pixels.
//      A mipmap (also MIP map) or pyramid is a pre-calculated, optimized sequences of images, each of which is a progressively lower resolution representation of the previous image.
//      The CreateWICTextureFromFile function is one way to create a texture from an image file, such as a .png file.
//
//   2. The terms texture buffer, texture array, and texture interface refer to DirectX programming constructs.
//      These terms are distinct from the term texture images defined above.

// objReader Header File for Wavefront .obj file I/O.
#include "objReader.h"

// Windows API Header File.
#include <windows.h>										// The Windows API (Win32 API) header file enables you to create 32-bit and 64-bit programs. It includes declarations for both Unicode and ANSI versions of the API. For more information, see Unicode in the Windows API.

//***
// Global Function Declarations.
// Function prototypes for functions defined in this program.
//***
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
int InitD3D(HWND hWnd);
void InitPipeline(void);
int InitGraphics(void);
void RenderFrame(void);
void CleanD3D(void);

// Note:
// DirectX member functions associated with a stage of the graphics pipeline usually have names prefixed with two capital letters identifying the related stage, e.g., the OMSetRenderTargets member function is related to the output-merger stage of the graphics pipeline.
// In addition to these DirectX member functions, other code may also be associated with a stage of the graphics pipeline (the code's comments will indicate this).
// In this program:
//   Member Function:								Graphics Pipeline Stage:		Member Function appears in this Internal Function:
//   ID3D11DeviceContext::OMSetRenderTargets		Output-Merger					InitD3D()
//   ID3D11DeviceContext::RSSetViewports			Rasterizer						InitD3D()
//   ID3D11DeviceContext::VSSetShader				Vertex Shader					InitPipeline()
//   ID3D11DeviceContext::PSSetShader				Pixel Shader					InitPipeline()
//   ID3D11Device::CreateInputLayout				Input-Assembler					InitPipeline()
//   ID3D11DeviceContext::IASetInputLayout			Input-Assembler					InitPipeline()
//   ID3D11DeviceContext::VSSetConstantBuffers		Vertex Shader					InitPipeline()
//   ID3D11DeviceContext::IASetVertexBuffers		Input-Assembler					RenderFrame()
//   ID3D11DeviceContext::IASetIndexBuffer			Input-Assembler					RenderFrame()
//   ID3D11DeviceContext::IASetPrimitiveTopology	Input-Assembler					RenderFrame()

// End: Global Function Declarations.

//***
// DirectX Global Declarations.
//***

// Pragma Directives.
// The #pragma directives offer machine-specific and operating system-specific features while retaining overall compatibility with the C and C++ languages.
// Direct3D Library files.
#pragma comment (lib, "d3d11.lib")							// Direct3D DirectX 11 Library.
#pragma comment(lib, "d3dcompiler.lib")						// Direct3D Compiler Library. Needed for D3DCompileFromFile, which compiles shaders.

// Direct3D Header Files.
#include <d3d11.h>											// This header is used by Direct3D 11 Graphics.
#include <d3dcompiler.h>									// Needed by D3DCompileFromFile, which compiles shaders.
#include <wictextureloader.h>								// DirectXTK library module WICTextureLoader is a Direct3D 2D texture loader using Windows Imaging Component to load, resize, and format convert a supported bitmap and then create a 2D texture from it.

// Using Declarations and Directives.
// Using declarations such as using std::string;   bring one identifier	 in the named namespace into scope.
// Using directives	  such as using namespace std; bring all identifiers in the named namespace into scope.
// Using declarations are preferred to using directives.
// Using declarations and directives must appear after their respective header file includes.
using namespace DirectX;

// Defines.
// Define the screen resolution of the client area.
#define SCREEN_WIDTH  800									// Visual Studio recommends converting macros to expressions: constexpr auto SCREEN_WIDTH = 800;
#define SCREEN_HEIGHT 600									// Visual Studio recommends converting macros to expressions: constexpr auto SCREEN_HEIGHT = 600;

// DirectX Global Interface Declarations.
IDXGISwapChain* swapchain;									// The pointer to the swap chain interface.			The swap chain interface implements one or more surfaces (image-data objects) for storing rendered data before presenting it to an output. It is the series of buffer resources (front buffer, back buffers) which take turns being rendered on.
ID3D11Device* dev;											// The pointer to the device interface.				A device is the virtual representation of the computer's display adapter. It is used to access video memory and create other Direct3D COM objects, such as graphics and special effects.
ID3D11DeviceContext* devcon;								// The pointer to the device context interface.		A device context is responsible for managing the graphics pipeline. It control the rendering sequence and the process that translates 3D models into the final 2D image that appears on the screen.

ID3D11Texture2D* pDepthBuffer;								// The pointer to a 2D texture interface.			A 2D texture interface manages texel data, which is structured memory. In this case the 2D texture that serves as the depth-stencil surface.
ID3D11DepthStencilView* depthbuffer;						// The pointer to the depth-stencil view interface.	A depth-stencil view interface accesses a texture resource (via the depth-stencil surface interface, pDepthBuffer) during depth-stencil testing. The stencil buffer typically shares the same memory space as the depth buffer (z-buffer). The depth-stencil view interface created by this program will only interpret the depth-stencil surface as a depth buffer (z-buffer) rather than a depth-stencil buffer.

ID3D11Texture2D* pBackBuffer;								// The pointer to a 2D texture interface.			A 2D texture interface manages texel data, which is structured memory. In this case for the back buffer interface.
ID3D11RenderTargetView* backbuffer;							// The pointer to the render target view interface. A render target view interface identifies the render target subresources (pBackBuffer) that can be accessed during rendering, in this case the back buffer.

ID3D11InputLayout* pLayout;									// The pointer to the input-layout interface.		An input-layout interface holds a definition of how to feed vertex data that is laid out in memory into the input-assembler stage of the graphics pipeline.
ID3D11VertexShader* pVS;									// The pointer to the vertex shader interface.		A vertex shader interface manages an executable program (a vertex shader) that controls the vertex shader stage of the graphics pipeline.
ID3D11PixelShader* pPS;										// The pointer to the pixel shader interface.		A pixel  shader interface manages an executable program (a pixel shader)  that controls the pixel shader stage of the graphics pipeline.
ID3D11Buffer* pVBuffer;										// The pointer to a buffer interface.				A buffer interface accesses a buffer resource, which is unstructured memory. In this case the vertex buffer.
ID3D11Buffer* pIBuffer;										// The pointer to a buffer interface.				A buffer interface accesses a buffer resource, which is unstructured memory. In this case the index buffer.
ID3D11Buffer* pCBuffer;										// The pointer to a buffer interface.				A buffer interface accesses a buffer resource, which is unstructured memory. In this case the constant buffer.

ID3D11ShaderResourceView* pTextureView;						// The pointer to a shader resource view interface.	A shader resource view interface specifies the subresource a shader can access during rendering. In this case the texture image.

// Declare the C++ constant buffer structure used to assign values to the HLSL constant buffer structure.
// This structure represents a constant buffer used in the graphics rendering pipeline.
// It contains information that is passed to the vertex shader stage of the pipeline and can be used to transform geometric vertices and calculate lighting effects on them.
//
// The matFinal member is a 4x4 matrix that represents the combined world, view, and projection transformations that are applied to the geometric vertices of the geometry being rendered.
//
// The matRotate member is a 4x4 matrix that represents a rotation transformation that is applied to the geometric vertices of the geometry being rendered.
// It is a component of the world transformation, and therefore of the matFinal matrix, but is also included separately in the constant buffer structure because the vertex normal vectors at the geometric vertices also need to be transformed by the same rotation matrix in order to correctly calculate lighting effects.
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
struct {
	XMMATRIX matFinal;
	XMMATRIX matRotate;										// Vertex normal vectors, like the geometric vertices comprising the object, also need to be transformed by the rotation matrix to correctly calculate lighting effects.
	XMFLOAT4 LightVector;									// Directional light's direction.
	XMFLOAT4 LightColor;									// Directional light's color (whiter color == brighter color).
	XMFLOAT4 AmbientColor;									// Ambient     light's color (whiter color == brighter color).
} ConstantBuffer;

// End: DirectX Global Declarations.

// End: Global Declarations.

//***
// Function Definitions.
//***

// WinMain function: Definition
//   This function is the entry point for a Windows program (vs. main for a console program) that is called by a hidden function that is the real entry point.
//   Not using the name WinMain results in the error: LNK2019: unresolved external symbol WinMain referenced in function "int __cdecl invoke_main(void)" (?invoke_main@@YAHXZ)
//   Parameter nCmdShow is a flag with an initial value set by the Run field on the Shortcut tab of the Properties associated the executable file of this program. This initial value is passed to the ShowWindow function the first time that function is called.
//   WINAPI is a Microsoft Windows data type used to define a function return value. Most functions in the Windows API are declared using WINAPI. It expands to __stdcall.
int WINAPI WinMain(HINSTANCE hInstance,						// The "handle to an instance" or "handle to a module." The operating system uses this value to identify the executable when it is loaded in memory.
	HINSTANCE hPrevInstance,								// This parameter has no meaning. It was used in 16-bit Windows, but is now always zero.
	LPSTR lpCmdLine,										// Contains the command line arguments, excluding the program name, as an ANSI string. To retrieve the entire command line, as a UNICODE string, use the GetCommandLine function.
	int nCmdShow)											// Indicates if the main program window will be minimized, maximized, or shown normally.
{
	RECT wr;												// The RECT structure contains the coordinates of the top-left and bottom-right corners of the client rectangle (initially) or the window rectangle (after the AdjustWindowRectEx function returns).
	HWND hWnd;												// The HWND handle for the window, assigned its value by function CreateWindowEx.
	MSG msg;												// The MSG structure holds window message and thread message information.

	// Create the window class structure that contains window class information.
	WNDCLASSEX wc;											// Contains the window class information.
	ZeroMemory(&wc, sizeof(WNDCLASSEX));					// ZeroMemory macro: Fills a block of memory with zeros.

	// Assign values to the window class WNDCLASSEX structure's members. Any subordinate members (variable.member.subordinatemember) are described in the comments.
	wc.cbSize = sizeof(WNDCLASSEX);							// Assigned a value specifying the size, in bytes, of this structure. This must be set before calling the GetClassInfoEx function.
	wc.style = CS_HREDRAW | CS_VREDRAW;						// Assigned a value specifying Class Style(s), in any combination:
															// CS_HREDRAW Redraws the entire window if a movement or size adjustment changes the width of the client area.
															// CS_VREDRAW Redraws the entire window if a movement or size adjustment changes the height of the client area.
	wc.lpfnWndProc = WindowProc;							// Assigned a value specifying a pointer to the window procedure, i.e., the WindowProc function, which is the main message handler function for this program (The WindowProc function is not called here because WindowProc, not WindowProc(), is assigned to wc.lpfnWndProc).
	wc.hInstance = hInstance;								// Assigned a value specifying the "handle to an instance" or "handle to a module" (WinMain) that contains the window procedure for the class.
	wc.hCursor =											// Assigned a value specifying a handle to the class cursor, a cursor resource.
		LoadCursor(NULL, IDC_CROSS);						// The LoadCursor function loads the specified cursor:
															// 1st Parameter: NULL indicates a predefined cursor is specified in the 2nd parameter,	otherwise a handle to an instance of the module whose executable file contains the cursor resource specified in the 2nd parameter.
															// 2nd Parameter: A predefined cursor if NULL is specified in the 1st parameter,		otherwise the name of the cursor resource to be loaded from the module specified in the 1st parameter.
															//				  For example, IDC_ARROW is a standard arrow and IDC_CROSS is a crosshair.
	wc.hbrBackground =										// Assigned a value specifying a handle to the class background brush.
		(HBRUSH)(COLOR_WINDOW + 1);							// This can be a handle to a brush used to paint the background or, as used here, to indicate the color (white) of the background (the value 1 must be added to the chosen color).
	wc.lpszClassName = L"WindowClass1";						// Assigned a value specifying a pointer to a null-terminated string, or it is an atom. If it is a string, as here, it specifies the window class name. This can be any name registered with RegisterClassEx, or any of the predefined control-class names. This same name is passed to CreateWindowEx.

	// Register the WNDCLASSEX window class.
	RegisterClassEx(&wc);

	// Calculate the appropriate window rectangle size based on the client rectangle size.
	// The window rectangle contains the client rectangle, as well as the window border and, if present, the window menu.
	//
	// AdjustWindowRectEx function:
	//   Calculates the required size of the window rectangle, based on the desired size of the client rectangle. The window rectangle can then be passed to the CreateWindowEx function to create a window whose client area is the desired size.
	wr = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };				// Set "wr" to the client rectangle's coordinates (size).
	AdjustWindowRectEx(&wr,									// "wr" will contain the window rectangle's coordinates (size) when AdjustWindowRectEx returns. Call by reference ("&") is required to update the actual "wr" (and not a copy) in the function.
					   WS_OVERLAPPEDWINDOW,					// Window style. The same style is passed to CreateWindowEx.
					   FALSE,								// Whether we are using (TRUE) or not using (FALSE) menus.
					   WS_EX_LEFT);							// The extended window style of the window whose required size is to be calculated. WS_EX_LEFT is the default.

	// Create the window.
	// CreateWindowEx function:
	//   Create an overlapped, pop-up, or child window with an extended window style.
	//   Returns the HWND handle "hWnd" for the window.
	hWnd = CreateWindowEx(NULL,								// NULL: Use all defaults for the extended window style of the window being created.
						  L"WindowClass1",					// Name of the window class. The same name is assigned to wc.lpszClassName.
						  L"objRenderer",					// Title of the window.
						  WS_OVERLAPPEDWINDOW,				// Window style. The same style is passed to AdjustWindowRectEx.
						  900,								// x-position of the window's true top-left corner, in screen coordinates.
						  900,								// y-position of the window's true top-left corner, in screen coordinates.
						  wr.right - wr.left,				// Width of the window rectangle.
						  wr.bottom - wr.top,				// Height of the window rectangle.
						  NULL,								// NULL: no parent window.
						  NULL,								// NULL: no menus.
						  hInstance,						// hInstance is a handle to the instance of the module (WINAPI) to be associated with the window.
						  NULL);							// NULL: multiple windows.

	// Display the window on the screen.
	// ShowWindow function:
	//   Sets the specified window's show state.
	//     Parameter nCmdShow is a flag, declared in the WinMain function parameter list, indicating if the main program window will be minimized, maximized, or shown normally.
	//       The first time the ShowWindow function is called, the value of parameter nCmdShow should be the value obtained by the WinMain function in its nCmdShow parameter.
	ShowWindow(hWnd,										// The HWND handle for the window.
			   nCmdShow);									// Indicates if the main program window will be minimized, maximized, or shown normally.

	// Initialize and prepare Direct3D for use.
	if (int InitD3DRC = InitD3D(hWnd); InitD3DRC != 0)		// Call the InitD3D function and check its return code.
	{
		// The InitD3D function terminated abnormally. Terminate the WinMain function with the return code of the InitD3D function.
		return InitD3DRC;
	}

	msg = { 0 };											// Set the entire structure holding window and thread messages to null.

	// Message Processing
	//   All messages, both thread messages and window messages, are on the thread message queue.
	//
	// In the Win32 API, both thread messages and window messages are used for communication between the operating system and the program, or within the program itself.
	//   Thread messages are retrieved from a specific thread message queue by the PeekMessage (or GetMessage) function in the infinite message loop.
	//     Since they are not associated with a specific window, they are processed directly in the infinite message loop or dispatched to a handler function designed for thread messages.
	//   Window messages are retrieved from a specific thread message queue by the PeekMessage (or GetMessage) function in the infinite message loop.
	//     Since they are	  associated with a specific window, they are dispatched by the DispatchMessage function in the infinite message loop to the window procedure, WindowProc, associated with the specific window for processing.

	// The Infinite Message Loop
	//   All messages, both thread messages and window messages, are retrieved in the infinite message loop.
	//
	// Create the infinite message loop and retrieve and process window messages and thread messages from the thread message queue.
	// You create the infinite message loop using the PeekMessage (or GetMessage), TranslateMessage and DispatchMessage functions.
	// The infinite message loop breaks (is ended) when a WM_QUIT thread message is posted to the thread message queue and processed.
	while(TRUE)
	{
		// PeekMessage function:
		//   Dispatches incoming nonqueued messages, checks the thread message queue for a posted message, and retrieves the message (if any exist).
		// Unlike GetMessage, the PeekMessage function does not wait for a message to be posted before returning. This allows processing to continue between "peeking" at the thread message queue.
		// The PeekMessage function retrieves both thread messages and window messages.
		if (PeekMessage(&msg,								// A pointer to the MSG structure that holds window message and thread message information.
					   NULL,								// A handle to the window whose messages are to be retrieved. The window must belong to the current thread.
															// If hWnd is NULL, PeekMessage retrieves messages for any window that belongs to the current thread, and any messages on the current thread's thread message queue whose hwnd value is NULL (see the MSG structure).
															// Therefore if hWnd is NULL, both window messages and thread messages are processed.
															// If hWnd is -1, PeekMessage retrieves only messages on the current thread's thread message queue whose hwnd value is NULL, that is, thread messages as posted by PostMessage (when the hWnd parameter is NULL) or PostThreadMessage.
					   0,									// The value of the first message in the range of messages to be examined.
					   0,									// The value of the last message in the range of messages to be examined.
															// If the values of the first and last messages are both 0 then PeekMessage returns all available messages.
					   PM_REMOVE))							// PM_REMOVE indicates messages are removed from the thread message queue after processing by PeekMessage.
		{
			// A message is waiting in the thread message queue: Perform message processing.

			// TranslateMessage function:
			//   If your program must obtain character input from the user, include this function in the infinite message loop.
			//     See https://docs.microsoft.com/en-us/windows/win32/winmsg/using-messages-and-message-queues.
			//   Translates a virtual-key message, which is a window message, into a new character message (no translation, or new character message creation, occurs if there is no virtual-key message).
			//   A virtual-key message is a window message that the system posts to the thread message queue when a key is pressed or released.
			//   The virtual-key message contains a virtual-key code to identify which key was pressed or released, along with additional information such as whether the key is being held down.
			//   The WM_KEYDOWN and WM_KEYUP messages are examples of virtual-key messages. These messages are sent to the window procedure, which can handle them to perform actions in response to key presses.
			//
			//   TranslateMessage does not modify the message pointed to by its first and only parameter. Instead, if translation occurs, it creates a new character message.
			//   The new character message is posted as a new message to the thread message queue, to be retrieved by the next call to the PeekMessage (or GetMessage) function.
			//   Therefore both the original virtual-key message (if any) and the new character message (if any) are retrieved by the PeekMessage (or GetMessage) function and dispatched by the DispatchMessage function.
			TranslateMessage(&msg);							// A pointer to the MSG structure that holds window message and thread message information.

			// DispatchMessage function:
			//   Dispatches a window message to the window procedure, i.e., the WindowProc function, which is the main message handler function for this program.
			//   It is typically used to dispatch a window message retrieved by the PeekMessage (or GetMessage) function.
			// The window procedure processes the window message and returns control back to DispatchMessage, which then returns control to the point where DispatchMessage was called, i.e., here in this program:
			DispatchMessage(&msg);

			// Check whether it's time to quit this program by breaking out of the infinite message loop, e.g., Has the user closed the window or pressed the Escape key?
			// If a WM_DESTROY window message has been processed by the window procedure, i.e., the WindowProc function, then a WM_QUIT thread message has been posted to the thread message queue.
			if (msg.message == WM_QUIT)
				break;										// Break out of the infinite message loop.
			// Stay in the infinite message loop.			// <-- Or not.
		}
		else
		{
			// No message is waiting in the thread message queue: Perform main processing.

			// Execute the graphics generating code.
			RenderFrame();									// This function renders a single frame.
		}
	}

	// The infinite message loop has ended. This program's window has been closed manually: Terminate the program.

	// Terminate Direct3D.
	CleanD3D();

	// End: WinMain function
	return msg.wParam;										// The exit value returned to the operating system must be the wParam parameter value of the WM_QUIT message (see PostQuitMessage).
}

// WindowProc function: Definition
//   This function defines the window procedure, which is the main window message handler for this program (see wc.lpfnWndProc = WindowProc).
//   Every window has an associated window procedure, which is a function that processes all window messages sent or posted to all windows of the class. All aspects of a window's appearance and behavior depend on the window procedure's response to these messages.
//   There is one instance of this function for each window class.
//   Its parameters are the elements of the MSG structure that defines "msg".
//   CALLBACK is a Microsoft Windows data type used to define a function return value, in this case indicating the calling convention for callback functions. It expands to __stdcall.
LRESULT CALLBACK WindowProc(HWND hWnd,						// The HWND handle for the window.
							UINT message,					// The message, e.g., the WM_DESTROY message indicates the window is destroyed.
							WPARAM wParam,					// Additional data that pertains to the message. The exact meaning depends on the message.
							LPARAM lParam)					// Additional data that pertains to the message. The exact meaning depends on the message.
{
	// Attempt to identify the current message on the thread message queue.
	// This switch statement uses return statements to exit the WindowProc function when a message is identified, not the break statements normally used in a switch statement.
	// In the context of a WindowProc function, it's common to see return statements instead of break statements in the switch clause. This is because the WindowProc function itself is expected to return a value: the result of its message processing.
	switch(message)
	{
		case WM_DESTROY:
			// The user closed the window      (messages =                             *WM_CLOSE* -> WM_DESTROY -> WM_QUIT), or
			// The user pressed the Escape key (messages = WM_KEYDOWN -> *VK_ESCAPE* -> WM_CLOSE  -> WM_DESTROY -> WM_QUIT).
			// In either case a WM_DESTROY message is sent to the thread message queue of the window being destroyed.
			//
			// WM_DESTROY message:
			//   It is sent to the thread message queue of the window being destroyed after the window is removed from the screen.
			//   This message is sent first for the window being destroyed and then for child windows (if any) as they are destroyed. During the processing of the message, it can be assumed that all child windows still exist.
			//
			// PostQuitMessage function:
			//   Indicates to the operating system that a thread has made a request to terminate (quit). It is typically called in response to a WM_DESTROY message.
			//   Posts a new WM_QUIT thread message to the thread message queue and returns immediately, indicating to the operating system that the thread is requesting to quit at some time in the future.
			//     The WM_QUIT thread message is not associated with a window and therefore will never be received through a window's window procedure. It is retrieved only by the PeekMessage (or GetMessage) function, which in this program is in the infinite message loop.
			//     Once the WM_QUIT thread message is retrieved and identified, the program breaks out of the infinite message loop and terminates.
			//     Its parameter is an exit code used as the wParam parameter of the WM_QUIT thread message.
			PostQuitMessage(0);								// PostQuitMessage(x), where x is an exit code used as the wParam parameter of the WM_QUIT thread message.
			return 0;										// The WindowProc function returns 0.
		case WM_KEYDOWN:
			// The user pressed a key (message = *WM_KEYDOWN*).
			// 
			// WM_KEYDOWN message:
			//   This message is sent to the thread message queue of the window with the keyboard focus when a non-system key is pressed.
			//   A non-system key is a key that is pressed when the ALT key is not pressed.
			switch (wParam)									// The virtual-key code of the non-system key.
			{
				case VK_ESCAPE:
					// The user pressed the Escape key (messages = WM_KEYDOWN -> *VK_ESCAPE* -> WM_CLOSE -> WM_DESTROY -> WM_QUIT).
					//
					// VK_ESCAPE message:
					//   The virtual-key code of the non-system key is VK_ESCAPE, the Escape key.
					//
					// PostMessage function:
					//   Places (posts) a window message in the thread message queue associated with the thread that created the specified window and returns without waiting for the thread to process the message.
					//   The form of the PostMessage function used here posts a WM_CLOSE message to the thread message queue of the window being closed.
					//   WM_CLOSE is sent as a signal that a window or an program should terminate.
					//   WM_CLOSE is received by a window through its window procedure, i.e., the WindowProc function.
					//     By default, the window procedure's (the WindowProc function's) default window procedure (the DefWindowProc function) processes the WM_CLOSE message.
					PostMessage(hWnd, WM_CLOSE, 0, 0);
					return 0;								// The WindowProc function returns 0.
				default:
					// The user pressed a key other than the Escape key  (messages = WM_KEYDOWN -> *return*).
					return 0;
			}
		default:
			// DefWindowProc function:
			//   This function is the default window procedure, called to provide default processing for any window messages that a program does not otherwise explicitly process in the window procedure, i.e., the WindowProc function.
			//   It is called with the same parameters received by the WindowProc function.
			//   The return value is the result of the message processing and depends on the message.
			// If the DefWindowProc function should be called but is not, then no window is displayed by this program (return 0 is not a sufficient alternative to the DefWindowProc function).
			// The DefWindowProc function processes the WM_CLOSE message by calling the DestroyWindow function to destroy the window. This is the default behavior: The DestroyWindow function is not coded in this program.
			//   The DestroyWindow function,
			//     1. Sends WM_DESTROY and WM_NCDESTROY window messages to the window to deactivate it and remove the keyboard focus from it.
			//     2. Destroys the window's menu, flushes the thread message queue, destroys timers, removes clipboard ownership, and breaks the clipboard viewer chain (if the window is at the top of the viewer chain).
			//     3. If the specified window is a parent or owner window, DestroyWindow automatically destroys the associated child or owned windows when it destroys the parent or owner window.
			//        The function first destroys child or owned windows, and then it destroys the parent or owner window.
			//     4. Destroys modeless dialog boxes created by the CreateDialog function.
			return DefWindowProc(hWnd,						// The HWND handle for the window.
				message,									// The message.
				wParam,										// Additional data that pertains to the message. The exact meaning depends on the message.
				lParam);									// Additional data that pertains to the message. The exact meaning depends on the message.
	}

	// End: WindowProc function
}

// InitD3D function: Definition
//   This function initializes and prepares Direct3D for use:
//     1. Create the device, the device context, and the swap chain with one back buffer.
//
//     2. Create the depth-stencil buffer (depth buffer (z-buffer)).
//
//     3. Complete setting up the back buffer.
//
//     4. Set the render target (back buffer) and the depth buffer (z-buffer) to the output-merger stage of the graphics pipeline.
//
//     5. Set the viewport to the rasterizer stage of the graphics pipeline.
//
//     6. Initialize the graphics pipeline.
//
//     7. Load and initialize all graphics data.
int InitD3D(HWND hWnd)										// The HWND handle for the window.
{
	//***
	// 1. Create the device, the device context, and the swap chain with one back buffer.
	//    The swap chain is created with one front buffer and one back buffer.
	//    The device and the device context represent the display adapter.
	//    The swap chain is used for rendering, and is created with one front buffer and one or more back buffers (in this program only one back buffer is created).
	//    Both types of buffer are frame buffers. The frame buffer that is currently being displayed is called the front buffer, and the frame buffers that we are rendering to are called the back buffers.
	//***

	// Create the swap chain description structure used to describe the swap chain.
	DXGI_SWAP_CHAIN_DESC scd;								// Describes the swap chain.
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));			// ZeroMemory macro: Fills a block of memory with zeros.

	// Assign values to the swap chain description DXGI_SWAP_CHAIN_DESC structure's members. Any subordinate members (variable.member.subordinatemember) are described in the comments.
	scd.BufferDesc.Width = SCREEN_WIDTH;					// .Width:	A member of DXGI_MODE_DESC structure assigned a value specifying the resolution width.	Set the back buffer width (needed when going full screen).
	scd.BufferDesc.Height = SCREEN_HEIGHT;					// .Height:	A member of DXGI_MODE_DESC structure assigned a value specifying the resolution height.	Set the back buffer height (needed when going full screen).
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;		// .Format:	A member of DXGI_MODE_DESC structure assigned a value that describes the display format.						A value of the DXGI_FORMAT enumerated type,			 i.e., DXGI_FORMAT_R8G8B8A8_UNORM:			   A four-component, 32-bit unsigned-normalized-integer format that supports 8 bits per channel including alpha, i.e., 32-bit color.
	scd.SampleDesc.Count = 4;								// .Count:	A member of DXGI_SAMPLE_DESC structure assigned a value specifying the number of multisamples per pixel. Also see texd.SampleDesc.Count.
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;		// Assigned a value specifying the surface usage and CPU access options for the back buffer.								A value of the DXGI_USAGE constants,				 i.e., DXGI_USAGE_RENDER_TARGET_OUTPUT:		   Use the surface or resource as an output render target.
	scd.BufferCount = 1;									// Assigned a value that describes the number of buffers in the swap chain. We'll create one back buffer. The back buffer can be used for shader input or render target output.
	scd.OutputWindow = hWnd;								// Assigned a value specifying the HWND handle for the window. This member must not be NULL.
	scd.Windowed = TRUE;									// Assigned a Boolean value that specifies whether the output is in windowed mode. TRUE if the output is in windowed mode; otherwise, FALSE. "We recommend that you create a windowed swap chain and allow the end user to change the swap chain to full screen through IDXGISwapChain::SetFullscreenState."
	// scd.SwapEffect										// Assigned a value that describes options for handling the contents of the presentation buffer after presenting a surface. A value of the DXGI_SWAP_EFFECT enumerated type.
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;		// Assigned a value that describes options for swap-chain behavior.															A value of the DXGI_SWAP_CHAIN_FLAG enumerated type, i.e., DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH: Enable an program to switch modes by calling IDXGISwapChain::ResizeTarget. When switching from windowed to full screen mode, the display mode (or monitor resolution) will be changed to match the dimensions of the program window. This allows switching to full screen via Alt-Enter.

	// Create the Direct3D feature level enumeration used to describe the set of features targeted by the Direct3D device.
	D3D_FEATURE_LEVEL pFeatureLevelsIn[] = {				// A pointer to an array of members selected from the enumerator-list defined in the D3D_FEATURE_LEVEL enumerated type.
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};
	D3D_FEATURE_LEVEL pFeatureLevelOut;						// The first element in an array of Direct3D driver types supported by the device created by the D3D11CreateDeviceAndSwapChain function.

	// D3D11CreateDeviceAndSwapChain function:
	//   Create the device, device context, and swap chain: "dev, devcon, swapchain"
	//   Notes on parameters:
	//   2nd Parameter: (2) Direct3D driver type. This parameter affects the 3rd and 11th parameters.
	//						D3D_DRIVER_TYPE_HARDWARE	(implements Direct3D features in hardware where possible, otherwise in software. Recommended for production)
	//						D3D_DRIVER_TYPE_REFERENCE	(implements every Direct3D features in software. The rasterizer portion of the driver does make use of special CPU instructions whenever it can. A reference driver is designed for accuracy rather than speed and is useful for testing)
	//						D3D_DRIVER_TYPE_WARP		(implements a high-performance software rasterizer in software. A WARP driver is designed for speed and is useful for testing)
	//						D3D_DRIVER_TYPE_SOFTWARE	(implements a low-performance software rasterizer in software, in which case the 3rd parameter is a handle to a DLL that implements the software rasterizer.
	//	 3rd Parameter: (3) When the 2nd parameter (Direct3D driver type) is specified as D3D_DRIVER_TYPE_SOFTWARE, this parameter specifies a handle to a DLL that implements a software rasterizer.
	//	 5th Parameter: (5) An array of Direct3D feature levels, which determine the order of feature levels to attempt to create. If NULL is specified then a preset array of feature levels is used.
	//  11th Parameter: (11) Direct3D feature level supported by the computer's display adapter, unless the 2nd parameter is specified as D3D_DRIVER_TYPE_WARP or D3D_DRIVER_TYPE_SOFTWARE, in which case this parameter potentially returns a higher feature level than what is supported by the computer's display adapter.
	//						NULL is sufficient if both of the following two conditions are true:
	//						1. The display adapter returns feature level D3D_FEATURE_LEVEL_9_1 or higher.
	//							And:
	//						2. The appropriate shader compiler target value (used in the D3DCompileFromFile function) is known and fixed.
	//
	//						Optionally use and inspect pFeatureLevelOut to:
	//						*** This is not done by this program ***
	//						1. When the feature level is		< D3D_FEATURE_LEVEL_9_1.
	//							Inform the user they have an insufficient display adapter and end this program.
	//						2. Or, when the feature level is	>= D3D_FEATURE_LEVEL_9_1:
	//							Determine the shader compiler target value appropriate for the D3DCompileFromFile function based on pFeatureLevelOut.
	//							The returned feature level must be used to determine the appropriate shader compiler target for the D3DCompileFromFile function (see the coding of that function for more information).
	D3D11CreateDeviceAndSwapChain(NULL,						// A pointer to the display adapter to use when creating a device. Pass NULL to use the default display adapter, which is the first display adapter enumerated by IDXGIFactory1::EnumAdapters.
		D3D_DRIVER_TYPE_HARDWARE,							// (2) The Direct3D driver type enumeration, which represents the Direct3D driver type to create.
		NULL,												// (3) Conditionally NULL or a handle to a DLL that implements a software rasterizer.
		NULL,												// Optional flags describing parameters used to create the device.
		pFeatureLevelsIn,									// (5) An array of Direct3D feature levels, which determine the order of feature levels to attempt to create.
		ARRAYSIZE(pFeatureLevelsIn),						// This is the number of elements in the 5th parameter.
		D3D11_SDK_VERSION,									// The SDK version; use D3D11_SDK_VERSION as D3D11CreateDeviceAndSwapChain is a D3D11 function.
		&scd,												// "&scd" is the address of (and therefore a pointer to) a swap chain description structure that contains initialization parameters for the swap chain.
		&swapchain,											// > Swap Chain:		The function returns "&swapchain",		which is the address of a pointer, "swapchain", to the swap chain interface that implements one or more surfaces for storing rendered data before presenting it to an output.
		&dev,												// > Device:			The function returns "&dev",			which is the address of a pointer, "dev", to the device interface that is the virtual representation of the computer's display adapter.
		&pFeatureLevelOut,									// (11)					The function returns &pFeatureLevelOut,	which is the address of (and therefore a pointer to) the first element in the array of Direct3D feature levels supported by the device.
		&devcon);											// > Device Context:	The function returns "&devcon",			which is the address of a pointer, "devcon", to the device context interface responsible for managing the graphics pipeline.

	// End: 1. Create the device, the device context, and the swap chain with one back buffer.

	//***
	// 2. Create the depth-stencil buffer (depth buffer (z-buffer)).
	//    Create the 2D texture array (in this case an array of one) that serves as the depth-stencil surface, and the depth-stencil view interface that in this program will only interpret the depth-stencil surface as a depth buffer (z-buffer) rather than a depth-stencil buffer.
	//    A depth buffer (z-buffer) stores depth information to control which areas of polygons are rendered rather than hidden from the viewer.
	//    A stencil buffer is used to mask pixels in an image, to produce special effects, including compositing; decaling; dissolves, fades, and swipes; outlines and silhouettes; and two-sided stencil.
	//    The stencil buffer typically shares the same memory space as the depth buffer (z-buffer). The depth-stencil view interface created by this program is only used as a depth buffer (z-buffer).
	//    A texture is a buffer of pixels (as defined by Direct3D).
	//      1. A view must be created and bound to the graphics pipeline.
	//      2. Using a view, texture data can be interpreted at run time within certain restrictions (textures cannot be bound directly to the graphics pipeline).
	//         For example, the depth-stencil view interface "depthbuffer", which effectively is the depth buffer (z-buffer), interprets texture data as a depth buffer (z-buffer).
	//***

	// Create the 2D texture description structure used to describe the 2D texture array (in this case an array of one) that will serve as the depth-stencil surface.
	D3D11_TEXTURE2D_DESC texd;								// The 2D texture description structure.
	ZeroMemory(&texd, sizeof(D3D11_TEXTURE2D_DESC));		// ZeroMemory macro: Fills a block of memory with zeros.

	// Assign values to the 2D texture description D3D11_TEXTURE2D_DESC structure's members. Any subordinate members (variable.member.subordinatemember) are described in the comments.
	texd.Width = SCREEN_WIDTH;								// Assigned a value specifying the texture width  (in texels). The range is constrained by the Direct3D feature level of the device.
	texd.Height = SCREEN_HEIGHT;							// Assigned a value specifying the texture height (in texels). The range is constrained by the Direct3D feature level of the device.
	texd.MipLevels = 1;										// Assigned a value specifying the maximum number of mipmap levels in the texture. Use 1 for a multisampled texture; or 0 to generate a full set of subtextures.
	texd.ArraySize = 1;										// Assigned a value specifying the number of textures in the 2D texture array. The range is constrained by the Direct3D feature level of the device.
	texd.Format = DXGI_FORMAT_D32_FLOAT;					// Assigned a value specifying the texture format.																			  A value of the DXGI_FORMAT enumerated type,	  i.e., DXGI_FORMAT_D32_FLOAT:	  A single-component, 32-bit floating-point format that supports 32 bits for depth.
	texd.SampleDesc.Count = 4;								// .Count: A member of DXGI_SAMPLE_DESC structure assigned a value specifying the number of multisamples per pixel. Also see scd.SampleDesc.Count
	texd.BindFlags = D3D11_BIND_DEPTH_STENCIL;				// Assigned values in any combination by a bitwise OR operation specifying the flags for binding to graphics pipeline stages. A value of the D3D11_BIND_FLAG enumerated type, i.e., D3D11_BIND_DEPTH_STENCIL: Bind a texture as a depth-stencil target for the output-merger stage of the graphics pipeline.

	// ID3D11Device::CreateTexture2D member function:
	//   Create the 2D texture array (in this case an array of one) that will serve as the depth-stencil surface.
	dev->CreateTexture2D(&texd,								// "&texd" is the address of (and therefore a pointer to) the 2D texture description structure used to describe the 2D texture array (in this case an array of one) that will serve as the depth-stencil surface.
		NULL,												// A pointer to the array of subresource initialization data structures that describe subresources for the 2D texture resource. If the resource is multisampled, this parameter must be NULL because multisampled resources cannot be initialized with data when they are created.
		&pDepthBuffer);										// &pDepthBuffer is the address of a pointer, pDepthBuffer, to the 2D texture interface for the created textures, an array of (in this case an array of one) textures that will serve as the depth-stencil surface.

	// Create the depth-stencil view description structure used to describe the depth-stencil view.
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;						  // The depth-stencil view description structure.
	ZeroMemory(&dsvd, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC)); // ZeroMemory macro: Fills a block of memory with zeros.

	// Assign values to the depth-stencil view description D3D11_DEPTH_STENCIL_VIEW_DESC structure's members. Any subordinate members (variable.member.subordinatemember) are described in the comments.
	dsvd.Format = DXGI_FORMAT_D32_FLOAT;					  // Assigned a value specifying the resource data format.						A value of the DXGI_FORMAT enumerated type,			i.e., DXGI_FORMAT_D32_FLOAT:		   A single-component, 32-bit floating-point format that supports 32 bits for depth.
	dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;	  // Assigned a value specifying how a depth-stencil resource will be accessed. A value of the D3D11_DSV_DIMENSION enumerated type, i.e., D3D11_DSV_DIMENSION_TEXTURE2DMS: The resource will be accessed as a 2D texture with multisampling.

	// ID3D11Device::CreateDepthStencilView member function:
	//   Create a depth-stencil view for accessing resource data.
	if (pDepthBuffer == NULL)								  // This check prevents the Visual Studio code analysis defect "C6387: 'pDepthBuffer' could be '0':" on statement "dev->CreateDepthStencilView".
	{
		// pDepthBuffer, a pointer to the 2D texture interface that will serve as the depth-stencil surface, is NULL: Exit the program.
		exit(1);
	}
	dev->CreateDepthStencilView(pDepthBuffer,				  // A pointer to the 2D texture interface for the created textures, an array of (in this case an array of one) textures that will serve as the depth-stencil surface.
		&dsvd,												  // "&dsvd" is the address of (and therefore a pointer to) the depth-stencil view description structure used to describe the depth-stencil view interface.
		&depthbuffer);										  // "&depthbuffer" is the address of a pointer, "depthbuffer", to the depth-stencil view interface, which effectively is the depth buffer (z-buffer).
	pDepthBuffer->Release();								  // Decrements the reference count for an interface on a COM object. If the reference count = 0, then the interface pointer is freed. If there are no other interface pointers, then the COM object is freed.

	// End: 2. Create the depth-stencil buffer (depth buffer (z-buffer)).

	//***
	// 3. Complete setting up the back buffer.
	//    Create the render target view for the render target, in this case one back buffer texture interface (in this program only one back buffer is created).
	//    The swap chain was created with one front buffer and one back buffer, and that back buffer is accessed here.
	//***

	// IDXGISwapChain::GetBuffer member function:
	//   Access one of the swap-chain's back buffers (in this program only one back buffer is created).
	swapchain->GetBuffer(0,									// "0" indicates the first back buffer is accessed (in this program only one back buffer is created).
		__uuidof(ID3D11Texture2D),							// The type of interface used to manipulate the back buffer. __uuidof retrieves the GUID attached to the expression.
		(LPVOID*)&pBackBuffer);								// &pBackBuffer is the address of a pointer, pBackBuffer, to the 2D texture interface that will serve as the back buffer texture interface.

	// ID3D11Device::CreateRenderTargetView member function:
	//   Creates a render-target view for accessing resource data.
	dev->CreateRenderTargetView(pBackBuffer,				// A pointer to the 2D texture interface that will serve as the render target, in this case one back buffer texture interface (in this program only one back buffer is created).
		NULL,												// A pointer to the render target view description structure that represents a render target view description. Setting this parameter to NULL creates a view that accesses all of the subresources in mipmap level 0.
		&backbuffer);										// "&backbuffer" is the address of a pointer, "backbuffer", to the render target view interface for the render target, in this case one back buffer texture interface (in this program only one back buffer is created).
	pBackBuffer->Release();									// Decrements the reference count for an interface on a COM object. If the reference count = 0, then the interface pointer is freed. If there are no other interface pointers, then the COM object is freed.

	// End: 3. Complete setting up the back buffer.

	//***
	// 4. Set the render target (back buffer) and the depth buffer (z-buffer) to the output-merger stage of the graphics pipeline.
	//***

	// ID3D11DeviceContext::OMSetRenderTargets member function:
	//   Set the render target (back buffer) and the depth buffer (z-buffer) to the output-merger stage of the graphics pipeline.
	//   Depth buffering can be disabled by changing "devcon->OMSetRenderTargets(1, &backbuffer, depthbuffer)" to "devcon->OMSetRenderTargets(1, &backbuffer, NULL)".
	devcon->OMSetRenderTargets(1,							// Number of render targets to bind.
		&backbuffer,										// "&backbuffer" is the address of a pointer, "backbuffer", to the render target view interface for the render target, in this case one back buffer texture interface (in this program only one back buffer is created).
		depthbuffer);										// A pointer to the depth-stencil view interface, which effectively is the depth buffer (z-buffer).

	// End: 4. Set the render target (back buffer) and the depth buffer (z-buffer) to the output-merger stage of the graphics pipeline.

	//***
	// 5. Set the viewport to the rasterizer stage of the graphics pipeline.
	//***
	
	// Create the viewport structure used to define the dimensions of the viewport.
	D3D11_VIEWPORT viewport;								// Defines the dimensions of the viewport.
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));			// ZeroMemory macro: Fills a block of memory with zeros.

	// Assign values to the dimensions of the viewport D3D11_VIEWPORT structure's members. Any subordinate members (variable.member.subordinatemember) are described in the comments.
	viewport.TopLeftX = 0;									// Assigned a value specifying the X position of the left hand side of the viewport. Ranges between D3D11_VIEWPORT_BOUNDS_MIN and D3D11_VIEWPORT_BOUNDS_MAX.
	viewport.TopLeftY = 0;									// Assigned a value specifying the Y position of the top of the viewport.			 Ranges between D3D11_VIEWPORT_BOUNDS_MIN and D3D11_VIEWPORT_BOUNDS_MAX.
	viewport.Width = SCREEN_WIDTH;							// Assigned a value specifying the width of the viewport.							 Width	must be >= 0. TopLeftX + Width	must be <= D3D11_VIEWPORT_BOUNDS_MAX.
	viewport.Height = SCREEN_HEIGHT;						// Assigned a value specifying the height of the viewport.							 Height	must be >= 0. TopLeftY + Height	must be <= D3D11_VIEWPORT_BOUNDS_MAX.
	viewport.MinDepth = 0;									// Assigned a value specifying the minimum depth of the viewport.					 Ranges between 0 and 1. (The closest an object can be on the depth buffer (z-buffer))
	viewport.MaxDepth = 1;									// Assigned a value specifying the maximum depth of the viewport.					 Ranges between 0 and 1. (The farthest an object can be on the depth buffer (z-buffer))

	// ID3D11DeviceContext::RSSetViewports member function:
	//   Set the array of (in this case an array of one) viewports to the rasterizer stage of the graphics pipeline.
	//   This indicates which portion of the back buffer to select pixels from.
	//   A viewport specifies a 2D rectangle into which a 3D scene is projected.
	//   In Direct3D, this rectangle exists as coordinates within a Direct3D surface that the system uses as a rendering target.
	//   The projection transformation converts geometric vertices into the coordinate system used for the viewport.
	//   A viewport is also used to specify the range of depth values on a render target surface into which a scene will be rendered (usually 0.0 to 1.0).
	devcon->RSSetViewports(1,								// Number of viewports to bind.
		&viewport);											// The address "&viewport" of (and therefore a pointer to) an array that, in this program, will be a one element array of D3D11_VIEWPORT structures to bind to the device.

	// End: 5. Set the viewport to the rasterizer stage of the graphics pipeline.

	//***
	// 6. Initialize the graphics pipeline.
	//***

	InitPipeline();

	// End: 6. Initialize the graphics pipeline.

	//***
	// 7. Load and initialize all graphics data.
	//***

	if (int InitGraphicsRC = InitGraphics(); InitGraphicsRC != 0)	// Call the InitGraphics function and check its return code.
	{
		// The InitGraphics function terminated abnormally. Terminate the InitD3D function with the return code of the InitGraphics function.
		return InitGraphicsRC;
	}

	// End: 7. Load and initialize all graphics data.

	// End: InitD3D function
	return 0;
}

// InitPipeline function: Definition
//   This function initializes the graphics pipeline:
//     1. Create the shader objects and set them to the associated shader stage of the graphics pipeline.
//
//     2. Create the input-layout object and set it to the input-assembler stage of the graphics pipeline.
//
//     3. Create the constant buffer object and set it to the vertex shader stage of the graphics pipeline.
void InitPipeline(void)
{
	//***
	// 1. Create the shader objects and set them to the associated shader stage of the graphics pipeline.
	//    Create the vertex shader object and set it to the vertex shader stage of the graphics pipeline.
	//    Create the pixel  shader object and set it to the pixel  shader stage of the graphics pipeline.
	//    Shader functions are small, low-level, programs that are compiled by the CPU and then run by the GPU at specific stages in the graphics pipeline. They automatically receive data from shader functions in previous stages, and return data to shader functions in successive stages, of the graphics pipeline.
	//    Their specialty is very fast floating-point mathematical operations. The most common shader programs are:
	//    Vertex shader:
	//      This shader is executed for each vertex in the scene.
	//      This shader operates on vertex buffer array elements provided to it by the calling program (this program) and at a minimum returns a 4-component position vector that will be rasterized into a pixel position.
	//      Optionally, this shader can output Vertex color, vertex texture coordinates, vertex lighting, fog factors, and other characteristics of a single vertex, are optional.
	//    Pixel shader:
	//      This shader is also known as a fragment shader.
	//      This shader is executed for each pixel (fragment) in the render target.
	//      This shader receives rasterized coordinates from previous shader stages (in the simplest pipelines, this would be the vertex shader) and at a minimum returns a color (or other 4-component value) for that pixel position, which is then written into a render target.
	//      Optionally, this shader can output the color, brightness, contrast, and other characteristics of a single pixel.
	//***

	ID3DBlob* VS;											// VS (Vertex Shader) is a pointer to the ID3DBlob interface used to return Direct3D data of arbitrary length.
	ID3DBlob* PS;											// PS (Pixel Shader)  is a pointer to the ID3DBlob interface used to return Direct3D data of arbitrary length.

	// D3DCompileFromFile function:
	//   Compile Microsoft High Level Shader Language (HLSL) code into bytecode for a given target.
	//   Compile the vertex shader:
	D3DCompileFromFile(L"shaders.hlsl",						// A pointer to a constant null-terminated string that contains the name of the file that contains the shader code.
		NULL,												// An optional array of D3D_SHADER_MACRO structures that define shader macros.
		NULL,												// An optional pointer to an ID3DInclude interface that the compiler uses to handle include files.
		"VShader",											// A pointer to a constant null-terminated string that contains the name of the shader entry point function where shader execution begins. When you compile an effect this parameter is ignored; Microsoft recommends setting it to NULL because it is good programming practice to set a pointer parameter to NULL if the called function will not use it.
		"vs_4_1",											// A pointer to a constant null-terminated string that specifies the shader target or set of shader features to compile against. The shader target can be a shader model. vs_4_1 is the vertex shader model (a shader target) of the Direct3D 10.1 feature level.
		D3DCOMPILE_DEBUG,									// A combination of shader compile options that are combined by using a bitwise OR operation. The resulting value specifies how the compiler compiles the HLSL code (set it to zero (0) to indicate no options). The D3DCOMPILE_DEBUG option directs the compiler to insert debug file/line/type/symbol information into the output code.
		0,													// A combination of effect compile options that are combined by using a bitwise OR operation. The resulting value specifies how the compiler compiles the effect. When you compile a shader and not an effect file, D3DCompileFromFile ignores this parameter (set it to zero (0) to indicate no options).
		&VS,												// &VS is the address of a pointer, VS, to the interface that you can use to access the compiled code.
		0);													// An optional pointer to a variable that receives a pointer to the ID3DBlob interface that you can use to access compiler error messages
	//   Compile the pixel shader:
	D3DCompileFromFile(L"shaders.hlsl",						// A pointer to a constant null-terminated string that contains the name of the file that contains the shader code.
		NULL,												// An optional array of D3D_SHADER_MACRO structures that define shader macros.
		NULL,												// An optional pointer to an ID3DInclude interface that the compiler uses to handle include files.
		"PShader",											// A pointer to a constant null-terminated string that contains the name of the shader entry point function where shader execution begins. When you compile an effect this parameter is ignored; Microsoft recommends setting it to NULL because it is good programming practice to set a pointer parameter to NULL if the called function will not use it.
		"ps_4_1",											// A pointer to a constant null-terminated string that specifies the shader target or set of shader features to compile against. The shader target can be a shader model. ps_4_1 is the pixel shader model (a shader target) of the Direct3D 10.1 feature level.
		D3DCOMPILE_DEBUG,									// A combination of shader compile options that are combined by using a bitwise OR operation. The resulting value specifies how the compiler compiles the HLSL code (set it to zero (0) to indicate no options). The D3DCOMPILE_DEBUG option directs the compiler to insert debug file/line/type/symbol information into the output code.
		0,													// A combination of effect compile options that are combined by using a bitwise OR operation. The resulting value specifies how the compiler compiles the effect. When you compile a shader and not an effect file, D3DCompileFromFile ignores this parameter (set it to zero (0) to indicate no options).
		&PS,												// &PS is the address of a pointer, PS, to the interface that you can use to access the compiled code.
		0);													// An optional pointer to a variable that receives a pointer to the ID3DBlob interface that you can use to access compiler error messages.

	// ID3D11Device::CreateVertexShader member function:
	//   Create the vertex shader object from a compiled shader.
	dev->CreateVertexShader(VS->GetBufferPointer(),			// A pointer to the compiled vertex shader.
		VS->GetBufferSize(),								// Size of the compiled vertex shader.
		NULL,												// An optional pointer to a class linkage ID3D11ClassLinkage interface.
		&pVS);												// &pVS is the address of a pointer, pVS, to the vertex shader ID3D11VertexShader interface.

	// ID3D11DeviceContext::VSSetShader member function:
	//   Set the vertex shader object to the vertex shader stage of the graphics pipeline.
	devcon->VSSetShader(pVS,								// Pointer to the vertex shader interface.
		0,													// An optional pointer to an array of class-instance ID3D11ClassInstance interfaces.
		0);													// The number of class-instance interfaces in the array.

	// ID3D11Device::CreatePixelShader member function:
	//   Create the pixel shader object from a compiled shader.
	dev->CreatePixelShader(PS->GetBufferPointer(),			// A pointer to the compiled pixel shader.
		PS->GetBufferSize(),								// Size of the compiled pixel shader.
		NULL,												// An optional pointer to a class linkage ID3D11ClassLinkage interface. 
		&pPS);												// &pPS is the address of a pointer, pPS, to the pixel shader ID3D11PixelShader interface.

	// ID3D11DeviceContext::PSSetShader member function:
	//   Set the pixel shader object to the pixel shader stage of the graphics pipeline.
	devcon->PSSetShader(pPS,								// Pointer to the pixel shader interface.
		0,													// An optional pointer to an array of class-instance ID3D11ClassInstance interfaces.
		0);													// The number of class-instance interfaces in the array.

	// End: 1. Create the shader objects and set them to the associated shader stage of the graphics pipeline.

	//***
	// 2. Create the input-layout object and set it to the input-assembler stage of the graphics pipeline.
	//    The input-layout object describes the VERTEX structure representing the attributes of each vertex of an object, i.e., the input buffers that will be read by the input-assembler stage of the graphics pipeline.
	//***

	// Create the input element description structure used to define the input-layout object that describes the VERTEX structure used in this program.
	D3D11_INPUT_ELEMENT_DESC ied[3];						// Defines the input-layout object containing an array of structures, each structure defines one element being read from an input slot.
	ZeroMemory(&ied, sizeof(ied));							// ZeroMemory macro: Fills a block of memory with zeros. "sizeof(ied)" is used instead of "sizeof(D3D11_INPUT_ELEMENT_DESC)" because the former fills both elements of array "ied".

	// Assign values to the input element description D3D11_INPUT_ELEMENT_DESC structure's members. Any subordinate members (variable.member.subordinatemember) are described in the comments.
	// Define the position input element of the VERTEX structure OurVertices.
	ied[0].SemanticName = "POSITION";						// Assigned a value specifying the HLSL semantic name associated with this element in a shader input signature.
	ied[0].SemanticIndex = 0;								// Assigned a value specifying the semantic index for the element. A semantic index modifies a semantic with an integer index number. A semantic index is only needed in a case where there is more than one element with the same semantic name.
	ied[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;			// Assigned a value specifying the data type of the element.					  A value of the DXGI_FORMAT enumerated type,				 i.e., DXGI_FORMAT_R32G32B32_FLOAT: A three-component, 96-bit floating-point format that supports 32 bits for the red channel, 32 bits for the green channel and 32 bits for the blue channel.
	ied[0].InputSlot = 0;									// Assigned a value specifying the integer value that identifies the input-assembler (see input slot).
	ied[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;// Assigned a value specifying the optional offset (in bytes) from the start of the vertex. Use D3D11_APPEND_ALIGNED_ELEMENT for convenience to define the current element directly after the previous one, including any packing if necessary. Position has an offset of 0 in this program.
	ied[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;	// Assigned a value specifying the input data slot class for a single input slot. A value of the D3D11_INPUT_CLASSIFICATION enumerated type, i.e., D3D11_INPUT_PER_VERTEX_DATA: Input data is per-vertex data.
	ied[0].InstanceDataStepRate = 0;						// Assigned a value specifying the number of instances to draw using the same per-instance data before advancing in the buffer by one element. This value must be 0 for an element that contains per-vertex data (the slot class is set to D3D11_INPUT_PER_VERTEX_DATA).

	// Define the texture  input element of the VERTEX structure OurVertices.
	ied[1].SemanticName = "TEXCOORD";						// Assigned a value specifying the HLSL semantic name associated with this element in a shader input signature.
	ied[1].SemanticIndex = 0;								// Assigned a value specifying the semantic index for the element. A semantic index modifies a semantic with an integer index number. A semantic index is only needed in a case where there is more than one element with the same semantic name.
	ied[1].Format = DXGI_FORMAT_R32G32_FLOAT;				// Assigned a value specifying the data type of the element.					  A value of the DXGI_FORMAT enumerated type,				 i.e., DXGI_FORMAT_R32G32_FLOAT:	A two-component, 64-bit floating-point format that supports 32 bits for the red channel and 32 bits for the green channel.
	ied[1].InputSlot = 0;									// Assigned a value specifying the integer value that identifies the input-assembler (see input slot).
	ied[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;// Assigned a value specifying the optional offset (in bytes) from the start of the vertex. Use D3D11_APPEND_ALIGNED_ELEMENT for convenience to define the current element directly after the previous one, including any packing if necessary. Normal has an offset of 12 in this program.
	ied[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;	// Assigned a value specifying the input data slot class for a single input slot. A value of the D3D11_INPUT_CLASSIFICATION enumerated type, i.e., D3D11_INPUT_PER_VERTEX_DATA: Input data is per-vertex data.
	ied[1].InstanceDataStepRate = 0;						// Assigned a value specifying the number of instances to draw using the same per-instance data before advancing in the buffer by one element. This value must be 0 for an element that contains per-vertex data (the slot class is set to D3D11_INPUT_PER_VERTEX_DATA).

	// Define the normal   input element of the VERTEX structure OurVertices.
	ied[2].SemanticName = "NORMAL";							// Assigned a value specifying the HLSL semantic name associated with this element in a shader input signature.
	ied[2].SemanticIndex = 0;								// Assigned a value specifying the semantic index for the element. A semantic index modifies a semantic with an integer index number. A semantic index is only needed in a case where there is more than one element with the same semantic name.
	ied[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;			// Assigned a value specifying the data type of the element.					  A value of the DXGI_FORMAT enumerated type,				 i.e., DXGI_FORMAT_R32G32B32_FLOAT: A three-component, 96-bit floating-point format that supports 32 bits for the red channel, 32 bits for the green channel and 32 bits for the blue channel.
	ied[2].InputSlot = 0;									// Assigned a value specifying the integer value that identifies the input-assembler (see input slot).
	ied[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;// Assigned a value specifying the optional offset (in bytes) from the start of the vertex. Use D3D11_APPEND_ALIGNED_ELEMENT for convenience to define the current element directly after the previous one, including any packing if necessary. Normal has an offset of 12 in this program.
	ied[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;	// Assigned a value specifying the input data slot class for a single input slot. A value of the D3D11_INPUT_CLASSIFICATION enumerated type, i.e., D3D11_INPUT_PER_VERTEX_DATA: Input data is per-vertex data.
	ied[2].InstanceDataStepRate = 0;						// Assigned a value specifying the number of instances to draw using the same per-instance data before advancing in the buffer by one element. This value must be 0 for an element that contains per-vertex data (the slot class is set to D3D11_INPUT_PER_VERTEX_DATA).

	// ID3D11Device::CreateInputLayout member function:
	//   Create the input-layout object to describe the input-buffer data for the input-assembler stage of the graphics pipeline.
	dev->CreateInputLayout(ied,								// An array of the input-assembler stage input data types, in this case POSITION and NORMAL, used to define the input-layout object. Each input data type is described by an element description.
		3,													// The number of input data types in the array, in this case 2 (POSITION and NORMAL), used to define the input-layout object.
		VS->GetBufferPointer(),								// Pointer to the compiled shader.
		VS->GetBufferSize(),								// Size of the compiled shader.
		&pLayout);											// &pLayout is the address of a pointer, pLayout, to an input-layout ID3D11InputLayout interface.

	// ID3D11DeviceContext::IASetInputLayout member function:
	//   Set the input-layout object to the input-assembler stage of the graphics pipeline.
	devcon->IASetInputLayout(pLayout);						// Pointer to the input-layout interface.

	// End: 2. Create the input-layout object and set it to the input-assembler stage of the graphics pipeline.

	//***
	// 3. Create the constant buffer object and set it to the vertex shader stage of the graphics pipeline.
	//		Multiple constant buffers can be created (see below) and each can be set to the vertex shader and/or pixel shader stage of the graphics pipeline, depending on how it will be used.
	//      Constant buffers are optimized for constant variable usage, which is characterized by lower-latency access and more frequent update from the CPU.
	//      Constant buffers are used to store data that is shared by all shaders in the graphics pipeline.
	//      The size of the constant buffer resource must be a multiple of 16 bytes, because constants are sent to the GPU in packs of 16 bytes, regardless of the size of the C++ constant buffer structure.
	//      A constant buffer can be a structure containing multiple constants. The order and size of these structure's members must match in both C++ and HLSL.
	//      Any one constant (structure member) cannot be split between two 16-byte areas of memory. Therefore, if the first constant in the structure is less than 16 bytes then the second constant will be aligned on the next 16-byte boundary. When it occurs, this automatic alignment must be accounted for in the C++ constant buffer structure in C++, otherwise it will not match the HLSL constant buffer structure, even if their code looks identical.
	//
	//      These structures and their members are optionally named the same in C++ and HLSL.
	//      The C++ constant buffer structure is copied to the constant buffer pointed to by pCBuffer using the ID3D11DeviceContext::UpdateSubresource member function.
	//      Copying to the constant buffer always provides		  position information for the object rendered, as it does in this program.
	//      Copying to the constant buffer may optionally provide scene    information for the object rendered, such as lighting information, timing information, among other details.
	//      A constant buffer's information should be sent to the GPU only as needed, matching its frequency of update. For example, if position information and scene information are updated at different frequencies (at different times), then create two constant buffers, one for position information and one for scene information.
	//
	//    Constant buffer and texture buffer shader constants:
	//      In Shader Model 4 (used in this program), shader constants are stored in one or more buffer resources in memory. They can be organized into two types of buffer: constant buffers (cbuffer) and texture buffers (tbuffer).
	//
	//      Constant buffers are optimized for constant-variable usage, which is characterized by lower-latency access and more frequent update from the CPU.
	//      A constant buffer is a specialized buffer resource that is accessed like a buffer. Each constant buffer can hold up to 4096 vectors; each vector contains up to four 32-bit values. You can bind up to 14 constant buffers per graphics pipeline stage (2 additional slots are reserved for internal use).
	//      A constant buffer does not require a view to be bound to the pipeline.
	//
	//      Texture buffers are accessed like textures and perform better for arbitrarily indexed data.
	//      A texture buffer is a specialized buffer resource that is accessed like a texture. Texture access (as compared with standard buffer access) can have better performance for arbitrarily indexed data. You can bind up to 128 texture buffers per graphics pipeline stage.
	//      A texture buffer requires a view and must be bound to a texture slot (or must be bound with SetTextureBuffer when using an effect).
	//
	//      Regardless of which type of resource you use, there is no limit to the number of constant buffers or texture buffers a program can create.
	//
	//      A buffer resource is designed to minimize the overhead of setting shader constants.
	//        Manage buffer updates using:
	//        - The effect framework (see ID3D10Effect Interface)
	//        - The Direct3D API (see ID3D11DeviceContext::UpdateSubresource member function)
	//        - A program can also copy data from another buffer (such as a render target or a stream-output target) into a constant buffer.
	//***

	// Create the buffer resource description structure used to define the constant buffer.
	D3D11_BUFFER_DESC bd;									// Describes the buffer resource.
	ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));				// ZeroMemory macro: Fills a block of memory with zeros.

	// Assign values to the buffer resource description D3D11_BUFFER_DESC structure's members. Any subordinate members (variable.member.subordinatemember) are described in the comments.
	bd.ByteWidth = sizeof(ConstantBuffer);					// Assigned a value specifying the size of the buffer in bytes. See the preceding comments for related information on the size of the constant buffer resource, including limitations.
	bd.Usage = D3D11_USAGE_DEFAULT;							// Assigned a value that identifies how the buffer is expected to be read from and written to. Frequency of update is a key factor. A value of the D3D11_USAGE enumerated type,		i.e., D3D11_USAGE_DEFAULT:		  A resource that requires read and write access by the GPU. This is likely to be the most common usage choice.
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;				// Assigned values in any combination by a bitwise OR operation specifying the flags for binding to graphics pipeline stages.		A value of the D3D11_BIND_FLAG enumerated type,	i.e., D3D11_BIND_CONSTANT_BUFFER: Bind a buffer as a constant buffer to a shader stage of the graphics pipeline; this flag may NOT be combined with any other bind flag.

	// ID3D11Device::CreateBuffer member function:
	//   Create the buffer object (vertex buffer, index buffer, or shader constant buffer), in this case the constant buffer object.
	dev->CreateBuffer(&bd,									// A pointer to a D3D11_BUFFER_DESC structure that describes the buffer, in this case a constant buffer as per bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER.
		NULL,												// A pointer to a D3D11_SUBRESOURCE_DATA structure that describes the initialization data; use NULL to allocate space only (with the exception that it cannot be NULL if bd.Usage is D3D11_USAGE_IMMUTABLE).
		&pCBuffer);											// &pCBuffer is the address of a pointer, pCBuffer, to the buffer interface for the buffer object created, in this case a constant buffer object.

	// ID3D11DeviceContext::VSSetConstantBuffers member function:
	//   Set the constant buffer object to the vertex shader stage of the graphics pipeline.
	devcon->VSSetConstantBuffers(0,							// Index into the device's zero-based array to begin setting constant buffers to (ranges from 0 to D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1).
		1,													// Number of buffers to set (ranges from 0 to D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - StartSlot).
		&pCBuffer);											// &pCBuffer is the address of a pointer, pCBuffer, to the constant buffer interface.

	// End: 3. Create the constant buffer object and set it to the vertex shader stage of the graphics pipeline.

	// End: InitPipeline function
}

// InitGraphics function: Definition
//   This function loads and initializes all graphics data.
//     1. Call the objReader function, which reads and parses a single 3D object's descriptive information from a Wavefront .obj file and uses it to define the variables needed to render the 3D object, i.e., OurVertices and OurIndices.
//
//     2. Create the structures used to define the vertex buffer and index buffer.
//
//     3. Create the vertex buffer and assign values to it from the variable OurVertices.
//
//     4. Create the index buffer and assign values to it from the variable OurIndices.
//
//     5. Create the texture image from an image file.
int InitGraphics(void)
{
	//***
	// 1. Call the objReader function, which reads and parses a single 3D object's descriptive information from a Wavefront .obj file and uses it to define the variables needed to render the 3D object, i.e., OurVertices and OurIndices.
	//***

	if (int objReaderRC = objReader(); objReaderRC != 0)	// Call the objReader function and check its return code.
	{
		// The objReader function terminated abnormally. Terminate the InitGraphics function with the return code of the objReader function.
		return objReaderRC;
	}

	// End: 1. Call the objReader function, which reads and parses a single 3D object's descriptive information from a Wavefront .obj file and uses it to define the variables needed to render the 3D object, i.e., OurVertices and OurIndices.

	//***
	// 2. Create the structures used to define the vertex buffer and index buffer.
	//***

	// Create the mapped subresource structure that provides access to subresource data. It is used to define the vertex buffer and index buffer.
	// The ID3D11DeviceContext::Map member function uses this structure.
	D3D11_MAPPED_SUBRESOURCE ms;							// Provides access to subresource data. ms.pData is used to copy data to both the vertex buffer and the index buffer.

	// Create the buffer resource description structure used to define the vertex buffer and index buffer.
	D3D11_BUFFER_DESC bd;									// Describes the buffer resource.
	ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));				// ZeroMemory macro: Fills a block of memory with zeros.

	// End: 2. Create the structures used to define the vertex buffer and index buffer.

	//***
	// 3. Create the vertex buffer and assign values to it from the variable OurVertices.
	//***

	// Assign values to the buffer resource description D3D11_BUFFER_DESC structure's members. Any subordinate members (variable.member.subordinatemember) are described in the comments.
	bd.ByteWidth = sizeof(VERTEX) * VertexAttributeSetsTotal;	// Assigned a value specifying the size of the buffer in bytes. The vertex buffer resource's size is the size of the VERTEX structure * the total number of array elements in OurVertices (VertexAttributeSetsTotal).
	bd.Usage = D3D11_USAGE_DYNAMIC;								// Assigned a value that identifies how the buffer is expected to be read from and written to. Frequency of update is a key factor.	A value of the D3D11_USAGE enumerated type,			  i.e., D3D11_USAGE_DYNAMIC:	  A resource that is accessible by both the GPU (read only) and the CPU (write only). A dynamic resource is a good choice for a resource that will be updated by the CPU at least once per frame. To update a dynamic resource, use a Map member function.
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;					// Assigned values in any combination by a bitwise OR operation specifying the flags for binding to graphics pipeline stages.		A value of the D3D11_BIND_FLAG enumerated type,		  i.e., D3D11_BIND_VERTEX_BUFFER: Bind a buffer as a vertex buffer to the input-assembler stage of the graphics pipeline.
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;					// Assigned values in any combination by a bitwise OR operation specifying the flags for binding to graphics pipeline stages.		A value of the D3D11_CPU_ACCESS_FLAG enumerated type, i.e., D3D11_CPU_ACCESS_WRITE:	  The resource is to be mappable so that the CPU can change its contents. Resources created with this flag cannot be set as outputs of the graphics pipeline and must be created with either dynamic or staging usage (see D3D11_USAGE).

	// ID3D11Device::CreateBuffer member function:
	//   Create a buffer object (vertex buffer, index buffer, or shader constant buffer), in this case the vertex buffer object.
	dev->CreateBuffer(&bd,									// A pointer to a buffer resource description structure that describes the buffer, in this case the vertex buffer, as per bd.BindFlags = D3D11_BIND_VERTEX_BUFFER.
		NULL,												// A pointer to a D3D11_SUBRESOURCE_DATA structure that describes the initialization data; use NULL to allocate space only (with the exception that it cannot be NULL if bd.Usage is D3D11_USAGE_IMMUTABLE).
		&pVBuffer);											// &pVBuffer is the address of a pointer, pVBuffer, to the buffer interface for the buffer object created, in this case the vertex buffer object.

	// Assign the vertex attributes by copying them from OurVertices to the vertex buffer.
	// ID3D11DeviceContext::Map member function:
	//   Mapping a buffer allows us to access it.
	//   Gets a pointer to the data contained in a subresource, and denies the GPU access to that subresource.
	//   The third parameter is a set of flags that allows us to control the CPUs access to the buffer while it's mapped.
	devcon->Map(pVBuffer,									// A pointer to the vertex buffer interface.
		NULL,												// Index number of the subresource.
		D3D11_MAP_WRITE_DISCARD,							// Flag that specifies the CPU's read and write permissions for a resource. A value of the D3D11_MAP enumerated type, i.e., D3D11_MAP_WRITE_DISCARD: Resource is mapped for writing; the previous contents of the resource will be undefined. The resource must have been created with write access and dynamic usage. "Previous contents of buffer are erased, and new buffer is opened for writing" DirectxTutorial.com.
		NULL,												// Flag that specifies how the CPU should respond when an program calls the ID3D11DeviceContext::Map method on a resource that is being used by the GPU. A value of the D3D11_MAP_FLAG enumerated type. "D3D11_MAP_FLAG_DO_NOT_WAIT cannot be used with D3D11_MAP_WRITE_DISCARD or D3D11_MAP_WRITE_NOOVERWRITE" Microsoft.com. "It can be NULL or D3D11_MAP_FLAG_DO_NOT_WAIT. This flag forces the program to continue, even if the GPU is still working with the buffer" DirectxTutorial.com.
		&ms);												// A pointer to the mapped subresource D3D11_MAPPED_SUBRESOURCE structure for the mapped subresource. The Map member function initializes this structure with necessary information.
	memcpy(ms.pData, &OurVertices[0], bd.ByteWidth);		// Copy the vertex attributes from OurVertices to the vertex buffer.
	// D3D11DeviceContext::Unmap member function:
	//   Invalidate the pointer to a resource and re-enable the GPU's access to that resource. Disable the CPU's access to that resource.
	devcon->Unmap(pVBuffer,									// A pointer to the vertex buffer interface.
		NULL);												// A subresource to be unmapped.

	// End: 3. Create the vertex buffer and assign values to it from the variable OurVertices.

	//***
	// 4. Create the index buffer and assign values to it from the variable OurIndices.
	//***

	// Assign values to the buffer resource description D3D11_BUFFER_DESC structure's members. Any subordinate members (variable.member.subordinatemember) are described in the comments.
	bd.ByteWidth = sizeof(DWORD) * (PrimitivesTotal * 3);	// Assigned a value specifying the size of the buffer in bytes. Three indices in the index buffer, each pointing to one set of vertex attributes in the vertex buffer, describe each triangle primitive, and PrimitivesTotal is the total number of triangles comprising the object. Therefore PrimitivesTotal * 3.
	bd.Usage = D3D11_USAGE_DYNAMIC;							// Assigned a value that identifies how the buffer is expected to be read from and written to. Frequency of update is a key factor.	A value of the D3D11_USAGE enumerated type,			  i.e., D3D11_USAGE_DYNAMIC:	 A resource that is accessible by both the GPU (read only) and the CPU (write only). A dynamic resource is a good choice for a resource that will be updated by the CPU at least once per frame. To update a dynamic resource, use a Map member function.
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;					// Assigned values in any combination by a bitwise OR operation specifying the flags for binding to graphics pipeline stages.		A value of the D3D11_BIND_FLAG enumerated type,		  i.e., D3D11_BIND_INDEX_BUFFER: Bind a buffer as an index buffer to the input-assembler stage of the graphics pipeline.
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;				// Assigned values in any combination by a bitwise OR operation specifying the flags for binding to graphics pipeline stages.		A value of the D3D11_CPU_ACCESS_FLAG enumerated type, i.e., D3D11_CPU_ACCESS_WRITE:	 The resource is to be mappable so that the CPU can change its contents. Resources created with this flag cannot be set as outputs of the graphics pipeline and must be created with either dynamic or staging usage (see D3D11_USAGE).

	// ID3D11Device::CreateBuffer member function:
	//   Create a buffer object (vertex buffer, index buffer, or shader constant buffer), in this case the index buffer object.
	dev->CreateBuffer(&bd,									// A pointer to a buffer resource description structure that describes the buffer, in this case an index buffer, as per bd.BindFlags = D3D11_BIND_INDEX_BUFFER.
		NULL,												// A pointer to a D3D11_SUBRESOURCE_DATA structure that describes the initialization data; use NULL to allocate space only (with the exception that it cannot be NULL if bd.Usage is D3D11_USAGE_IMMUTABLE).
		&pIBuffer);											// &pIBuffer is the address of a pointer, pIBuffer, to the buffer interface for the buffer object created, in this case an index buffer object.

	// Assign the index information by copying it from OurIndices to the index buffer.
	// ID3D11DeviceContext::Map member function:
	//   Mapping a buffer allows us to access it.
	//   Gets a pointer to the data contained in a subresource, and denies the GPU access to that subresource.
	//   The third parameter is a set of flags that allows us to control the CPUs access to the buffer while it's mapped.
	devcon->Map(pIBuffer,									// A pointer to the index buffer interface.
		NULL,												// Index number of the subresource.
		D3D11_MAP_WRITE_DISCARD,							// Flag that specifies the CPU's read and write permissions for a resource. A value of the D3D11_MAP enumerated type, i.e., D3D11_MAP_WRITE_DISCARD: Resource is mapped for writing; the previous contents of the resource will be undefined. The resource must have been created with write access and dynamic usage. "Previous contents of buffer are erased, and new buffer is opened for writing" DirectxTutorial.com.
		NULL,												// Flag that specifies how the CPU should respond when an program calls the ID3D11DeviceContext::Map method on a resource that is being used by the GPU. A value of the D3D11_MAP_FLAG enumerated type. "D3D11_MAP_FLAG_DO_NOT_WAIT cannot be used with D3D11_MAP_WRITE_DISCARD or D3D11_MAP_WRITE_NOOVERWRITE" Microsoft.com. "It can be NULL or D3D11_MAP_FLAG_DO_NOT_WAIT. This flag forces the program to continue, even if the GPU is still working with the buffer" DirectxTutorial.com.
		&ms);												// A pointer to the mapped subresource D3D11_MAPPED_SUBRESOURCE structure for the mapped subresource. The Map member function initializes this structure with necessary information.
	memcpy(ms.pData, &OurIndices[0], bd.ByteWidth);			// Copy the index information from OurIndices to the index buffer.
	// D3D11DeviceContext::Unmap member function:
	//   Invalidate the pointer to a resource and re-enable the GPU's access to that resource. Disable the CPU's access to that resource.
	devcon->Unmap(pIBuffer,									// A pointer to the index buffer interface.
		NULL);												// A subresource to be unmapped.

	// End: 4. Create the index buffer and assign values to it from the variable OurIndices.

	//***
	// 5. Create the texture image from an image file.
	//***

	// DirectX::CreateWICTextureFromFile function:
	//   Loads a WIC-supported bitmap file from disk, creates a Direct3D 11 resource from it, and optionally a Direct3D 11 shader resource view.
	CreateWICTextureFromFile(dev,							// A pointer to the device interface.
		L"Wood.png",										// The filename of the texture image file.
		NULL,												// NULL, as in most use cases for rendering you only need the shader resource view interface (the parameter below).
															// Otherwise, you would specify &pTexture:
															// &pTexture	 is the address of a pointer, pTexture,		to the resource				interface for the resource	  created, in this case a texture image.
		&pTextureView);										// &pTextureView is the address of a pointer, pTextureView, to the shader resource view interface for the subresource created, in this case a texture image.

	// ID3D11DeviceContext::PSSetShaderResources member function:
	//   Bind an array of shader resources to the pixel shader stage.
	devcon->PSSetShaderResources(0,							// Index into the device's zero-based array (in this case an array of one) to begin setting shader resources to.
		1,													// Number of shader resources to set.
		&pTextureView);										// &pTextureView is the address of a pointer, pTextureView, to the array of (in this case an array of one) shader resource view interfaces for the subresources created, in this case a texture image.

	// End: 5. Create the texture image from an image file.

	// End: InitGraphics function
	return 0;
}

// RenderFrame function: Definition
//   This function renders a single frame.
//     1. Define the final transformation matrix, matFinal, which contains all the information necessary to transform each vertex of the object being rendered.
//
//     2. Assign values that determine the attributes of light.
//
//     3. Clear the render target, in this case one back buffer texture interface, and the depth-stencil view interface, which effectively is the depth buffer (z-buffer).
//
//     4. Specify the vertex buffers, the index buffer, and the primitive type used when drawing.
//
//     5. Render the objects.
void RenderFrame(void)
{
	//***
	// 1. Define the final transformation matrix, matFinal, which contains all the information necessary to transform each geometric vertex of the object being rendered.
	//    The final matrix, matFinal = matWorld x matView x matProjection
	//
	//   i.	Define the world matrix, matWorld.
	//		What is the purpose of the world transformation?
	//		A model-to-world transformation, colloquially called a world transformation (or model transformation), converts model geometric vertices to world coordinates.
	//		In other words, it places a model in a world at an exact point defined by coordinates, and involves:
	//		1. Translation (movement)
	//		   It's defined by a single matrix (matTranslate), using a single DirectX function (XMMatrixTranslation).
	//		2. Rotation
	//		   It's defined by a single matrix (matRotate) that is itself defined by one to three matrices (matRotateX, matRotateY and / or matRotateZ), using one to three DirectX functions for x, y, and / or z axis rotation (XMMatrixRotationX, XMMatrixRotationY, and / or XMMatrixRotationZ).
	//		   If there are multiple rotation matrices, these are multiplied together to define matRotate.
	//		3. Scaling
	//		   It's defined by a single matrix (matScale), using a single DirectX function (XMMatrixScaling).
	//		World Matrix (matWorld) Examples:
	//		  matWorld = matScale x matRotateX
	//		  matWorld = matRotate x matScale x matTranslate (translate last, which is most commonly used)
	//		  matWorld = matTranslate x matRotate x matScale (translate first, providing a different result because when it is used first it offsets the object from the origin (0, 0, 0) before rotation takes place relative to the origin)
	//
	//  ii.	Define the view matrix, matView.
	//		What is the purpose of the view transformation?
	//		The view transformation converts model geometric vertices to view coordinates.
	//		View transformation can be considered similar to setting up a virtual camera, and involves:
	//		1. Position of the camera
	//		2. Location the camera is looking at
	//		3. Orientation of the camera
	//		   This is the direction of "up" for the camera; i.e., the direction pointing to the top of the screen. Usually, game programmers use the y-axis as the "up" direction.
	//		   To orient the camera this way, specify (0, 1, 0), or 1 on the y axis and 0 on the x and z axes; i.e., (0.0f, 1.0f, 0.0f).
	//		4. It's defined by a single matrix (matView), using a single DirectX function (XMMatrixLookAtLH).
	//
	// iii. Define the projection matrix, matProjection.
	//		What is the purpose of the projection transformation?
	//		The projection transformation converts model geometric vertices to screen coordinates.
	//		Projection transformation can be considered similar to setting up a camera lens, and involves:
	//		1. Field of View
	//		   In 3D graphics, the field of view is defined by setting the amount of radians allowed (vertically). The standard amount for this is 0.78539 (which is pi/4 radians, or 45 degrees).
	//		2. View-Plane Clipping
	//		   View-plane clipping omits the parts of an image that are unnecessary to draw; e.g., parts too far to see because of fog or the horizon.
	//		   Direct3D asks for a near view-plane and a far view-plane, and only draws the graphics that are between them; i.e., in the viewing frustum.
	//		3. It's defined by a single matrix (matProjection), using a single DirectX function (XMMatrixPerspectiveFovLH).
	//		   However, it's probably the most complex type of transformation, and the underlying math performed by the DirectX function is complicated.
	//
	//  iv.	Define the final transformation matrix, matFinal.
	//		matFinal = matWorld x matView x matProjection
	//		Each geometric vertex is multiplied by the final transformation matrix.
	//		The final transformation matrix is one member of the C++ constant buffer structure that matches the HLSL constant buffer structure. The C++ constant buffer structure will be copied to the HLSL constant buffer structure (they are, but do not have to be, named the same).
	//		Using the HLSL constant buffer is efficient, as multiplication and other common operations can be performed on its members by the GPU's vertex shader.
	//***

	// Declare transformation matrices that are not members of the C++ constant buffer structure.
	XMMATRIX matRotateY, matWorld, matView, matProjection;

	// Define the world matrix, matWorld.
	//   This matrix is updated each frame, causing the object to rotate.
	// XMMatrixRotationY function:
	//   Builds a matrix that rotates around the y-axis.
	static float Angle = 0.0f;								// "Angle" must be declared static so that its value is preserved though multiple calls to the function that declares it. This supports incremental frame by frame changes to the rendered object.
	Angle += 0.001f;										// This is the incremental frame by frame change to the rendered object.
	matRotateY = XMMatrixRotationY(Angle);					// "Angle" is the angle of rotation around the y-axis, in radians. Angles are measured clockwise when looking along the rotation axis toward the origin.
	ConstantBuffer.matRotate = matRotateY;					// The final rotation matrix is the product of all defined rotation matrices.				Here, only matRotateY is defined.
	matWorld = ConstantBuffer.matRotate;					// The world transformation is a function of translation (movement), rotation, and scaling. Here, only rotation   is defined.

	// Define the view matrix, matView.
	// XMMatrixLookAtLH function:
	//   Builds a view matrix for a left-handed coordinate system using a camera position, a focal point position, and the up direction of the camera.
	//   Returns a view matrix that transforms a point from world space into view space.
	// XMVectorSet function:
	//   Create a vector using four floating-point values: EyePosition
	//   Returns an instance XMVECTOR each of whose four components (x,y,z, and w) is a floating-point number with the same value as the corresponding input argument to XMVectorSet.
	//     XMVECTOR is a portable type used to represent a vector of four 32-bit floating-point or integer components, each aligned optimally and mapped to a hardware vector register.
	//       The DirectXMath.h header file (see objReader.h) uses aliases to the XMVECTOR object, specifically CXMVECTOR and FXMVECTOR.
	XMVECTOR EyePosition = XMVectorSet(0.0f,				// The x component of the vector to return.
		9.0f,												// The y component of the vector to return.
		24.0f,												// The z component of the vector to return.
		0.0f);												// The w component of the vector to return.
	XMVECTOR FocusPosition = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR UpDirection = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	matView = XMMatrixLookAtLH(EyePosition,					// Position of the camera.
		FocusPosition,										// Position of the focal point.
		UpDirection);										// Up direction of the camera, typically < 0.0f, 1.0f, 0.0f, 0.0f >.

	// Define the projection matrix, matProjection.
	// XMMatrixPerspectiveFovLH function:
	//   Builds a left-handed perspective projection matrix based on a field of view.
	float FovAngleY = XMConvertToRadians(45);				// The XMConvertToRadians function converts the size of an angle measured in degrees into one measured in radians.
	float AspectRatio = (FLOAT)SCREEN_WIDTH / (FLOAT)SCREEN_HEIGHT;
	float NearZ = 1.0f;
	float FarZ = 100.0f;
	matProjection = XMMatrixPerspectiveFovLH(FovAngleY,		// Top-down field-of-view angle in radians.
		AspectRatio,										// Aspect ratio of view-space X:Y.
		NearZ,												// Distance to the near clipping plane. Must be greater than zero.
		FarZ);												// Distance to the far clipping plane. Must be greater than zero.

	// Define the final transformation matrix, matFinal.
	ConstantBuffer.matFinal = matWorld * matView * matProjection;

	// End: 1. Define the final transformation matrix, matFinal, which contains all the information necessary to transform each geometric vertex of the object being rendered.

	//***
	// 2. Assign values that determine the attributes of light.
	//***

	ConstantBuffer.LightVector = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.0f);
	ConstantBuffer.LightColor = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	ConstantBuffer.AmbientColor = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);

	// Sample alternative values and their effect.
	//ConstantBuffer.LightVector = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f); // Dark
	//ConstantBuffer.LightVector = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); // Medium
	//ConstantBuffer.LightVector = XMFLOAT4(2.0f, 2.0f, 2.0f, 2.0f); // Medium
	//
	//ConstantBuffer.LightColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f); // Dark
	//ConstantBuffer.LightColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); // Medium
	//ConstantBuffer.LightColor = XMFLOAT4(2.0f, 2.0f, 2.0f, 2.0f); // Light
	//
	//ConstantBuffer.AmbientColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f); // Dark
	//ConstantBuffer.AmbientColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); // Medium
	//ConstantBuffer.AmbientColor = XMFLOAT4(2.0f, 2.0f, 2.0f, 2.0f); // Light

	// End: 2. Assign values that determine the attributes of light.

	//***
	// 3. Clear the render target, in this case one back buffer texture interface, and the depth-stencil view interface, which effectively is the depth buffer (z-buffer).
	//    Clearing the back buffer sets the color that fills the window into which the object is rendered.
	//    Clearing the depth buffer (z-buffer) with values closer to 0.0f reduces the distance to which triangles are drawn:
	//      0.0f:	No	triangles are drawn (or visible),	regardless of how close,   regardless of the coding of this program.
	//      0.97f:	Triangles at the rear of the object are not drawn (or visible).	   The effect of this specific value is dependent on the coding of this program, i.e., the position of triangles.
	//      1.0f:	All	triangles are drawn (and visible),	regardless of how distant, regardless of the coding of this program.
	//***

	// Clear the render target, in this case one back buffer texture interface, to a color that fills the window into which the object is rendered.
	float color[4] = { 0.0f, 0.2f, 0.4f, 1.0f };			// A 4-component array that represents the color using RGBA color values. RGBA color values are an extension of RGB color values, with an A (alpha channel) value added that specifies the opacity of a color. The alpha channel value is a number between 0.0 (fully transparent) and 1.0 (fully opaque). RGBA color values are specified as (red, green, blue, alpha).
	// ID3D11DeviceContext::ClearRenderTargetView member function:
	//   Set all the elements in a render target to one value.
	devcon->ClearRenderTargetView(backbuffer,				// A pointer to the render target view interface for the render target, in this case one back buffer texture interface (in this program only one back buffer is created).
		color);												// A 4-component array that represents the color to fill the render target with.

	// Clear the depth-stencil view interface, which effectively is the depth buffer (z-buffer).
	// ID3D11DeviceContext::ClearDepthStencilView member function:
	//   Clear the depth-stencil resource.
	devcon->ClearDepthStencilView(depthbuffer,				// A pointer to the depth-stencil view interface, which effectively is the depth buffer (z-buffer).
		D3D11_CLEAR_DEPTH,									// Flag that identifies the type of data to clear. A value of the D3D11_CLEAR_FLAG enumerated type, i.e., D3D11_CLEAR_DEPTH: Clear the depth buffer (z-buffer), using fast clear if possible, then place the resource in a compressed state.
		1.0f,												// Clear the depth buffer (z-buffer) with this value. This value will be clamped between 0 and 1.
		0);													// Clear the stencil buffer with this value.

	// End: 3. Clear the render target, in this case one back buffer texture interface, and the depth-stencil view interface, which effectively is the depth buffer (z-buffer).

	//***
	// 4. Specify the vertex buffers, the index buffer, and the primitive type used when drawing.
	//    Specify the vertex buffers to draw.
	//      This program uses only one vertex buffer.
	//    Specify the index buffer to use when drawing.
	//    Specify the primitive type we are using, i.e., the triangle primitive.
	//      Point, line, triangle and many other primitive types can be specified.
	//***

	// Specify the vertex buffers to draw.
	//   This program uses only one vertex buffer.
	UINT stride = sizeof(VERTEX);							// A "stride" is the size (in bytes) of the elements that are to be used from a vertex buffer.								  Define an array of strides when multiple vertex buffers are used.
	UINT offset = 0;										// An "offset" is the number of bytes between the first element of the vertex buffer and the first element that will be used. Define an array of offsets when multiple vertex buffers are used.
	// ID3D11DeviceContext::IASetVertexBuffers member function:
	//   Set the array of (in this case an array of one) vertex buffers to the input-assembler stage of the graphics pipeline.
	devcon->IASetVertexBuffers(0,							// The first input slot for binding. The first vertex buffer is explicitly bound to the start slot; this causes each additional vertex buffer in the array to be implicitly bound to each subsequent input slot.
		1,													// The number of vertex buffers in the array.
		&pVBuffer,											// &pVBuffer is the address of a pointer, pVBuffer, to an array of (in this case an array of one) vertex buffer interfaces.
		&stride,											// &stride is the address of stride, and therefore a pointer to the array of (in this case an array of one) stride values (one stride value for each buffer in the vertex buffer array).
		&offset);											// &offset is the address of offset, and therefore a pointer to the array of (in this case an array of one) offset values (one offset value for each buffer in the vertex buffer array).
	
	// Specify the index buffer to use when drawing.
	// ID3D11DeviceContext::IASetIndexBuffer member function:
	//   Set the index buffer to the input-assembler stage of the graphics pipeline.
	devcon->IASetIndexBuffer(pIBuffer,						// A pointer to the index buffer interface.
		DXGI_FORMAT_R32_UINT,								// A value of the DXGI_FORMAT enumerated type, i.e., DXGI_FORMAT_R32_UINT: A single-component, 32-bit unsigned-integer format that supports 32 bits for the red channel.
		0);													// The offset (in bytes) from the start of the index buffer to the first index to use.

	// Specify the primitive type we are using, i.e., the triangle primitive.
	// ID3D11DeviceContext::IASetPrimitiveTopology member function:
	//   Set information about the primitive type, and data order that describes input data for the input-assembler stage of the graphics pipeline.
	devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // A value of the  D3D11_PRIMITIVE_TOPOLOGY enumerated type, i.e., D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST: Interpret the vertex data as a list of triangles.

	// End: 4. Specify the vertex buffers, the index buffer, and the primitive type used when drawing..

	//***
	// 5. Render the objects.
	//   i.	Copy the C++ constant buffer structure to the HLSL constant buffer structure used by the GPU's vertex shader.
	//  ii. Draw the object's primitives to the back buffer.
	// iii. Switch the back buffer and the front buffer to present the rendered image to the user.
	//
	//    Each UpdateSubresource() and DrawIndexed() pair draws one instance of the object.
	//    A second instance of the same object is also drawn to the scene, offset from the first object.
	//***

	// Draw the first instance of the object to the scene.
	//
	// Prepare to draw the first instance of the object using the updated constant buffer.
	// ID3D11DeviceContext::UpdateSubresource member function:
	//   The CPU copies data from memory		   to a subresource created in non-mappable memory.
	//   Specifically:
	//   The CPU copies the C++ constant buffer	   to the HLSL constant buffer used by the GPU's vertex shader.
	devcon->UpdateSubresource(pCBuffer,						// A pointer to the destination resource, in this case the constant buffer interface.
		0,													// A zero-based index that identifies the destination subresource.
		0,													// A pointer to a box that defines the portion of the destination subresource to copy the resource data into. For a constant buffer, set this parameter to NULL, as it is not possible to use this member function to partially update a constant buffer.
		&ConstantBuffer,									// &ConstantBuffer is the address of ConstantBuffer, and therefore a pointer to the source data in memory, in this case the C++ constant buffer structure.
		0,													// The size of one row of the source data.
		0);													// The size of one depth slice of source data.
	//
	// Draw the first instance of the object using the updated constant buffer.
	// ID3D11DeviceContext::DrawIndexed member function:
	//   Draw indexed, non-instanced primitives.
	devcon->DrawIndexed(PrimitivesTotal * 3,				// Number of indices to draw. Three indices in the index buffer, each pointing to one set of vertex attributes in the vertex buffer, describe each triangle primitive, and PrimitivesTotal is the total number of triangles comprising the object. Therefore PrimitivesTotal * 3.
		0,													// The location of the first index read by the GPU from the index buffer.
		0);													// A value added to each index before reading a vertex from the vertex buffer.
	
	// Draw a second instance of the same object to the scene, using different world coordinates that offset it from the first instance of the object.
	//
	// *** This code that draws a second instance of the same object is written in a somewhat self-contained "one-off" style, compared to other code in this program, and for this reason can be improved.
	// *** For example, matTranslateY and Angle2 are declared and defined here, rather than at the start of this function like most other variables.
	//
	// Update the final transformation matrix, matFinal, using different world coordinates that offset the second instance of the object from the first instance of the object.
	// XMMatrixTranslation function:
	//   Builds a translation matrix from the specified offsets.
	// Translate the second instance of the object in a positive direction along the y-axis.
	XMMATRIX matTranslateY = XMMatrixTranslation(0.0f, 3.0f, 0.0f);
	// Rotate the second instance of the object counterclockwise.
	static float Angle2 = 0.0f; Angle2 -= 0.001f;			// Angle2 must be declared static so that its value is preserved though multiple calls of the function that contains it. This supports incremental frame by frame changes to the rendered object.
	matRotateY = XMMatrixRotationY(Angle2);					// Angle of rotation around the y-axis, in radians. Angles are measured clockwise when looking along the rotation axis toward the origin.
	ConstantBuffer.matRotate = matRotateY;					// The final rotation matrix is the product of all defined rotation matrices.				Here, only matRotateY is defined.
	matWorld = matTranslateY * ConstantBuffer.matRotate;	// The world transformation is a function of translation (movement), rotation, and scaling. Here, only translation and rotation are used.
	// Update the final transformation matrix, matFinal, by multiplying the new world matrix by the original view and projection matrices.
	ConstantBuffer.matFinal = matWorld * matView * matProjection;
	//
	// Prepare to draw the second instance of the object using the updated constant buffer.
	devcon->UpdateSubresource(pCBuffer,						// A pointer to the destination resource, in this case the constant buffer interface.
		0,													// A zero-based index that identifies the destination subresource.
		0,													// A pointer to a box that defines the portion of the destination subresource to copy the resource data into. For a constant buffer, set this parameter to NULL, as it is not possible to use this member function to partially update a constant buffer.
		&ConstantBuffer,									// &ConstantBuffer is the address of ConstantBuffer, and therefore a pointer to the source data in memory, in this case the C++ constant buffer structure.
		0,													// The size of one row of the source data.
		0);													// The size of one depth slice of source data.
	//
	// Draw the second instance of the object using the updated constant buffer.
	devcon->DrawIndexed(PrimitivesTotal * 3,				// Number of indices to draw. Three indices in the index buffer, each pointing to one set of vertex attributes in the vertex buffer, describe each triangle primitive, and PrimitivesTotal is the total number of triangles comprising the object. Therefore PrimitivesTotal * 3.
		0,													// The location of the first index read by the GPU from the index buffer.
		0);													// A value added to each index before reading a vertex from the vertex buffer.

	// Switch the back buffer and the front buffer.
	// IDXGISwapChain::Present member function:
	//   Present the rendered image to the user.
	swapchain->Present(0,									// An integer that specifies how to synchronize presentation of a frame with the vertical blank. '0' indicates the presentation occurs immediately,i.e., there is no synchronization.
		0);													// An integer value that contains swap-chain presentation options. These options are defined by the DXGI_PRESENT constants.
	
	// End: 5. Render the objects.

	// End: RenderFrame function
}

// CleanD3D function: Definition
//   This function performs an orderly termination of Direct3D.
//   1. Switch to windowed mode.
//
//   2. Free memory.
void CleanD3D(void)
{
	//***
	// 1. Switch to windowed mode.
	//    Switch to windowed mode, in case the user switched to full screen mode (this program starts in windowed mode).
	//    Direct3D is incapable of closing when in full screen mode. This is due to certain threading issues that occur behind the scenes.
	//    To correctly close down, we must make sure that we are in windowed mode.
	//***

	// IDXGISwapChain::SetFullscreenState member function:
	//   Set the display state to windowed or full screen.
	swapchain->SetFullscreenState(FALSE,					// TRUE for full screen, and FALSE for windowed.
		NULL);												// If you specify FALSE as the first parameter, then you must set this parameter to NULL.

	// End: 1. Switch to windowed mode.

	//***
	// 2. Free memory.
	//    Close Direct3D and release its memory.
	//    Deallocate any dynamically allocated objects, i.e., objects created with the new operator.
	//***

	// Close Direct3D and release its memory.
	// IUnknown::Release member function:
	//   Decrement the reference count for an interface on a COM object. If the reference count = 0, then the interface pointer is freed. If there are no other interface pointers, then the COM object is freed.
	pLayout->Release();
	pVS->Release();
	pPS->Release();
	depthbuffer->Release();
	pVBuffer->Release();
	pCBuffer->Release();
	pIBuffer->Release();
	swapchain->Release();
	backbuffer->Release();
	dev->Release();
	devcon->Release();

	// End: 2. Free memory.

	// End: CleanD3D function
}