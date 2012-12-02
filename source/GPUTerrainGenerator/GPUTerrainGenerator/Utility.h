#ifndef UTILITY_H_
#define UTILITY_H_

#include <GL/glew.h>

namespace Utility
{
	GLuint createProgram(const char *vertexShaderPath, const char *fragmentShaderPath, 
		                 const char *geometryShaderPath, bool hasGeometryShader, 
						 const char *tcsShaderPath, const char *tesShaderPath, bool hasTessellationShader,
						 const char *attributeLocations[], GLuint numberOfLocations);
}
 
#endif