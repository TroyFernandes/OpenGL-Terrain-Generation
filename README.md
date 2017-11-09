# Robot Kinematics with Interactive Terrain Generation
Program written in C and utilizes OpenGL to allow the user to create complex smooth holes on a quadrilateral mesh using an interactive robot. Kinematic calculations are used as a simple form of collision detection to decide where to make the holes.

To compile the program, download and open with Visual Studio. Visual Studio 2017 RC was used to create the program. You can press the F1 key when the program is running to get a help screen

# Robot Control
   1. Shoulder Yaw: Rotates the base of the robot around the world Y-Axis
   2. Shoulder Pitch: Shoulder of the robot can be raised or lowered
   3. Elbow Pitch: Arm of the robot can be raised or lowered
   
# Scene Control
Users can use the mouse and keyboard to Zoom in/out, Rotate the scene left/right and up/down

# Kinematics and Collision Detection

Composition of all transformations are calculated to determine the location of the tip of the robot (in this case the digger object).

This is acheived by Left Multiplying transformations on a 4x4 Homogeneous Identity Matrix and finally multiplying by a vector which represents a point V(0,0,0)

Collision detection is calculated based on the actual location on the X-Z plane, and not simply at X=0 & Z=0. This means that holes get progressively deeper the lower the digger goes.

Holes created are cumulative which allows the creation of multiple holes and deeper holes.

# Hole Creation
Holes are created using a guassian function. Given a location on the X-Z plane, the function calculates the proper Y value and updates the mesh

![](https://latex.codecogs.com/gif.latex?f%28x%2Cz%29%20%3D%20%5Csum_%7Bk%7D%5E%7B%20%7Db_%7Bk%7De%5E%7B-a_%7Bk%7Dr%5E%7B2%7D_%7Bk%7D%7D)


k = holes

r = distance from mesh vertex to k<sup>th</sup> hole

b = height of hole(negative values creates holes)

a = width of hole

