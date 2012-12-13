#include "Utility.h"

#include <iostream>
#include <fstream>
#include <string>

using std::ios;

namespace Utility {

	/*typedef struct {
		GLuint vertex;
		GLuint fragment;
		GLuint geometry;
		GLuint tess_tcs;
		GLuint tess_tes;
	} shaders_t;*/

	char* loadFile(const char *fname, GLint &fSize)
	{
		// file read based on example in cplusplus.com tutorial
		std::ifstream file (fname, ios::in|ios::binary|ios::ate);
		if (file.is_open())
		{
			unsigned int size = (unsigned int)file.tellg();
			fSize = size;
			char *memblock = new char [size];
			file.seekg (0, ios::beg);
			file.read (memblock, size);
			file.close();
			std::cout << "file " << fname << " loaded" << std::endl;
    		return memblock;
		}

		std::cout << "Unable to open file " << fname << std::endl;
		exit(1);
	}

	// printShaderInfoLog
	// From OpenGL Shading Language 3rd Edition, p215-216
	// Display (hopefully) useful error messages if shader fails to compile
	void printShaderInfoLog(GLint shader)
	{
		int infoLogLen = 0;
		int charsWritten = 0;
		GLchar *infoLog;

		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLen);

		if (infoLogLen > 1)
		{
			infoLog = new GLchar[infoLogLen];
			// error check for fail to allocate memory omitted
			glGetShaderInfoLog(shader,infoLogLen, &charsWritten, infoLog);
			std::cout << "InfoLog:" << std::endl << infoLog << std::endl;
			delete [] infoLog;
		}
	}

	void printLinkInfoLog(GLint prog) 
	{
		int infoLogLen = 0;
		int charsWritten = 0;
		GLchar *infoLog;

		glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &infoLogLen);

		if (infoLogLen > 1)
		{
			infoLog = new GLchar[infoLogLen];
			// error check for fail to allocate memory omitted
			glGetProgramInfoLog(prog,infoLogLen, &charsWritten, infoLog);
			std::cout << "InfoLog:" << std::endl << infoLog << std::endl;
			delete [] infoLog;
		}
	}

	shaders_t loadShaders(const char * vert_path, const char * frag_path, const char *geom_path, bool hasGeometryShader,
		                  const char *tcs_path, const char *tes_path, bool hasTessellationShader) {
		GLuint f, v, g, tc, te;

		char *vs,*fs, *gs, *tcs, *tes;

		v = glCreateShader(GL_VERTEX_SHADER);
		f = glCreateShader(GL_FRAGMENT_SHADER);	

		if (hasGeometryShader)
			g = glCreateShader(GL_GEOMETRY_SHADER);

		if (hasTessellationShader)
		{
			tc = glCreateShader(GL_TESS_CONTROL_SHADER);
			te = glCreateShader(GL_TESS_EVALUATION_SHADER);
		}

		// load shaders & get length of each
		GLint vlen;
		GLint flen;
		GLint glen;
		GLint tclen;
		GLint telen;
		vs = loadFile(vert_path,vlen);
		fs = loadFile(frag_path,flen);
		if (hasGeometryShader)
			gs = loadFile(geom_path, glen);

		if (hasTessellationShader)
		{
			tcs = loadFile(tcs_path, tclen);
			tes = loadFile(tes_path, telen);
		}

		const char * vv = vs;
		const char * ff = fs;
		const char * gg;
		const char * tct;
		const char * tet;

		if (hasGeometryShader)
			gg = gs;

		if (hasTessellationShader)
		{
			tct = tcs;
			tet = tes;
		}

		glShaderSource(v, 1, &vv,&vlen);
		glShaderSource(f, 1, &ff,&flen);
		if (hasGeometryShader)
			glShaderSource(g, 1, &gg, &glen);

		if (hasTessellationShader)
		{
			glShaderSource(tc, 1, &tct, &tclen);
			glShaderSource(te, 1, &tet, &telen);
		}

		GLint compiled;

		glCompileShader(v);
		glGetShaderiv(v, GL_COMPILE_STATUS, &compiled);
		if (!compiled)
		{
			std::cout << "Vertex shader not compiled." << std::endl;
		} 
		printShaderInfoLog(v);

		glCompileShader(f);
		glGetShaderiv(f, GL_COMPILE_STATUS, &compiled);
		if (!compiled)
		{
			std::cout << "Fragment shader not compiled." << std::endl;
		} 
		printShaderInfoLog(f);

		if (hasGeometryShader)
		{
			glCompileShader(g);
			glGetShaderiv(g, GL_COMPILE_STATUS, &compiled);
			if (!compiled)
			{
				std::cout << "Geometry shader not compiled." << std::endl;
			} 
			printShaderInfoLog(g);
		}

		if (hasTessellationShader)
		{
			glCompileShader(tc);
			glGetShaderiv(tc, GL_COMPILE_STATUS, &compiled);
			if (!compiled)
			{
				std::cout << "Tesselation Control shader not compiled." << std::endl;
			} 
			printShaderInfoLog(tc);

			glCompileShader(te);
			glGetShaderiv(te, GL_COMPILE_STATUS, &compiled);
			if (!compiled)
			{
				std::cout << "Tesselation Evaluation shader not compiled." << std::endl;
			} 
			printShaderInfoLog(te);
		}

		shaders_t out;
		out.vertex = v;
		out.fragment = f;
		if (hasGeometryShader)
			out.geometry = g;

		if (hasTessellationShader)
		{
			out.tess_tcs = tc;
			out.tess_tes = te;
		}

		delete [] vs; // dont forget to free allocated memory
		delete [] fs; // we allocated this in the loadFile function...
		if (hasGeometryShader)
			delete [] gs;

		if (hasTessellationShader)
		{
			delete [] tcs;
			delete [] tes;
		}

		return out;
	}

	void attachAndLinkProgram( GLuint program, shaders_t shaders, bool hasGeometryShader, bool hasTessellationShader) {
		glAttachShader(program, shaders.vertex);
		glAttachShader(program, shaders.fragment);
		if (hasGeometryShader)
		{
			glAttachShader(program, shaders.geometry);

			glProgramParameteriEXT(program,GL_GEOMETRY_INPUT_TYPE_EXT,GL_TRIANGLES);
			
			int temp = 3;
			//glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT,&temp);
			glProgramParameteriEXT(program,GL_GEOMETRY_VERTICES_OUT_EXT,temp);
		}

		if (hasTessellationShader)
		{
			glAttachShader(program, shaders.tess_tcs);
			glAttachShader(program, shaders.tess_tes);
		}

		glLinkProgram(program);
		GLint linked;
		glGetProgramiv(program,GL_LINK_STATUS, &linked);
		if (!linked) 
		{
			std::cout << "Program did not link." << std::endl;
		}
		printLinkInfoLog(program);
	}

    GLuint createProgram(const char *vertexShaderPath, const char *fragmentShaderPath, 
		                 const char *geometryShaderPath, bool hasGeometryShader, 
						 const char *tcsShaderPath, const char *tesShaderPath, bool hasTessellationShader,
						 const char *attributeLocations[], GLuint numberOfLocations)
    {
	    Utility::shaders_t shaders = Utility::loadShaders(vertexShaderPath, fragmentShaderPath, geometryShaderPath, hasGeometryShader,
														  tcsShaderPath, tesShaderPath, hasTessellationShader);
	
	    GLuint program = glCreateProgram();

		for (GLuint i = 0; i < numberOfLocations; ++i)
		{
            glBindAttribLocation(program, i, attributeLocations[i]);
		}

	    Utility::attachAndLinkProgram(program, shaders, hasGeometryShader, hasTessellationShader);

        return program;
    }
}