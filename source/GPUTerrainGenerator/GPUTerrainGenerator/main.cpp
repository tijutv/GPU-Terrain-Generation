#include <iostream>
#include <sstream>
#include "main.h"
#include "Utility.h"
#include "Terrain.h"
#include <GL/glut.h>
#include "..\SOIL\SOIL.h"
#include <string>

using namespace glm;

const char* mainTitle = "Terrain Generator";

GLuint positionLocation = 0;
const char *attributeLocations[] = { "Position" };

GLuint posLocSecondPass = 0;
GLuint texLocSecondPass = 0;
const char *attribLocSecondPass[] = { "Position", "Texture" };

namespace secondPassAttributes {
	enum {
		POSITION,
		TEXCOORD
	};
}

Camera cam;
GLuint shaderProgram;
GLuint shaderSecondPassProgram;

GLuint depthTexture = 0;
GLuint normalTexture = 0;
GLuint positionTexture = 0;
GLuint colorTexture = 0;
GLuint worldPosTexture = 0;
GLuint FBO = 0;

GLuint vao;
GLuint vbo, ibo;

int triVerticesToDraw;

GLuint vaoSecondPass;
GLuint iboSecondPass;

typedef struct {
	unsigned int vao;
	unsigned int ibo;
	unsigned int numIndices;
	//Don't need these to get it working, but needed for deallocation
	unsigned int vbo;
} bufferObjects;

typedef struct {
	glm::vec3 pt;
	glm::vec2 texcoord;
} vertex2_t;

namespace attribsSecondPass {
	enum {
		POSITION,
		TEXCOORD
	};
}

bufferObjects bufferSecondPass;

// Mouse movemet variables
int theButtonState = 0;
int theModifierState = 0;
int lastX = 0, lastY = 0;

vec2 meshX;
vec2 meshZ;
vec2 meshSize;

int noiseOctaves = 4;
float noiseLacunarity = 0.07;
float noiseGain = 0.35;

const unsigned int numDeforms = 20;
int uniformDisplay;
int uniformDisplayFog;
bool discoLight;
bool deform;
bool deformClickChanged;
float deformPos;
vec2 deformClickValue;
vec4 deformPosArr[numDeforms];
int deformArrIndex;
vec3 terrainColor;
bool useHeightMap;
bool tessDistSame;

struct Tess
{
	float innerTessellation;
	glm::vec3 outerTessellation;
};

Tess tessellation;
Tess tessellation2;

// Error message for Frame buffer attachment
void checkFramebufferStatus(GLenum framebufferStatus) {
	switch (framebufferStatus) {
        case GL_FRAMEBUFFER_COMPLETE_EXT: break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
            printf("Attachment Point Unconnected");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
            printf("Missing Attachment");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
            printf("Dimensions do not match");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
            printf("Formats");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
            printf("Draw Buffer");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
            printf("Read Buffer");
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
            printf("Unsupported Framebuffer Configuration");
            break;
        default:
            printf("Unkown Framebuffer Object Failure");
            break;
    }
}

// Initialize shader for the first pass
GLuint initShader(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile, bool hasGeometryShader,
	              const char* tcsShaderFile, const char* tesShaderFile, bool hasTessellationShader)
{
	GLuint program = Utility::createProgram(vShaderFile, fShaderFile, gShaderFile, hasGeometryShader, 
		                                    tcsShaderFile, tesShaderFile, hasTessellationShader,
											attributeLocations, 1);
	glUseProgram(program);
	return program;
}

// Initialize shader for the second pass
GLuint initSecondPassShader(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile, bool hasGeometryShader,
	              const char* tcsShaderFile, const char* tesShaderFile, bool hasTessellationShader)
{
	GLuint program = Utility::createProgram(vShaderFile, fShaderFile, gShaderFile, hasGeometryShader, 
		                                    tcsShaderFile, tesShaderFile, hasTessellationShader,
											attribLocSecondPass, 2);
	return program;
}

void init()
{
	meshX = vec2(-512.0, 512.0);
	meshZ = vec2(1.0, 1025.0);
	meshX = vec2(-800.0, 800.0);
	meshZ = vec2(1.0, 2500.0);
	meshSize = vec2(meshX.y - meshX.x, meshZ.y - meshZ.x);
	//Terrain terrain(-800.0f, 800.0f, 1, 2500);
	Terrain terrain;
	terrain.InitializeTerrain(meshX.x, meshX.y, meshZ.x, meshZ.y);
	terrain.GenerateTerrainData();

	//GLushort indices[] = { 0, 1, 3, 3, 1, 2 };
	//Triangle indices[] = {Triangle(0,1,3)};

	// Find an unused name for the buffer and create it
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create a buffer object to place data
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ibo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, terrain.vertices.size()*sizeof(glm::vec3), &terrain.vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(positionLocation);
	glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, terrain.faces.size()*sizeof(Triangle), &terrain.faces[0], GL_STATIC_DRAW);
	triVerticesToDraw = 3*terrain.faces.size();

	//Unplug Vertex Array
    glBindVertexArray(0);
}

void initFBO(int w, int h) {
    GLenum FBOstatus;
	
	glActiveTexture(GL_TEXTURE0);
	
	glGenTextures(1, &depthTexture);
    glGenTextures(1, &normalTexture);
	glGenTextures(1, &positionTexture);
	glGenTextures(1, &colorTexture);
	glGenTextures(1, &worldPosTexture);
	
	glBindTexture(GL_TEXTURE_2D, depthTexture);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

	glBindTexture(GL_TEXTURE_2D, normalTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);	
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB32F , w, h, 0, GL_RGBA, GL_FLOAT,0);

	glBindTexture(GL_TEXTURE_2D, positionTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB32F , w, h, 0, GL_RGBA, GL_FLOAT,0);

	glBindTexture(GL_TEXTURE_2D, colorTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);	
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB32F , w, h, 0, GL_RGBA, GL_FLOAT,0);

	glBindTexture(GL_TEXTURE_2D, worldPosTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);	
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB32F , w, h, 0, GL_RGBA, GL_FLOAT,0);
		
	// create a framebuffer object
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	
	// Instruct openGL that we won't bind a color texture with the currently binded FBO
	glReadBuffer(GL_NONE);
    GLint normal_loc = glGetFragDataLocation(shaderProgram,"out_Normal");
    GLint position_loc = glGetFragDataLocation(shaderProgram,"out_Position");
    GLint color_loc = glGetFragDataLocation(shaderProgram,"out_Color");
    GLint worldPos_loc = glGetFragDataLocation(shaderProgram,"out_WorldPos");
	GLenum draws [4];
    draws[normal_loc] = GL_COLOR_ATTACHMENT0;
    draws[position_loc] = GL_COLOR_ATTACHMENT1;
	draws[color_loc] = GL_COLOR_ATTACHMENT2;
	draws[worldPos_loc] = GL_COLOR_ATTACHMENT3;
	glDrawBuffers(4, draws);

	// attach the texture to FBO depth attachment point
    glBindTexture(GL_TEXTURE_2D, depthTexture);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);
	glBindTexture(GL_TEXTURE_2D, normalTexture);    
	glFramebufferTexture(GL_FRAMEBUFFER, draws[normal_loc], normalTexture, 0);
	glBindTexture(GL_TEXTURE_2D, positionTexture);    
	glFramebufferTexture(GL_FRAMEBUFFER, draws[position_loc], positionTexture, 0);
	glBindTexture(GL_TEXTURE_2D, colorTexture);    
	glFramebufferTexture(GL_FRAMEBUFFER, draws[color_loc], colorTexture, 0);
	glBindTexture(GL_TEXTURE_2D, worldPosTexture);    
	glFramebufferTexture(GL_FRAMEBUFFER, draws[worldPos_loc], worldPosTexture, 0);

	// check FBO status
	FBOstatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(FBOstatus != GL_FRAMEBUFFER_COMPLETE) {
		printf("GL_FRAMEBUFFER_COMPLETE failed, CANNOT use FBO\n");
        checkFramebufferStatus(FBOstatus);
	}

	// switch back to window-system-provided framebuffer
	glClear(GL_DEPTH_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void bindFBO() 
{
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,0); //Bad mojo to unbind the framebuffer using the texture
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    //glColorMask(false,false,false,false);
    glEnable(GL_DEPTH_TEST);
}

void initSecondPass() {

	// Screen positions and texture coordinates
	vertex2_t verts [] = { 
		{vec3(-1,1,0),  vec2(0,1)},
		{vec3(-1,-1,0), vec2(0,0)},
		{vec3(1,-1,0),  vec2(1,0)},
		{vec3(1,1,0),   vec2(1,1)}
	};

	// Indices to create triangle using above vertices
	unsigned short indices[] = {0,1,2,
		                        0,2,3};
	bufferSecondPass.numIndices = 6;

	//Allocate vertex array
	//Vertex arrays encapsulate a set of generic vertex attributes and the buffers they are bound too
	glGenVertexArrays(1, &(bufferSecondPass.vao));
    glBindVertexArray(bufferSecondPass.vao);

    
	//Allocate vbos for data - The vbo will haveboth positions and textures
	glGenBuffers(1,&(bufferSecondPass.vbo));
	glGenBuffers(1,&(bufferSecondPass.ibo));
    
	//Upload vertex data
	glBindBuffer(GL_ARRAY_BUFFER, bufferSecondPass.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    
	//Use of strided data, Array of Structures instead of Structures of Arrays
	glVertexAttribPointer(attribsSecondPass::POSITION, 3, GL_FLOAT, GL_FALSE,sizeof(vertex2_t),0);
	glVertexAttribPointer(attribsSecondPass::TEXCOORD, 2, GL_FLOAT, GL_FALSE,sizeof(vertex2_t),(void*)sizeof(vec3));
	glEnableVertexAttribArray(attribsSecondPass::POSITION);
	glEnableVertexAttribArray(attribsSecondPass::TEXCOORD);

    //indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferSecondPass.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, bufferSecondPass.numIndices*sizeof(GLushort), indices, GL_STATIC_DRAW);
    
	//Unplug Vertex Array
    glBindVertexArray(0);
}

int perm[256]= {151,160,137,91,90,15,
  131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
  190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
  88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
  77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
  102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
  135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
  5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
  223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
  129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
  251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
  49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
  138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180};

int grad3[16][3] = {{0,1,1},{0,1,-1},{0,-1,1},{0,-1,-1},
                   {1,0,1},{1,0,-1},{-1,0,1},{-1,0,-1},
                   {1,1,0},{1,-1,0},{-1,1,0},{-1,-1,0}, // 12 cube edges
                   {1,0,-1},{-1,0,-1},{0,-1,1},{0,1,1}}; // 4 more to make 16

void initNoiseTexture(GLuint *texID)
{
	char *pixels;
	int i,j;
  
	glGenTextures(1, texID); // Generate a unique texture ID
	glBindTexture(GL_TEXTURE_2D, *texID); // Bind the texture to texture unit 0

	pixels = new char[256*256*4];
	for(i = 0; i<256; i++)
	{
		for(j = 0; j<256; j++) 
		{
			int offset = (i*256+j)*4;
			char value = perm[(j+perm[i]) & 0xFF];
			pixels[offset] = grad3[value & 0x0F][0] * 64 + 64;   // Gradient x
			pixels[offset+1] = grad3[value & 0x0F][1] * 64 + 64; // Gradient y
			pixels[offset+2] = grad3[value & 0x0F][2] * 64 + 64; // Gradient z
			pixels[offset+3] = value;                     // Permuted index
		}
	}
  
	glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, *texID);
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glUniform1i(glGetUniformLocation(shaderProgram, "u_Noise"),0);
	
	delete[] pixels;
}

GLuint heightMapTex;
GLuint random_normal_tex;
GLuint random_scalar_tex;
GLuint random_scalar_tex1;
void loadHeightMap()
{
	heightMapTex = (unsigned int)SOIL_load_OGL_texture("West_Norway.png",0,0,0);
	glBindTexture(GL_TEXTURE_2D, heightMapTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);

	random_scalar_tex1 = (unsigned int)SOIL_load_OGL_texture("random.png",0,0,0);
	glBindTexture(GL_TEXTURE_2D, random_scalar_tex1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);
}


void initNoiseSoil() {  
	random_normal_tex = (unsigned int)SOIL_load_OGL_texture("random_normal.png",0,0,0);
	glBindTexture(GL_TEXTURE_2D, random_normal_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);

	random_scalar_tex = (unsigned int)SOIL_load_OGL_texture("random.png",0,0,0);
	glBindTexture(GL_TEXTURE_2D, random_scalar_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);
}

int frame = 0;
int currTime = 0;
int lastTime = 0;

// Calculate the frames per second and update display
void updateTitleFPS()
{
	frame++;

	// Get the current time
	currTime = glutGet(GLUT_ELAPSED_TIME);
	
	// Check if a second has passed
	if (currTime - lastTime > 1000) 
	{
		string title;
		string titleEnd;
		char frameRate[20];
		sprintf_s(frameRate, 20, " FPS: %4.2f", frame*1000.0/(currTime-lastTime));
		title = mainTitle;
		title.append(frameRate);
		glutSetWindowTitle(title.c_str());
	 	lastTime = currTime;
		frame = 0;
	}
}

glm::vec4 nearModelView;
glm::vec4 farModelView;

// Create uniforms for the first pass
void GetUniforms()
{
	glm::mat4 view = cam.GetViewTransform();
	glm::mat4 perspective = cam.GetPerspective();

	//glUniformMatrix4fv(glGetUniformLocation(shaderProgram,"u_Model"),1,GL_FALSE,&cam.GetModelView()[0][0]);
	GLuint tex = 0;
	initNoiseTexture(&tex);

	nearModelView = view*glm::vec4(0,0,cam.near,1);
	farModelView = view*glm::vec4(0,0,cam.far,1);

	glm::vec4 leftModelView = view*glm::vec4(cam.left,0,0,1);
	glm::vec4 rightModelView = view*glm::vec4(cam.right,0,0,1);

	glm::vec4 topModelView = view*glm::vec4(0,cam.top,0,1);
	glm::vec4 bottomModelView = view*glm::vec4(0,cam.bottom,0,1);
	/*std::cout << "Near: " << nearModelView.z << " Far: " << farModelView.z << " Left: " << leftModelView.x 
		<< "  Right: " << rightModelView.x << "  top: " << topModelView.y << "  bottom: " << bottomModelView.y << std::endl << std::endl;*/

	glm::vec4 camPosModelView = view*vec4(0,0,0,1.0);

	// The tessellation distance - Terrain that is closer to camera than this value is tessellated
	/*float tessDist = -((farModelView.z - nearModelView.z)/3.0 + nearModelView.z);
	float tessDist2 = -(2.0*(farModelView.z - nearModelView.z)/3.0 + nearModelView.z);*/
	
	/*float rangeX = abs(rightModelView.x - leftModelView.x)/2.0;
	float extraX = abs(tessDist2)*tan(cam.fovx*PI/180.0/2.0);

	float rangeY = abs(topModelView.y - bottomModelView.x)/2.0;
	float extraY = abs(tessDist2)*tan(cam.fovy*PI/180.0/2.0);*/

	float tessDist = -((1.0*cam.far - cam.near)/3.0 + cam.near);
	float tessDist2 = -(2.0*(cam.far - cam.near)/3.0 + cam.near);
	if (tessDistSame)
		tessDist = tessDist2;

	float rangeX = abs(cam.right - cam.left)/2.0;
	float extraX = abs(tessDist2)*tan(cam.fovx*PI/180.0/2.0);

	float rangeY = abs(cam.top - cam.bottom)/2.0;
	float extraY = abs(cam.far)*tan(cam.fovy*PI/180.0/2.0);
	
	//std::cout << "Near: " << extraX << "  Range: " << rangeX << " Far: " << cam.pos.x << "   Trans: " << cam.translate.x << std::endl;
	/*std::cout << "Near: " << nearModelView.z << " Far: " << farModelView.z 
				<< " Left: " << -camPosModelView.x-rangeX-extraX << "  Right: " << -camPosModelView.x+rangeX+extraX 
				<< "  top: " << -camPosModelView.y+rangeY+extraY << "  bottom: " << -camPosModelView.y-rangeY-extraY << std::endl;*/

	glUniform1f(glGetUniformLocation(shaderProgram,"u_LeftMeshMin"), meshX.x);
	glUniform1f(glGetUniformLocation(shaderProgram,"u_NearMeshMin"), meshZ.x);
	glUniform2f(glGetUniformLocation(shaderProgram,"u_MeshSize"), meshSize.x, meshSize.y);

	glUniform1f(glGetUniformLocation(shaderProgram,"u_Near"), -cam.near);
	glUniform1f(glGetUniformLocation(shaderProgram,"u_Far"), -cam.far);
	glUniform1f(glGetUniformLocation(shaderProgram,"u_Left"), -rangeX-extraX);
	glUniform1f(glGetUniformLocation(shaderProgram,"u_Right"), rangeX+extraX);
	glUniform1f(glGetUniformLocation(shaderProgram,"u_Bottom"), -rangeY-extraY);
	glUniform1f(glGetUniformLocation(shaderProgram,"u_Top"), rangeY+extraY);

	glUniform1i(glGetUniformLocation(shaderProgram,"u_Display"), uniformDisplay);
	
	glUniform1f(glGetUniformLocation(shaderProgram,"u_TessDistance"), tessDist);
	glUniform1f(glGetUniformLocation(shaderProgram,"u_TessDistance2"), tessDist2);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram,"u_View"), 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram,"u_Persp"), 1, GL_FALSE, &perspective[0][0]);
	glUniform1f(glGetUniformLocation(shaderProgram,"u_InnerTessLevel"), tessellation.innerTessellation);
	glUniform3f(glGetUniformLocation(shaderProgram,"u_OuterTessLevel"), 
		        tessellation.outerTessellation.x, tessellation.outerTessellation.y, tessellation.outerTessellation.z);
	glUniform1f(glGetUniformLocation(shaderProgram,"u_InnerTessLevel2"), tessellation2.innerTessellation);
	glUniform3f(glGetUniformLocation(shaderProgram,"u_OuterTessLevel2"), 
		        tessellation2.outerTessellation.x, tessellation2.outerTessellation.y, tessellation2.outerTessellation.z);

	glUniform1i(glGetUniformLocation(shaderProgram,"u_NoiseOctaves"), noiseOctaves);
	glUniform1f(glGetUniformLocation(shaderProgram,"u_NoiseLacunarity"), noiseLacunarity);
	glUniform1f(glGetUniformLocation(shaderProgram,"u_NoiseGain"), noiseGain);
	
	glUniform1f(glGetUniformLocation(shaderProgram,"u_UseHeightMap"), useHeightMap? 1.0 : -1.0);
	
	glUniform1f(glGetUniformLocation(shaderProgram,"u_ShowTerrainColor"), discoLight? 1.0 : -1.0);
	glUniform3f(glGetUniformLocation(shaderProgram,"u_TerrainColor"), terrainColor.x, terrainColor.y, terrainColor.z);

	float deformVal = -1;

	// If there's an entry for size of blast then we need to run deformation in the TES shader
	if (deformPosArr[0].w > 0)
	{
		deformVal = 1;
	}

	// Store into the deform array if the mode is enabled and the user clicks on the output screen
	// Claculate z distance and radius to pass into the Tessellation Evaluation Shader
	vec4 deformPos;
	if (deform && deformClickChanged)
	{
		deformPos.w = 0;
		deformVal = 1;
		mat4 invView = inverse(view);
		// Transform the clicked position to world
		deformPos.x = cam.left + deformClickValue.x * (2.0*rangeX);
		deformPos.y = deformPos.x;//.y + deformClickValue.y * (2.0*rangeY);
		deformPos.z = cam.near;

		vec4 deformPosNorm = normalize(deformPos);
		
		deformPos.x = -camPosModelView.x + deformPosNorm.x * 45.0;
		deformPos.z = -camPosModelView.z + deformPosNorm.z * -45.0;
		//deformPos = view*vec4(deformPos.x, deformPos.y, deformPos.z, 1.0);
		// Random Radius
		deformPos.w = ((perm[((int)abs(deformPos.z*200))%256]+perm[((int)abs(deformPos.x)*200)%256])/256.0) * 4.0 + 3.0;

		deformPosArr[deformArrIndex] = deformPos;
		++deformArrIndex;
		deformArrIndex = deformArrIndex % numDeforms;
		deformClickChanged = false;
		//std::cout << "Deform: " << deformPos.x << ", " << deformPos.y << ", " << deformPos.z << ", " << deformPos.w << std::endl << std::endl;
	}

	glUniform1f(glGetUniformLocation(shaderProgram,"u_Deform"), deformVal);
	glUniform4fv(glGetUniformLocation(shaderProgram,"u_DeformPosArr"), numDeforms*4, &(deformPosArr[0][0]));
}

void GetUniformsSecondPass()
{
	glm::mat4 view = cam.GetViewTransform();
	glm::mat4 perspective = cam.GetPerspective();
	nearModelView = view*glm::vec4(0,0,cam.near,1);
	farModelView = view*glm::vec4(0,0,cam.far,1);

	glUniform1f(glGetUniformLocation(shaderSecondPassProgram,"u_NearModel"), -nearModelView.z);
	glUniform1f(glGetUniformLocation(shaderSecondPassProgram,"u_FarModel"), -farModelView.z);
	
	glUniform1i(glGetUniformLocation(shaderSecondPassProgram,"u_ScreenWidth"), width);
	glUniform1i(glGetUniformLocation(shaderSecondPassProgram,"u_ScreenHeight"), height);
	
	glUniform1i(glGetUniformLocation(shaderSecondPassProgram,"u_Display"), uniformDisplay);
	glUniform1i(glGetUniformLocation(shaderSecondPassProgram,"u_DisplayFog"), uniformDisplayFog);
	
	glUniformMatrix4fv(glGetUniformLocation(shaderSecondPassProgram,"u_View"), 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shaderSecondPassProgram,"u_Persp"), 1, GL_FALSE, &perspective[0][0]);
}

void setTextures()
{
    glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,0); 
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//glColorMask(true,true,true,true);
    glDisable(GL_DEPTH_TEST);

	// Use the clear buffer if you want to just draw want is output in the second pass.
	// Not using this will make sure that the output from the first pass is considered while displaying the second pass
	//glClear(GL_COLOR_BUFFER_BIT);
}

void displaySecondPass() {
    glUseProgram(shaderSecondPassProgram);
	GetUniformsSecondPass();

	glBindVertexArray(bufferSecondPass.vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferSecondPass.ibo);

    glEnable(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glUniform1i(glGetUniformLocation(shaderSecondPassProgram, "u_Depthtex"),0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalTexture);
    glUniform1i(glGetUniformLocation(shaderSecondPassProgram, "u_Normaltex"),1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, positionTexture);
    glUniform1i(glGetUniformLocation(shaderSecondPassProgram, "u_Positiontex"),2);
	glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glUniform1i(glGetUniformLocation(shaderSecondPassProgram, "u_Colortex"),3);
	glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, worldPosTexture);
    glUniform1i(glGetUniformLocation(shaderSecondPassProgram, "u_WorldPostex"),4);
	glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, random_normal_tex);
    glUniform1i(glGetUniformLocation(shaderSecondPassProgram, "u_RandomNormaltex"),5);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, random_scalar_tex);
    glUniform1i(glGetUniformLocation(shaderSecondPassProgram, "u_RandomScalartex"),6);
    
    
    glDrawElements(GL_TRIANGLES, bufferSecondPass.numIndices, GL_UNSIGNED_SHORT,0);

    glBindVertexArray(0);
}


void display(void)
{
	bindFBO();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// First use the shaders to tessellate and get the correct geometry in the first pass
	glUseProgram(shaderProgram);
	glPatchParameteri(GL_PATCH_VERTICES, 3);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	GetUniforms();

	// GL_LINE displays just the wireframe while GL_FILL displays a solid primitive
	if (uniformDisplay == DISPLAY_MESH)
	{
		glClearColor(0,0,0,1);
		glPolygonMode(GL_FRONT, GL_LINE);
	}
	else
	{
		glClearColor(0.1,0.4,0.7,1.0);
		glPolygonMode(GL_FRONT, GL_FILL);
	}

	glBindVertexArray(vao);
	glEnable(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, heightMapTex);
    glUniform1i(glGetUniformLocation(shaderProgram, "u_HeightMap"),7);
	
	// This random texture is used to displace water
	glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, random_scalar_tex1);
    glUniform1i(glGetUniformLocation(shaderProgram, "u_RandomScalartex1"),8);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glDrawElements(GL_PATCHES, triVerticesToDraw,  GL_UNSIGNED_INT, 0);

	// Set the textures from the first pass to display it in the second pass
	setTextures();

	// Always want to show solid mesh here as the vbos in this pass are in screen space
	glPolygonMode(GL_FRONT, GL_FILL);
	displaySecondPass();
	
	updateTitleFPS();

	glutPostRedisplay();
	glutSwapBuffers();
}

void freeFBO()
{
	glDeleteTextures(1,&depthTexture);
    glDeleteTextures(1,&normalTexture);
    glDeleteTextures(1,&positionTexture);
	glDeleteTextures(1,&colorTexture);
	glDeleteTextures(1,&worldPosTexture);
    glDeleteFramebuffers(1,&FBO);
}

void reshape(int w, int h)
{
    width = w;
    height = h;
    glBindFramebuffer(GL_FRAMEBUFFER,0);
	glViewport(0,0,(GLsizei)w,(GLsizei)h);
	if (FBO != 0 || depthTexture != 0 || normalTexture != 0 || 
		positionTexture != 0 || colorTexture != 0 || worldPosTexture != 0) {
		freeFBO();
	}
    initFBO(w,h);
}

void keyboard(unsigned char key, int x, int y)
{
	switch(key)
	{
	case 'z':
	case 'Z':
		cam.pos.z -= 0.2;
		cam.lookPos.z -= 0.2;
		break;
	case 'x':
	case 'X':
		cam.pos.z += 0.2;
		cam.lookPos.z += 0.2;
		break;
	case 'w':
	case 'W':
		cam.pos.y += 1;
		cam.lookPos.y += 1;
		break;
	case 's':
	case 'S':
		cam.pos.y -= 1;
		cam.lookPos.y -= 1.0;
		break;
	case 'a':
	case 'A':
		cam.pos.x -= 1;
		cam.lookPos.x -= 1;
		break;
	case 'd':
	case 'D':
		cam.pos.x += 1;
		cam.lookPos.x += 1;
		break;
	case 'm':
		uniformDisplay = DISPLAY_MESH;
		break;
	case 'M':
		uniformDisplay = DISPLAY_SHADED;
		
		break;
	case 'i':
		// Decrease Inner Tesselation
		if (tessellation.innerTessellation > 1.0)
			tessellation.innerTessellation -= 1.0;
		break;
	case 'I':
		// Increase Inner Tesselation
		tessellation.innerTessellation += 1.0;
		break;
	case 'o':
		// Decrease Outer Tesselation
		if (tessellation.outerTessellation.x > 1.0)
			tessellation.outerTessellation -= glm::vec3(1.0);
		break;
	case 'O':
		// Decrease Outer Tesselation
		tessellation.outerTessellation += glm::vec3(1.0);
		break;
	case 'k':
		// Decrease Inner Tesselation
		if (tessellation2.innerTessellation > 1.0)
			tessellation2.innerTessellation -= 1.0;
		break;
	case 'K':
		// Increase Inner Tesselation
		tessellation2.innerTessellation += 1.0;
		break;
	case 'l':
		// Decrease Outer Tesselation
		if (tessellation2.outerTessellation.x > 1.0)
			tessellation2.outerTessellation -= glm::vec3(1.0);
		break;
	case 'L':
		// Decrease Outer Tesselation
		tessellation2.outerTessellation += glm::vec3(1.0);
		break;
	case 't':
	case 'T':
		tessellation.innerTessellation = 14.0;
		tessellation.outerTessellation = vec3(14.0);
		tessellation2.innerTessellation = 6.0;
		tessellation2.outerTessellation = vec3(6.0);
		break;
	case 'c':
	case 'C':
		uniformDisplay = DISPLAY_OCCLUSION;
		break;
	case 'v':
	case 'V':
		// Display depth
		uniformDisplay = DISPLAY_DEPTH;
		break;
	case 'n':
	case 'N':
		// Display normals
		uniformDisplay = DISPLAY_NORMAL;
		break;
	case 'f':
		// Display Fog
		uniformDisplayFog = DISPLAY_FOG;
		break;
	case 'F':
		// Don't display fog
		uniformDisplayFog = 0;
		break;
	case 'g':
		// Display fog based on depth
		uniformDisplayFog = DISPLAY_DEPTH_FOG;
		break;
	case 'G':
		// Display fog based on height
		uniformDisplayFog = DISPLAY_LOWER_FOG;
		break;
	case 'q':
	case 'Q':
		// Disco Light effect On/Off
		discoLight = !discoLight;
		break;
	case 'b':
		// Deform - blast mode
		deform = !deform;
		break;
	case 'h':
	case 'H':
		// Display using height map
		useHeightMap = !useHeightMap;
		break;
	case 'r':
		//noiseOctaves--;
		noiseLacunarity -= 0.01;
		break;
	case 'R':
		//noiseOctaves++;
		noiseLacunarity += 0.01;
		break;
	case 'p':
		//noiseOctaves--;
		noiseGain -= 0.1;
		break;
	case 'P':
		//noiseOctaves++;
		noiseGain += 0.1;
		break;
	case 'j':
		tessDistSame = !tessDistSame;
		break;
	}

	//std::cout << " Noise - Octaves: " << noiseOctaves << ", Lac: " << noiseLacunarity << ", Gain: " << noiseGain << std::endl;
}

void mouse(int button, int state, int x, int y)
{
	theButtonState = button;
	theModifierState = glutGetModifiers();
	lastX = x;
	lastY = y;

    if (deform)
	{
		deformClickValue = vec2(x*1.0/width, (height - y - 1)*1.0/height);
		deformClickChanged = true;
	}

	if (discoLight)
	{
		terrainColor = vec3(perm[(int)x*255/width], perm[(int)y*255/height], perm[(int)x*y*255/(width*height)])/vec3(256.0,256.0,256.0);
	}
}

void mouse_motion(int x, int y)
{
   int deltaX = lastX - x;
   int deltaY = lastY - y;
   bool moveLeftRight = abs(deltaX) > abs(deltaY);
   bool moveUpDown = !moveLeftRight;

   switch(theButtonState)
   {
   case GLUT_LEFT_BUTTON:
	    // Move Camera
		if (theModifierState & GLUT_ACTIVE_ALT)
		{
			
			if (deltaY > 0)
			{
				//if (cam.pos.z < 100)
					cam.pos.z += 5;
				
			}
			else if (deltaY < 0)
			{
				//if (cam.pos.z > 10)
					cam.pos.z -= 5;
			}
		}
		else
		{
			if (deltaX > 0) cam.rot.y -= 2.0;
			else if (deltaX < 0) cam.rot.y += 2.0;
			else 
				if (deltaY > 0) cam.rot.x += 2.0;
			else if (deltaY < 0) cam.rot.x -= 2.0;
		}
		break;
   }
   lastX = x;
   lastY = y;
   glutPostRedisplay();
}

int main(int argc, char** argv)
{
	// Initialize and create window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(50, 50);
	glutCreateWindow(mainTitle);

	// Initialize glew
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		//Problem: glewInit failed, something is seriously wrong.
		std::cout<<"glewInit failed, aborting."<<std::endl;
		return -1;
	}

	char* versionStr = (char*)glGetString(GL_VERSION);
	if (atof(versionStr) < 4.0f)
	{
		cout << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << endl;
		cout << "OpenGL v " << versionStr << " supported on this machine" << endl;
		cout << "Your OpenGL version doesn't support this application. This application requires OpenGL v4.0 or greater" << endl;
		exit(0);
	}

	tessDistSame = false;
	useHeightMap = false;
	terrainColor = vec3(0.5,0.8,0.1);
	tessellation.innerTessellation = 1.0f;
	tessellation.outerTessellation = glm::vec3(1);
	discoLight = false;
	deform = false;
	deformClickChanged = false;
	
	for (int i = 0; i<numDeforms; ++i)
	{
		deformPosArr[i] = vec4(-1);
	}
	int deformArrIndex = 0;

	// Create, compile and attach shaders
	bool hasGeometryShader = true;
	bool hasTessellationShader = true;
	shaderSecondPassProgram = initShader("secondPassVS.glsl", "secondPassFS.glsl", NULL, false, 
		                                 NULL, NULL, false);
	shaderProgram = initShader("simpleVS.glsl", "simpleFS.glsl", "simpleGS.glsl", hasGeometryShader, 
		                            "Terrain_TCS.glsl", "Terrain_TES.glsl", hasTessellationShader);
	initFBO(width, height);
	init();
	loadHeightMap();
	initSecondPass();

	uniformDisplay = DISPLAY_MESH;
	uniformDisplayFog = DISPLAY_FOG;
	std::cout << "GPU Terrain Generator" << std::endl;

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
    glutMotionFunc(mouse_motion);
	glutMainLoop();
	return 0;
}