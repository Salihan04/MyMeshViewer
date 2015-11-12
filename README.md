# My Mesh Viewer
The Mesh Viewer is an easy to use lightweight application for displaying three dimensional models (triangle meshes). It uses OpenGL to render the models. In this assignment, you are given several 3D models in Hugues Hoppe’s M file format. Triangle meshes can be displayed solid, as a wire frame (all lines or just front lines), or as point clouds. The surface normals of the triangles can be displayed optionally. Loaded models can be rotated, translated and scaled (all done with the mouse). The model is lit by one or multiple light sources. Users can also change the type of the projection (orthogonal projection and perspective projection).

<b>Note: </b>This project was done as a school assignment. GLUT was used in this project.

##Instructions
*	Press keys 1-8 to load a model.
  *	1: bimba
  *	2: bottle
  *	3: bunny
  *	4: cap
  *	5: eight
  *	6: gargoyle
  *	7: knot
  *	8: statute
*	Press key ‘p’ to render the model in point cloud mode
*	Press key ‘w’ to render the model in wireframe mode
*	Press key ‘f’ to render the model in flat shading mode
*	Press key ‘s’ to render the model in smooth shading mode
*	Press key ‘r’ to reset any transformation
*	Press Spacebar key to switch projection from perspective to orthogonal and vice versa
*	Press Up Arrow key to increase brightness/intensity of light
*	Press Down Arrow key to decrease brightness/intensity of light
*	Hold down Left mouse button for rotation
  *	Move mouse vertically to rotate about x-axis
  *	Move mouse horizontally to rotate about y-axis
  *	Move mouse in a circular motion to rotate about z-axis
*	Hold down Middle mouse button for translation
  *	Move mouse vertically to translate along y-axis
  *	Move mouse horizontally to translate along x-axis
*	Hold down Right mouse button for scaling
  *	Move mouse horizontally
    *	Left to scale/zoom out
    *	Right to scale/zoom in

##Screenshots
###1) Render as Point Cloud
![Point Cloud](https://github.com/Salihan04/MyMeshViewer/raw/master/Screenshots/ModelPoint.PNG)
###2) Render as Wireframe
![Wireframe](https://github.com/Salihan04/MyMeshViewer/raw/master/Screenshots/ModelWireframe.PNG)
###3) Render in Flat Shading Mode
![Flat Shading](https://github.com/Salihan04/MyMeshViewer/raw/master/Screenshots/ModelFlat.PNG)
###4) Render in Smooth Shading Mode
![Smooth Shading](https://github.com/Salihan04/MyMeshViewer/raw/master/Screenshots/ModelSmooth.PNG)

##To Do
* Upgrade code to use modern OpenGL and shaders
* Implement GUI using libraries such as GLUI or any other GUI libraries
