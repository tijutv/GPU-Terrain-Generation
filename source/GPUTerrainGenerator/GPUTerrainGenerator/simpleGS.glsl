#version 400
#extension GL_ARB_geometry_shader4 : enable

layout(triangles) in;

in vec3 tes_worldCoord[3];

out vec3 gs_normal;
out vec3 gs_worldCoord;

void main(void)
{
	// Calculate the normal for the triangle
	vec3 diff1 = tes_worldCoord[2] - tes_worldCoord[0];
	vec3 diff2 = tes_worldCoord[1] - tes_worldCoord[0];
	vec3 normal = normalize(cross(diff1, diff2));
	
	//Pass through the original vertex and assign normals
   for(int i=0; i<gl_VerticesIn; i++)
   {
	   gl_Position = gl_PositionIn[i];
	   gs_worldCoord = tes_worldCoord[i];
	   gs_normal = normal;
	   EmitVertex();
   }
   EndPrimitive();
}
