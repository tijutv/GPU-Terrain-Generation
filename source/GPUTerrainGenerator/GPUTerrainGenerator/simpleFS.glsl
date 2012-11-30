#version 330

out vec4 out_Color;
uniform sampler2D u_Noise;

in vec2 fs_Texcoord;
in vec3 screenPos;
in vec3 worldCoord;

void main(void)
{
	//out_Color = vec4(texture(u_Noise, screenPos+vec2(1)).r,0,0,1);
	if (worldCoord.y > 8)
		out_Color = vec4(1.0);
	else if (worldCoord.y > 5)
		out_Color = vec4(0.6,0.6,0.6,1.0);
	else if (worldCoord.y > 2)
		out_Color = vec4(0.6,0.2,0.0,1.0);
	else if (worldCoord.y > 0.02)
		out_Color = vec4(0.1,0.6,0.0,1.0);
	else
		out_Color = vec4(0,0.3,0.6,1.0);
	//out_Color = vec4(0.6,0.6,0.6,1.0);
}
