#version 400

layout(vertices = 3) out;

uniform float u_InnerTessLevel;
uniform vec3 u_OuterTessLevel;

uniform float u_Near;
uniform float u_TessDistance;

in vec3 vs_Position[];

out vec3 tcs_Position[];

#define ID gl_InvocationID

void main(void)
{
	tcs_Position[ID] = vs_Position[ID];

	if (vs_Position[ID].z <= u_Near && vs_Position[ID].z >= u_TessDistance)
	{
		// Set the inner and outer tessellation levels
		//if (ID == 0)
		{
			gl_TessLevelInner[0] = u_InnerTessLevel;
			gl_TessLevelOuter[0] = u_OuterTessLevel.x;
			gl_TessLevelOuter[1] = u_OuterTessLevel.y;
			gl_TessLevelOuter[2] = u_OuterTessLevel.z;
		}
	}
	else
	{
		// Set the inner and outer tessellation levels
		if (ID == 0)
		{
			gl_TessLevelInner[0] = 1.0f;
			gl_TessLevelOuter[0] = 1.0f;
			gl_TessLevelOuter[1] = 1.0f;
			gl_TessLevelOuter[2] = 1.0f;
		}
	}
}
