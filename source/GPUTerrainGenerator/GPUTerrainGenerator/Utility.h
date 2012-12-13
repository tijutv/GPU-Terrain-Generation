#ifndef UTILITY_H_
#define UTILITY_H_

#include <GL/glew.h>

namespace Utility
{
	typedef struct {
		GLuint vertex;
		GLuint fragment;
		GLuint geometry;
		GLuint tess_tcs;
		GLuint tess_tes;
	} shaders_t;	




shaders_t loadShaders(const char * vert_path, const char * frag_path, const char *geom_path, bool hasGeometryShader,
		                  const char *tcs_path, const char *tes_path, bool hasTessellationShader);

void attachAndLinkProgram( GLuint program, shaders_t shaders, bool hasGeometryShader, bool hasTessellationShader);

	GLuint createProgram(const char *vertexShaderPath, const char *fragmentShaderPath, 
		                 const char *geometryShaderPath, bool hasGeometryShader, 
						 const char *tcsShaderPath, const char *tesShaderPath, bool hasTessellationShader,
						 const char *attributeLocations[], GLuint numberOfLocations);
}
 
#endif