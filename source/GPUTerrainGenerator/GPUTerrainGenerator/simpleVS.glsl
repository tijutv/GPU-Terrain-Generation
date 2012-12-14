#version 400

uniform sampler2D u_Noise;
uniform int u_NoiseOctaves;
uniform float u_NoiseLacunarity;
uniform float u_NoiseGain;

uniform sampler2D u_HeightMap;
uniform float u_UseHeightMap;
uniform float u_LeftMeshMin;
uniform float u_NearMeshMin;
uniform vec2 u_MeshSize;

in vec4 Position;

out vec3 vs_Position;

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
	if (u_UseHeightMap > 0)
	{
		vec2 texCoord = vec2((Position.x + 512.0)/1024.0, (Position.z - 1.0)/1024.0);
		float height = texture(u_HeightMap, texCoord).r;
		height *= 30.0;
		vs_Position = vec3(Position.x, Position.y+height, Position.z);
	}
	else
	{
		float height = turbulence(u_NoiseOctaves, Position.xyz, u_NoiseLacunarity, u_NoiseGain);
		height -= 0.02;
		height = max(height, 0.0);
		height *= 30;

		/*height = RidgedMultiFractal(Position.xyz, 1.0, 2.2, 5, 1.0, 2.0);
		height = step(0.5, height) * height;
		height *= 0.5;
		height = clamp(height, 0.0, 1.0);
		height *= 10;
		height = max(height, 0.0);*/

		vs_Position = vec3(Position.x, Position.y+height, Position.z);
		//vs_Position = Position.xyz;
	}
}