GPU-Terrain-Generation
======================

Procedural Terrain Generation using the GPU

Blog
======================
http://GPUTerrain.blogspot.com/

Goal
======================
Terrain Generation on the GPU with increasing level of detail using tessellation depending on the camera position.

Controls
======================
case 'z':
case 'Z':
Move forward in z

case 'x':
case 'X':
	Move backward in z

case 'w':
case 'W':
	Move up in y

case 's':
case 'S':
	Move down in z

case 'a':
case 'A':
	Move left in x

case 'd':
case 'D':
	Move right in x

case 'm':
	Display Mesh

case 'M':
	Display Shaded Terrain

case 'i':
	Decrease Inner Tesselation

case 'I':
	Increase Inner Tesselation

case 'o':
	Decrease Outer Tesselation

case 'O':
	Decrease Outer Tesselation

case 'k':
	Decrease Inner Tesselation 2

case 'K':
	Increase Inner Tesselation 2

case 'l':
	Decrease Outer Tesselation 2

case 'L':
	Increase Outer Tesselation 2

case 't':
case 'T':
	Default Tessellation

case 'c':
case 'C':
	Display Occlusion

case 'v':
case 'V':
	Display Depth

case 'n':
case 'N':
	Display normals

case 'f':
	Display Fog

case 'F':
	Don't display fog

case 'g':
	Display fog based on depth

case 'G':
	Display fog based on height

case 'q':
case 'Q':
	Disco Light effect On/Off - Click anywhere after enabling to get different colors

case 'b':
	Deform - blast mode.
	After selecting 'b' click on the screen to get the blast effect

case 'h':
case 'H':
	Display using height map

case 'r':
	noiseLacunarity Decrease

case 'R':
	noise Lacunarity Increase

case 'p':
	noise Gain Decrease

case 'P':
	noise Gain Increase

References
======================
<ul>
<li>GPU Gems 3 - Generating Complex Procedural Terrains Using the GPU (http://http.developer.nvidia.com/GPUGems3/gpugems3_ch01.html)</li>
<li>http://outerra.blogspot.com/search/label/fractal%20terrain - My Lecturer for the course, Patrick Cozzi, sent me this link for reference</li>
</ul>
