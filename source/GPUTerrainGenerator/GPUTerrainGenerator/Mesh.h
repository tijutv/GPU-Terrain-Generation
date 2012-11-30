#pragma once

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include <vector>

using namespace std;

struct Triangle
{
	unsigned short x;
	unsigned short y;
	unsigned short z;

	Triangle(unsigned short i_x, unsigned short i_y, unsigned short i_z)
	{
		x = i_x;
		y = i_y;
		z = i_z;
	}
};

class Mesh
{
public:
	vector<glm::vec3> vertices;
	vector<glm::vec3> normals;
	vector<Triangle> faces;

	Mesh()
	{
	}

	~Mesh()
	{
		vertices.clear();
		normals.clear();
		faces.clear();
	}
};