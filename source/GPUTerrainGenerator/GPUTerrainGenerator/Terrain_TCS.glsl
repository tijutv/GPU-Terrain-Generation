#version 400

layout(vertices = 3) out;

uniform float u_InnerTessLevel;
uniform vec3 u_OuterTessLevel;
uniform float u_InnerTessLevel2;
uniform vec3 u_OuterTessLevel2;

uniform float u_Near;
uniform float u_Far;
uniform float u_Left;
uniform float u_Right;
uniform float u_Top;
uniform float u_Bottom;
uniform float u_TessDistance;
uniform float u_TessDistance2;

uniform mat4 u_View;

in vec3 vs_Position[];

out vec3 tcs_Position[];

#define ID gl_InvocationID

void main(void)
{
	tcs_Position[ID] = vs_Position[ID];

	// Convert the point into view space and compare with camera coordinates
	vec4 posView = u_View * vec4(vs_Position[ID],1);
	
	if (posView.z <= u_Near && posView.z >= u_TessDistance2
		&& posView.x >= u_Left && posView.x <= u_Right
		&& posView.y >= u_Bottom && posView.y <= u_Top)
	{
		// Tessellate based on distance
		if (posView.z >= u_TessDistance)
		{
			gl_TessLevelInner[0] = u_InnerTessLevel;
			gl_TessLevelOuter[0] = u_OuterTessLevel.x;
			gl_TessLevelOuter[1] = u_OuterTessLevel.y;
			gl_TessLevelOuter[2] = u_OuterTessLevel.z;
		}
		else
		{
			gl_TessLevelInner[0] = mix(u_InnerTessLevel, u_InnerTessLevel2,     abs(posView.z - u_TessDistance)/abs(u_TessDistance2-u_TessDistance));
			gl_TessLevelOuter[0] = mix(u_OuterTessLevel.x, u_OuterTessLevel2.x, abs(posView.z - u_TessDistance)/abs(u_TessDistance2-u_TessDistance));
			gl_TessLevelOuter[1] = mix(u_OuterTessLevel.y, u_OuterTessLevel2.y, abs(posView.z - u_TessDistance)/abs(u_TessDistance2-u_TessDistance));
			gl_TessLevelOuter[2] = mix(u_OuterTessLevel.z, u_OuterTessLevel2.z, abs(posView.z - u_TessDistance)/abs(u_TessDistance2-u_TessDistance));
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
