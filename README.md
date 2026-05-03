The project objRenderer is a C++ program that parses one or more 3D object descriptions from one or more Wavefront .obj files, and renders those objects.
In order to be usable by this program each Wavefront .obj file must contain geometric vertex statements (v), vertex texture coordinate statements (vt), vertex normal vector statements (vn), and their associated face element statements (f).
A Wavefront .obj file that only contains geometric vertex statements and their associated face element statements, but not vertex texture coordinate statements and vertex normal vector statements, is a valid Wavefront .obj file but is not usable by this program.

In its present intermediate form, two instances of a rotating object are rendered from a single Wavefront .obj file.
The first instance of the object is initially drawn at the origin of world space (0, 0, 0). The second instance of the object is initially drawn above the first object at (0, 3, 0).

Incrementing the z coordinate of the camera's position moves the camera deeper into the physical screen, such that world objects appear further away from the end-user.
The camera always looks at the second instance of the object, following it when it is moved. Therefore the second instance of the object always appears stationary, while the first instance of the object (which is stationary) appears to move in the direction opposite to how the second instance of the object moves.
The W, A, S, D keys move the second object. Because the camera follows the second object, the W key makes the first object appear to move in the -x direction. Similarly for the other keys.

  W moves the object +x
  
  S moves the object -x
  
  A moves the object +y
  
  D moves the object -y
  
The I, K keys move the camera.

  I moves the camera +z
  
  K moves the camera -z.
  
The program uses the Windows API (Win32 API).
DirectX 11 is used to render 3D objects and 2D text.
DirectXTK is used to texture 3D objects.
