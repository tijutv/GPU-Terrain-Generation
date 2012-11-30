#pragma once

#include <GL/glew.h>
#include <GL/glut.h>
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"

#define PI 3.1415926535897
const int width  = 720;
const int height = 720;

struct Camera
{
	glm::vec3 pos;
	glm::vec3 translate;
	glm::vec3 rot;

	glm::vec3 up;
	glm::vec3 lookPos;

	float fovy;
	float aspect;
	float near;
	float far;

	float left;
	float right;
	float bottom;
	float top;

	Camera()
	{
		pos = glm::vec3(0,5,-1);
		translate = glm::vec3(0,0,0);
		rot = glm::vec3(0,0,0);

		up = glm::vec3(0,1,0);
		lookPos = glm::vec3(0,0,-200);

		fovy = 90.0;
		aspect = width/height;
		near = 3.0f;
		far = 200.0f;

		left = -10;
		right = 10;
		bottom = -10;
		top = 10;
	}

	glm::mat4 GetViewTransform()
	{
		glm::mat4 model_view = glm::lookAt(pos+translate, lookPos, up);

		// X-Y-Z rotation
		model_view *= glm::rotate(model_view, rot.x, glm::vec3(1,0,0)) *
			          glm::rotate(model_view, rot.y, glm::vec3(0,1,0)) *
					  glm::rotate(model_view, rot.z, glm::vec3(0,0,1));

		return model_view;
	}

	glm::mat4 GetPerspective()
	{
		return glm::perspective(fovy, aspect, near, far);
		//return glm::frustum(left, right, bottom, top, near, far);
	}
};