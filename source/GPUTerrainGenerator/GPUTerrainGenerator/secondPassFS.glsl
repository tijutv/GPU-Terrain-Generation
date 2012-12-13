#version 400

#define	DISPLAY_MESH 0
#define	DISPLAY_SHADED 1
#define	DISPLAY_DEPTH 2
#define	DISPLAY_FOG 3
#define	DISPLAY_LOWER_FOG 4
#define	DISPLAY_DEPTH_FOG 5
#define	DISPLAY_OCCLUSION 6
#define	DISPLAY_NORMAL 7

in vec2 fs_Texcoord;

uniform sampler2D u_Depthtex;
uniform sampler2D u_Normaltex;
uniform sampler2D u_Positiontex;
uniform sampler2D u_Colortex;
uniform sampler2D u_WorldPostex;
uniform sampler2D u_RandomNormaltex;
uniform sampler2D u_RandomScalartex;

uniform float u_NearModel;
uniform float u_FarModel;
uniform int u_ScreenWidth;
uniform int u_ScreenHeight;
uniform int u_Display;
uniform int u_DisplayFog;
uniform mat4 u_Persp;

out vec4 out_Color;

#define PI 3.1415926535897

//Depth used in the Z buffer is not linearly related to distance from camera
//This restores linear depth
float linearizeDepth(float exp_depth, float near, float far)
{
    return	(2 * near) / (far + near -  exp_depth * (far - near)); 
}

//Helper function to automatically sample and unpack normals
vec3 sampleNrm(vec2 texcoords) {
	return texture(u_Normaltex,texcoords).xyz;
}

//Helper function to automatically sample and unpack positions
// u_Positiontex values range from (-ScreenWidth/2 to ScreenWidth/2), (-ScreenHeight/2 to ScreenHeight/2), (near to far)
vec3 samplePos(vec2 texcoords) {
	return texture(u_Positiontex,texcoords).xyz;
}

vec3 sampleWorldPos(vec2 texcoords) {
	return texture(u_WorldPostex,texcoords).xyz;
}

//Helper function to automatically sample and unpack positions
vec3 sampleDepth(vec2 texcoords) {
	return texture(u_Depthtex,texcoords).xyz;
}


//Get a random normal vector  given a screen-space texture coordinate
//Actually accesses a texture of random vectors
vec3 getRandomNormal(vec2 texcoords) {
	ivec2 sz = textureSize(u_RandomNormaltex,0);
	return texture(u_RandomNormaltex,vec2(texcoords.s* (u_ScreenWidth)/sz.x,
		(texcoords.t)*(u_ScreenHeight)/sz.y)).rgb;
}


//Get a random scalar given a screen-space texture coordinate
//Fetches from a random texture
float getRandomScalar(vec2 texcoords) {
	ivec2 sz = textureSize(u_RandomScalartex,0);
	return texture(u_RandomScalartex,vec2(texcoords.s*u_ScreenWidth/sz.x,
		texcoords.t*u_ScreenHeight/sz.y)).r;
}

vec3 calcNormalBasedOnScreenData(vec2 texcoords)
{
	// Calculate the normals for the terrain
	float offsetX = 1.f/u_ScreenWidth;
	float offsetY = 1.f/u_ScreenHeight;
	vec3 normalX = samplePos(vec2(texcoords.x + offsetX, texcoords.y)) - samplePos(vec2(texcoords.x - offsetX, texcoords.y));
	vec3 normalY = samplePos(vec2(texcoords.x, texcoords.y + offsetY)) - samplePos(vec2(texcoords.x, texcoords.y - offsetY));

	float scaleX = 2.f * offsetX;
	float scaleY = 2.f * offsetY;
	float scaleZ = 1.0/abs(u_FarModel - u_NearModel);
	normalX *= vec3(scaleX, scaleY, -scaleZ);
	normalY *= vec3(scaleX, scaleY, -scaleZ);
	/*normalX.x = sqrt(scaleZ);
	normalX.y = 0;
	normalY.x = 0;
	normalY.y = sqrt(scaleZ);*/

	vec3 normal = normalize(cross(normalX, normalY));
	normal.y = - normal.y;

	return normal;
}



////////////////////////////////////////////////
/// BEGIN TODO: START HERE
//////////////////////////////////////////////

//Estimate occlusion based on a point and a sampled occluder
//Design this function based on specified constraints
float gatherOcclusion( vec3 pt_normal,
	vec3 pt_position,
	vec3 occluder_normal,
	vec3 occluder_position) {

	//if (occluder_position.z >= pt_position.z)
		//return 0.0;

	float depthDiff = (-pt_position.z + occluder_position.z)/abs(u_NearModel - u_FarModel);
	//if (depthDiff < 0.001)
		//return 0.0;

	float normDiff = (1.0-dot(occluder_normal, pt_normal));
	return 2.0*(step(0.001, depthDiff)*normDiff*(1.0-smoothstep(0.001, 0.08, depthDiff)));
	//return abs(depthDiff);

	//return -1.0f;///IMPLEMENT THIS
}

const float REGULAR_SAMPLE_STEP = 0.0012f;
float occlusionWithRegularSamples(vec2 texcoord, 
	vec3 position,
    vec3 normal) {
	
	float occlusion = 0.0;
	//normal = normalize(normal);
	for (float x = -1.5*REGULAR_SAMPLE_STEP; x<= 1.5*REGULAR_SAMPLE_STEP; x+= REGULAR_SAMPLE_STEP)
	{
		for (float y = -1.5*REGULAR_SAMPLE_STEP; y<= 1.5*REGULAR_SAMPLE_STEP; y+= REGULAR_SAMPLE_STEP)
		{
			vec2 occluderTexcoord = vec2(texcoord.x+x, texcoord.y+y);
			vec3 occluderPosition = samplePos(occluderTexcoord);
			//vec3 occluderNormal = sampleNrm(occluderTexcoord);
			// occluderNormal = normalize(occluderNormal);
			vec3 occluderNormal = calcNormalBasedOnScreenData(occluderTexcoord);
			occlusion += gatherOcclusion(normal, position, occluderNormal, occluderPosition);
		}
	}

	return occlusion * (1.0/16.0); 
	//return -1.0f; //IMPLEMENT THIS
}


//Disk of samples for the SS sampling
#define NUM_SS_SAMPLES 16
vec2 poissonDisk[NUM_SS_SAMPLES] = vec2[]( 
    vec2( -0.94201624, -0.39906216 ), 
	vec2( 0.94558609, -0.76890725 ), 
	vec2( -0.094184101, -0.92938870 ), 
	vec2( 0.34495938, 0.29387760 ), 
	vec2( -0.91588581, 0.45771432 ), 
	vec2( -0.81544232, -0.87912464 ), 
	vec2( -0.38277543, 0.27676845 ), 
	vec2( 0.97484398, 0.75648379 ), 
	vec2( 0.44323325, -0.97511554 ), 
	vec2( 0.53742981, -0.47373420 ), 
	vec2( -0.26496911, -0.41893023 ), 
	vec2( 0.79197514, 0.19090188 ), 
	vec2( -0.24188840, 0.99706507 ), 
	vec2( -0.81409955, 0.91437590 ), 
	vec2( 0.19984126, 0.78641367 ), 
	vec2( 0.14383161, -0.14100790)
);

const float SS_RADIUS = 0.02f;
float occlusionWithPoissonSSSamples(vec2 texcoord, 
	vec3 position,
    vec3 normal) {

	float occlusion = 0.0;
	normal = normalize(normal);
	float randVal = getRandomScalar(texcoord)*2.0*PI;
	float cosVal = cos(randVal);
	float sinVal = sin(randVal);

	for(int i=0; i<NUM_SS_SAMPLES; ++i)
	{
		vec2 occluderTexcoord = vec2(texcoord.x+SS_RADIUS*(cosVal*poissonDisk[i].x - sinVal*poissonDisk[i].y), 
									 texcoord.y+SS_RADIUS*(sinVal*poissonDisk[i].x + cosVal*poissonDisk[i].y));
		vec3 occluderPosition = samplePos(occluderTexcoord);
		//vec3 occluderNormal = sampleNrm(occluderTexcoord);
		//occluderNormal = normalize(occluderNormal);
		vec3 occluderNormal = calcNormalBasedOnScreenData(occluderTexcoord);
		occlusion += gatherOcclusion(normal, position, occluderNormal, occluderPosition);
	}
	
	return occlusion * (1.0/16.0); 

	//return -1.0f; //IMPLEMENT THIS
}


//Sphere of samples for the World Space sampling
#define NUM_WS_SAMPLES 16
vec3 poissonSphere[NUM_WS_SAMPLES] = vec3[](
	vec3(0.53812504, 0.18565957, -0.43192),
	vec3(0.13790712, 0.24864247, 0.44301823),
	vec3(0.33715037, 0.56794053, -0.005789503),
	vec3(-0.6999805, -0.04511441, -0.0019965635),
	vec3(0.06896307, -0.15983082, -0.85477847),
	vec3(0.056099437, 0.006954967, -0.1843352),
	vec3(-0.014653638, 0.14027752, 0.0762037),
	vec3(0.010019933, -0.1924225, -0.034443386),
	vec3(-0.35775623, -0.5301969, -0.43581226),
	vec3(-0.3169221, 0.106360726, 0.015860917),
	vec3(0.010350345, -0.58698344, 0.0046293875),
	vec3(-0.08972908, -0.49408212, 0.3287904),
	vec3(0.7119986, -0.0154690035, -0.09183723),
	vec3(-0.053382345, 0.059675813, -0.5411899),
	vec3(0.035267662, -0.063188605, 0.54602677),
	vec3(-0.47761092, 0.2847911, -0.0271716));



const float SPHERE_RADIUS = 0.3f;
float occlusionWithWorldSpaceSamples(vec2 texcoord, vec3 position, vec3 normal) 
{
	float occlusion = 0.0;
	normal = normal * 2.0 -1.0;
	normal = normalize(normal);
	vec3 randPlane = getRandomNormal(texcoord);
		
	for(int i=0; i<NUM_SS_SAMPLES; ++i)
	{
		vec3 newPoint = reflect(poissonSphere[i], randPlane);
		
		vec3 viewNewPoint = position + sign(dot(newPoint,normal)) * newPoint * SPHERE_RADIUS;
		vec4 screenPointHom = u_Persp * vec4(viewNewPoint,1.0);
		vec2 screenPoint = (screenPointHom.xy)/screenPointHom.w;
		//screenPoint += vec2(diffX,0.0);
		screenPoint *= vec2(0.55,1.0);
		screenPoint = clamp(screenPoint, vec2(-1.0), vec2(1.0));
		screenPoint = screenPoint*0.5 + vec2(0.5);
			
		vec2 occluderTexcoord = vec2(screenPoint.x, screenPoint.y);
		vec3 occluderPosition = samplePos(occluderTexcoord);
		/*vec3 occluderNormal = sampleNrm(occluderTexcoord);
		occluderNormal = normalize(occluderNormal);*/
		vec3 occluderNormal = calcNormalBasedOnScreenData(occluderTexcoord);
		occlusion += gatherOcclusion(normal, position, occluderNormal, occluderPosition);
	}

	return occlusion * (1.0/16.0); 
	//return -1.0f; //IMPLEMENT THIS
}

//////////////////////////////////////
// END TODO
//////////////////////////////////////

const float occlusion_strength = 1.5f;
void main(void)
{
	float exp_depth = texture(u_Depthtex, fs_Texcoord).r;
    float lin_depth = linearizeDepth(exp_depth,1,400) + 0.2;
	lin_depth = clamp(lin_depth, 0.0, 1.0);

	vec3 position = samplePos(fs_Texcoord);
	vec3 normal = sampleNrm(fs_Texcoord);//calcNormalBasedOnScreenData(fs_Texcoord);

	switch (u_Display)
	{
	case DISPLAY_MESH:
		out_Color = vec4(texture(u_Colortex, fs_Texcoord).xyz, 1.0);
		break;
	case DISPLAY_DEPTH:
		// Display Depth
		out_Color = vec4(vec3(1.0-lin_depth),0);
		break;
	case DISPLAY_OCCLUSION:
		// Calculate SSAO
		float occlusionSSAO = occlusionWithPoissonSSSamples(fs_Texcoord, position, normal);
		occlusionSSAO = clamp(occlusionSSAO*occlusion_strength,0.0f,1.0f);
		out_Color = vec4(vec3(1.0f) - occlusionSSAO, 1.0f);
		break;
	case DISPLAY_NORMAL:
		out_Color = vec4(normal,1.0);
		break;
	default:
		float diffuse = dot(-normal,normalize(vec3(0,0,-1))) + dot(-normal,normalize(vec3(0,-1,0))) + dot(-normal,normalize(vec3(1,0,0)));
		float ambient = 0.2;
		vec3 matColor = texture(u_Colortex, fs_Texcoord).xyz;
		
		// Calculate SSAO
		float occlusion = occlusionWithPoissonSSSamples(fs_Texcoord, position, normal);
		occlusion = clamp(occlusion*occlusion_strength,0.0f,1.0f);
		
		vec3 colorFinal = (diffuse*0.5 + ambient*(1.0-occlusion)) * matColor;

		// Fog based on height - lower levels get fog
		if (u_DisplayFog == DISPLAY_FOG || u_DisplayFog == DISPLAY_LOWER_FOG)
		{
			vec3 worldCoord = sampleWorldPos(fs_Texcoord);
	
			float heightFog = smoothstep(-0.50, 2.5, worldCoord.y);
			heightFog = 1.0 - heightFog;
			colorFinal = mix(colorFinal, vec3(0.95), heightFog);
		}

		// Get fog based on distance from camera
		if (u_DisplayFog == DISPLAY_FOG || u_DisplayFog == DISPLAY_DEPTH_FOG)
		{
			clamp(lin_depth, 0.0, 1.0);
			lin_depth += 0.15;
			float fogDepth = smoothstep(0.3, 0.6, (lin_depth))* (lin_depth);
			vec3 fogColor = vec3(fogDepth); 

			colorFinal = mix(colorFinal, fogColor, fogDepth);
			colorFinal = clamp(colorFinal, 0.0, 1.0);
		}
		out_Color = vec4(colorFinal, 1.0);
	}
	
}
