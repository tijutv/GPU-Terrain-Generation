#include <iostream>
#include "main.h"
#include "Utility.h"
#include "Terrain.h"
#include <string>

const char* mainTitle = "Terrain Generator";

GLuint positionLocation = 0;
GLuint texcoordsLocation = 1;
const char *attributeLocations[] = { "Position", "Texcoords" };

Camera cam;
GLuint passthroughProgram;
GLuint depthTexture = 1;
GLuint FBO = 0;
int triVerticesToDraw;

// Mouse movemet variables
int theButtonState = 0;
int theModifierState = 0;
int lastX = 0, lastY = 0;

bool displayMesh;

struct Tess
{
	float innerTessellation;
	glm::vec3 outerTessellation;
};

Tess tessellation;

GLuint initShader(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile, bool hasGeometryShader,
	              const char* tcsShaderFile, const char* tesShaderFile, bool hasTessellationShader)
{
	GLuint program = Utility::createProgram(vShaderFile, fShaderFile, gShaderFile, hasGeometryShader, 
		                                    tcsShaderFile, tesShaderFile, hasTessellationShader,
											attributeLocations, 2);
	glUseProgram(program);
	return program;
}	

void init()
{
	Terrain terrain(-80.0f, 80.0f, 18, 200);
	terrain.GenerateTerrainData();

	float baryCentricCoords[] =
	{ 
        0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f,
        1.0f, 0.0f, 0.0f
    };

	GLfloat texcoords[] = 
    { 
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f
    };

	//GLushort indices[] = { 0, 1, 3, 3, 1, 2 };
	Triangle indices[] = {Triangle(0,1,3)};

	// Find an unused name for the buffer and create it
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create a buffer object to place data
	GLuint vbo, tbo, ibo;
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &tbo);
	glGenBuffers(1, &ibo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, terrain.vertices.size()*sizeof(glm::vec3), &terrain.vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(positionLocation);
	glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, tbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords), texcoords, GL_STATIC_DRAW);
	glEnableVertexAttribArray(texcoordsLocation);
	glVertexAttribPointer(texcoordsLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, terrain.faces.size()*sizeof(Triangle), &terrain.faces[0], GL_STATIC_DRAW);
	triVerticesToDraw = 3*terrain.faces.size();

	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
	glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

	// create a framebuffer object
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);

	// check FBO status
	GLenum FBOstatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(FBOstatus != GL_FRAMEBUFFER_COMPLETE) {
		printf("GL_FRAMEBUFFER_COMPLETE failed, CANNOT use FBO\n");
        //checkFramebufferStatus(FBOstatus);
	}

	// switch back to window-system-provided framebuffer
	glClear(GL_DEPTH_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
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
    glUniform1i(glGetUniformLocation(passthroughProgram, "u_Noise"),0);
	
	delete[] pixels;
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
		char title[512];
		char titleEnd[512];
		strcpy(title, mainTitle);
		sprintf(titleEnd, " FPS: %4.2f", frame*1000.0/(currTime-lastTime));
		strcat(title, titleEnd);
		glutSetWindowTitle(title);
	 	lastTime = currTime;
		frame = 0;
	}
}

void GetUniforms()
{
	glm::mat4 view = cam.GetViewTransform();
	glm::mat4 perspective = cam.GetPerspective();

	//glUniformMatrix4fv(glGetUniformLocation(passthroughProgram,"u_Model"),1,GL_FALSE,&cam.GetModelView()[0][0]);
	GLuint tex = 0;
	initNoiseTexture(&tex);

	glm::vec4 nearModelView = view*glm::vec4(0,0,cam.near,1);
	glm::vec4 farModelView = view*glm::vec4(0,0,cam.far,1);

	// The tessellation distance - Terrain that is closer to camera than this value is tessellated
	float tessDist = -((farModelView.z - nearModelView.z)/3.0 + nearModelView.z);
	//std::cout << "Near: " << nearModelView.z << " Far: " << farModelView.z << std::endl;

	glUniform1f(glGetUniformLocation(passthroughProgram,"u_Near"), cam.near);
	glUniform1f(glGetUniformLocation(passthroughProgram,"u_Far"), cam.far);
	glUniform1f(glGetUniformLocation(passthroughProgram,"u_Left"), cam.left);
	glUniform1f(glGetUniformLocation(passthroughProgram,"u_Right"), cam.right);
	glUniform1f(glGetUniformLocation(passthroughProgram,"u_Top"), cam.top);
	glUniform1f(glGetUniformLocation(passthroughProgram,"u_Bottom"), cam.bottom);
	
	glUniform1f(glGetUniformLocation(passthroughProgram,"u_TessDistance"), tessDist);
	glUniformMatrix4fv(glGetUniformLocation(passthroughProgram,"u_View"), 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(passthroughProgram,"u_Persp"), 1, GL_FALSE, &perspective[0][0]);
	glUniform1f(glGetUniformLocation(passthroughProgram,"u_InnerTessLevel"), tessellation.innerTessellation);
	glUniform3f(glGetUniformLocation(passthroughProgram,"u_OuterTessLevel"), 
		        tessellation.outerTessellation.x, tessellation.outerTessellation.y, tessellation.outerTessellation.z);

	glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glUniform1i(glGetUniformLocation(passthroughProgram, "u_Depthtex"),0);
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPatchParameteri(GL_PATCH_VERTICES, 3);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	GetUniforms();

	// The line below just displays the mesh
	if (displayMesh)
		glPolygonMode(GL_FRONT, GL_LINE);
	else
		glPolygonMode(GL_FRONT, GL_FILL);

	glDrawElements(GL_PATCHES, triVerticesToDraw,  GL_UNSIGNED_SHORT, 0);

	updateTitleFPS();

	glutPostRedisplay();
	glutSwapBuffers();
}

void reshape(GLsizei w, GLsizei h)
{
	width = w;
	height = h;
	glViewport(0, 0, w, h);
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
	case 'M':
		displayMesh = !displayMesh;
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
	}
}

void mouse(int button, int state, int x, int y)
{
   theButtonState = button;
   theModifierState = glutGetModifiers();
   lastX = x;
   lastY = y;
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
			if (deltaX > 0) cam.rot.y -= 5;
			else if (deltaX < 0) cam.rot.y += 5;
			else 
				if (deltaY > 0) cam.rot.x += 5;
			else if (deltaY < 0) cam.rot.x -= 5;
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

	tessellation.innerTessellation = 1.0f;
	tessellation.outerTessellation = glm::vec3(1);

	// Create, compile and attach shaders
	bool hasGeometryShader = true;
	bool hasTessellationShader = true;
	passthroughProgram = initShader("simpleVS.glsl", "simpleFS.glsl", "simpleGS.glsl", hasGeometryShader, 
		                            "Terrain_TCS.glsl", "Terrain_TES.glsl", hasTessellationShader);
	init();

	displayMesh = true;
	std::cout << "GPU Terrain Generator" << std::endl;

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
    glutMotionFunc(mouse_motion);
	glutMainLoop();
	return 0;
}