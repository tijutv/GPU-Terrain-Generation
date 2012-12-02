#version 400

layout(triangles) in;

uniform mat4 u_View;
uniform mat4 u_Persp;
uniform sampler2D u_Noise;

in vec3 tcs_Position[];

out vec3 tes_worldCoord;

void main(void)
{
	// gl_TessCoord is the barycentric coordinates of the original triangle
	// It gives the position of the new vertices of the triangles based on these values
	vec3 p0 = gl_TessCoord.x * tcs_Position[0];
	vec3 p1 = gl_TessCoord.y * tcs_Position[1];
	vec3 p2 = gl_TessCoord.z * tcs_Position[2];

	tes_worldCoord = p0 + p1 + p2;
	
	vec4 pos = u_Persp * u_View * vec4(tes_worldCoord, 1.0);
	gl_Position = pos/pos.w;
}
