#version 400

layout(triangles) in;

const unsigned int numDeforms = 20;

uniform mat4 u_View;
uniform mat4 u_Persp;
uniform sampler2D u_Noise;
uniform float u_Deform;
uniform vec4 u_DeformPosArr[numDeforms];
uniform sampler2D u_HeightMap;
uniform float u_UseHeightMap;

uniform sampler2D u_RandomScalartex;

in vec3 tcs_Position[];

out vec3 tes_worldCoord;
out vec4 tes_Position;

// Noise Code adapted from http://www.sci.utah.edu/~leenak/IndStudy_reportfall/PNoiseCode.txt
#define ONE 0.00390625
#define ONEHALF 0.001953125

float fade(float t) {
  //return t*t*(3.0-2.0*t); // Old fade
  return t*t*t*(t*(t*6.0-15.0)+10.0); // Improved fade
}
 
float noise(vec3 P)
{
  vec3 Pi = ONE*floor(P)+ONEHALF; 
                                 
  vec3 Pf = P-floor(P);
  
  // Noise contributions from (x=0, y=0), z=0 and z=1
  float perm00 = texture(u_Noise, Pi.xy).a ;
  vec3  grad000 = texture(u_Noise, vec2(perm00, Pi.z)).rgb * 4.0 - 1.0;
  float n000 = dot(grad000, Pf);
  vec3  grad001 = texture(u_Noise, vec2(perm00, Pi.z + ONE)).rgb * 4.0 - 1.0;
  float n001 = dot(grad001, Pf - vec3(0.0, 0.0, 1.0));

  // Noise contributions from (x=0, y=1), z=0 and z=1
  float perm01 = texture(u_Noise, Pi.xy + vec2(0.0, ONE)).a ;
  vec3  grad010 = texture(u_Noise, vec2(perm01, Pi.z)).rgb * 4.0 - 1.0;
  float n010 = dot(grad010, Pf - vec3(0.0, 1.0, 0.0));
  vec3  grad011 = texture(u_Noise, vec2(perm01, Pi.z + ONE)).rgb * 4.0 - 1.0;
  float n011 = dot(grad011, Pf - vec3(0.0, 1.0, 1.0));

  // Noise contributions from (x=1, y=0), z=0 and z=1
  float perm10 = texture(u_Noise, Pi.xy + vec2(ONE, 0.0)).a ;
  vec3  grad100 = texture(u_Noise, vec2(perm10, Pi.z)).rgb * 4.0 - 1.0;
  float n100 = dot(grad100, Pf - vec3(1.0, 0.0, 0.0));
  vec3  grad101 = texture(u_Noise, vec2(perm10, Pi.z + ONE)).rgb * 4.0 - 1.0;
  float n101 = dot(grad101, Pf - vec3(1.0, 0.0, 1.0));

  // Noise contributions from (x=1, y=1), z=0 and z=1
  float perm11 = texture(u_Noise, Pi.xy + vec2(ONE, ONE)).a ;
  vec3  grad110 = texture(u_Noise, vec2(perm11, Pi.z)).rgb * 4.0 - 1.0;
  float n110 = dot(grad110, Pf - vec3(1.0, 1.0, 0.0));
  vec3  grad111 = texture(u_Noise, vec2(perm11, Pi.z + ONE)).rgb * 4.0 - 1.0;
  float n111 = dot(grad111, Pf - vec3(1.0, 1.0, 1.0));

  // Blend contributions along x
  vec4 n_x = mix(vec4(n000, n001, n010, n011), vec4(n100, n101, n110, n111), fade(Pf.x));

  // Blend contributions along y
  vec2 n_xy = mix(n_x.xy, n_x.zw, fade(Pf.y));

  // Blend contributions along z
  float n_xyz = mix(n_xy.x, n_xy.y, fade(Pf.z));
 
  return n_xyz;
}

float turbulence(int octaves, vec3 P, float lacunarity, float gain)
{	
  float sum = 0;
  float scale = 1;
  float totalgain = 1;
  for(int i=0;i<octaves;i++){
    sum += totalgain*noise(P*scale);
    scale *= lacunarity;
    totalgain *= gain;
  }
  return abs(sum);
}



// Rigid MultiFractal Terrain Model - from the book "Texturing & Modeling: A Procedural Approach"
float RidgedMultiFractal(vec3 point, float H, float lacunarity, float octaves, float offset, float gain)
{
	float result, frequency, signal, weight;
	int i;
	bool first = true;
	float exponentArray[5];

	frequency = 1.0;
	if (first)
	{
		for (int i = 0; i<5; ++i)
		{
			exponentArray[i] = pow(frequency, -H);
			frequency *= lacunarity;
		}

		first = false;
	}

	/* get first octave */
	signal = noise( point );
	/* get absolute value of signal (this creates the ridges) */
	if ( signal < 0.0 ) signal = -signal;
	/* invert and translate (note that "offset" should be ~= 1.0) */
	signal = offset - signal;
	/* square the signal, to increase "sharpness" of ridges */
	signal *= signal;
	/* assign initial values */
	result = signal;
	weight = 1.0;

	for( i=1; i<octaves; i++ ) 
	{
		/* increase the frequency */
		point.x *= lacunarity;
		point.y *= lacunarity;
		point.z *= lacunarity;

		/* weight successive contributions by previous signal */
		weight = signal * gain;
		if ( weight > 1.0 ) weight = 1.0;
		if ( weight < 0.0 ) weight = 0.0;
		signal = noise( point );
		if ( signal < 0.0 ) signal = -signal;
		signal = offset - signal;
		signal *= signal;
		/* weight the contribution */
		signal *= weight;
		result += signal * exponentArray[i];
	}

	return result;
}

void main(void)
{
	// gl_TessCoord is the barycentric coordinates of the original triangle
	// It gives the position of the new vertices of the triangles based on these values
	vec3 p0 = gl_TessCoord.x * tcs_Position[0];
	vec3 p1 = gl_TessCoord.y * tcs_Position[1];
	vec3 p2 = gl_TessCoord.z * tcs_Position[2];

	tes_worldCoord = p0 + p1 + p2;

	// Tessellate and give random height to terrrain
	float height;
	//if (u_UseHeightMap > 0)
	//{
	//	vec2 texCoord = vec2((tes_worldCoord.x + 512.0)/1024.0, (tes_worldCoord.z - 1.0)/1024.0);
	//	height = texture(u_HeightMap, texCoord).r;
	//	//height = clamp(height, 0.0, 0.7);
	//	height *= 7.0;
	//	//vs_Position = vec3(Position.x, Position.y+height, Position.z);
	//}
	//else
	//{
		height = turbulence(4, vec3(tes_worldCoord.x, 0.0, tes_worldCoord.z), 0.07, 0.35);
		//height -= 0.02;
		height = clamp(height, 0.0, 0.7);
		if ((tes_worldCoord.y+height) < 1.0)
		{
			height *= 0.2;
			//height = 0.0;
		}
		//height *= 30;
	//}
	tes_worldCoord = vec3(tes_worldCoord.x, tes_worldCoord.y+height, tes_worldCoord.z);

	// Deforming terrain
	if (u_DeformPosArr[0].w > 0)
	{
		for (int i = 0; i<numDeforms; ++i)
		{
			if (u_DeformPosArr[i].w > 0)
			{
				float deformRadius = u_DeformPosArr[i].w;
				vec3 currDeformPos = u_DeformPosArr[i].xyz;
				vec3 minVal = currDeformPos - vec3(deformRadius, 0, deformRadius);
				vec3 maxVal = currDeformPos + vec3(deformRadius, 0, deformRadius);
				vec2 diff = vec2(tes_worldCoord.x, tes_worldCoord.z) - vec2(currDeformPos.x, currDeformPos.z);
				if (length(diff) < deformRadius)
				{
					float blowHeight = turbulence(4, vec3(currDeformPos.x, 0.0, currDeformPos.z), 0.07, 0.35);
					blowHeight = clamp(blowHeight, 0.0, 0.8);
					blowHeight *= 10.0;
					//tes_worldCoord.y -= (0.95+2*clamp(height,0.0,0.2));
					if (tes_worldCoord.y > (blowHeight + clamp(height, 0.0, 0.4)))
					{
						tes_worldCoord.y = min(tes_worldCoord.y, blowHeight);
						tes_worldCoord.y = max(tes_worldCoord.y,tes_worldCoord.y + clamp(height, 0.0, 0.4));
					}
				}
			}
		}
	}

	tes_Position = u_View * vec4(tes_worldCoord, 1.0);
	vec4 pos = u_Persp * tes_Position;
	gl_Position = pos;///pos.w;
}
