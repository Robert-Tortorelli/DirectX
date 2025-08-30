The project objRenderer is a C++ program that parses one or more 3D object descriptions from one or more Wavefront .obj files, and renders those objects.
In its present intermediate form, the render results in the display of what appears as two rotating objects.
The first instance of the object is drawn at the origin of world space, i.e., (0, 0, 0). The second instance of the object is initially drawn above the first instance, at (xWorld, yWorld, zWorld) = (0, 3, 0).

Incrementing the value of z (variable zCamera) moves the camera's new position deeper into the physical screen.
The camera points at the second instance of the object even as it moves. Thus the second instance of the object appears stationary, while the first instance of the object (which is actually stationary) appears to move in the direction opposite to how the second instance of the object moves.
The W, A, S, D keys move the top object. However as the camera follows the top object, the bottom object that appears to move in the oposite direction. For example, W makes the bottom object appear to move -x.
  W moves the object +x
  S moves the object -x
  A moves the object +y
  D moves the object -y
The I, K keys move the camera.
  I moves the camera +z
  K moves the camera -z.

The program uses the Windows API (Win32 API) for general Windows program housekeeping, as well as a resource.rc resource file to create a title bar menu.
DirectX 11 is used to render 3D objects and 2D text.
DirectXTK is used to texture 3D objects.
