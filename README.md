ModelViewer : An example viewing 3D models
==========================================

Required 3rdparty
-----------------
OpenCV ---- read texture images<br />
glut/freeglut ---- view 3D model in window context<br />
glew (ver 2.1.0) ---- use opengl extension<br />
glm (ver 0.9.9.2) ---- OpenGL Mathematics, only includes header files<br />
assimp ---- import 3D models<br />

Project directories
-------------------
include : declaration 3D model class<br />
src : implemention of 3D model class<br />
example : implemention of viewer<br />

Build project
-------------
Linux : simply run "sh build.sh" in terminal<br />
Windows : use Visual Studio 2013 or higher (vs2015 is recommended) to build the project<br />

Usage
-----
use command to open 3D model file :<br />
Linux : ./ModelViewer 3Dmodel_path<br />
Windows : ModelViewer.exe 3Dmodel_path<br />
(for example : ./ModelViewer /usr/share/scene.obj)<br />

use key and mouse to translate and rotate model :<br />
'R' : make the model pose initialized<br />
'L' : turn on/off the light<br />
'W','A','S','D' : make the model translate up, left, down, right<br />
'F','N' : make the model translate far, near<br />
'Q','E' : make the model rotate along Z-axis<br />
press left mouse button and drag : make the model rotate along X-axis and Y-axis
