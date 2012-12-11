#version 400

in vec3 gs_normal;
in vec3 gs_worldCoord;

out vec4 out_Color;

void main(void)
{
	//out_Color = vec4(texture(u_Noise, screenPos+vec2(1)).r,0,0,1);

	// Different colors based on height -- Start
	vec4 color0 = vec4(0,0.3,0.6,1.0);
	vec4 color1 = vec4(0.1,0.6,0.0,1.0);
	vec4 color2 = vec4(0.6,0.2,0.0,1.0);
	vec4 color3 = vec4(0.6,0.6,0.6,1.0);
	vec4 color4 = vec4(1.0);
	
	float compareVal = gs_worldCoord.y/10;

	vec4 mix0 = mix(color0, color1, smoothstep(0.0, 0.1, compareVal));
	vec4 mix1 = mix(color1, color2, smoothstep(0.1, 0.2, compareVal));
	vec4 mix2 = mix(color2, color3, smoothstep(0.2, 0.5, compareVal));
	vec4 mix3 = mix(color3, color4, smoothstep(0.5, 0.8, compareVal));
	vec4 mix4 = color4;

	vec4 colorHeight = mix0 * step(0.0, compareVal)  * (1.0-step(0.1, compareVal)) + 
		        mix1 * step(0.1, compareVal) * (1.0-step(0.2, compareVal))  +
				mix2 * step(0.2, compareVal)  * (1.0-step(0.5, compareVal))  +
				mix3 * step(0.5, compareVal)  * (1.0-step(0.8, compareVal))  +
				mix4 * step(0.8, compareVal);
	vec3 colorFinal = (vec3(dot(gs_normal,normalize(vec3(0,0,-1)))) + vec3(dot(gs_normal,normalize(vec3(0,-1,0)))) + vec3(dot(gs_normal,normalize(vec3(1,0,0)))))* vec3(colorHeight);
	colorFinal = clamp(colorFinal, 0.0, 1.0);
	out_Color = vec4(colorFinal, 1.0);
	// Different colors based on height -- End

	vec3 matColor = vec3(0.9,0.9,0.1);
	//out_Color = vec4(0.1,0.6,0.6,1.0);

	// Diffuse Shading
	//out_Color = vec4(vec3(dot(gs_normal,normalize(vec3(0,-1,0)))) * vec3(matColor), 1.0);

	//out_Color = vec4(vec3(1.0-abs(screenPos.z)/101.0),1);
}
