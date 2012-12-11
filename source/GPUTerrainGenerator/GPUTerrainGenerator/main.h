#pragma once

#include <GL/glew.h>
#include <GL/glut.h>
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"

#define PI 3.1415926535897
int width  = 720;
int height = 720;

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
		rot = glm::vec3(0,3,0);

		up = glm::vec3(0,1,0);
		lookPos = glm::vec3(0,0,-200);

		fovy = 80.0;
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
		//return glm::frustum(left, right, bottom, top, near, far);
		glm::mat4 persp = glm::perspective(fovy, aspect, near, far);
		/*std::cout << "Mat: " << persp[0][0] << ", " << persp[0][1] << ", " << persp[0][2] << ", " << persp[0][3] << std::endl;
		std::cout << "Mat: " << persp[1][0] << ", " << persp[1][1] << ", " << persp[1][2] << ", " << persp[1][3] << std::endl;
		std::cout << "Mat: " << persp[2][0] << ", " << persp[2][1] << ", " << persp[2][2] << ", " << persp[2][3] << std::endl;
		std::cout << "Mat: " << persp[3][0] << ", " << persp[3][1] << ", " << persp[3][2] << ", " << persp[3][3] << std::endl << std::endl;*/
		
		right = near * (persp[0][2] - 1)/persp[0][0];
		left  = near * (persp[0][2] + 1)/persp[0][0];
		top    = near * (persp[1][2] - 1)/persp[1][1];
		bottom = near * (persp[1][2] + 1)/persp[1][1];

		//std::cout << "Near: " << near << " Far: " << far << " Left: " << left << "  Right: " << right << "  top: " << top << "  bottom: " << bottom << std::endl;
		return persp;
	}
};