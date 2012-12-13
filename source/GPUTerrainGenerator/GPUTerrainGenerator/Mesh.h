#pragma once

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include <vector>

using namespace std;

struct Triangle
{
	unsigned int x;
	unsigned int y;
	unsigned int z;

	Triangle(unsigned int i_x, unsigned int i_y, unsigned int i_z)
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