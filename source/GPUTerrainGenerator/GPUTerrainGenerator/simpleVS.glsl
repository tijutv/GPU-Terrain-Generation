#version 330

in vec4 Position;

uniform mat4 u_View;
uniform mat4 u_Persp;
uniform sampler2D u_Noise;

in vec2 Texcoords;

out vec3 screenPos; // z has the depth value
out vec2 fs_Texcoord;
out vec3 worldCoord;

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



void main(void)
{
	fs_Texcoord = Texcoords;
	
	float height = turbulence(4, Position.xyz, 0.14, 0.35);
	height -= 0.02;
	height = max(height, 0.0);
	height *= 30;
	vec4 pos = vec4(Position.x, Position.y+height, Position.z, 1.0);
	worldCoord = pos.xyz;
	pos = u_Persp*u_View*pos;
	gl_Position = pos/pos.w;
	screenPos = vec3(gl_Position.xy, worldCoord.z);
}