#pragma once

#include "Mesh.h"

class Terrain:public Mesh
{
	float minX;
	float maxX;
	float minZ;
	float maxZ;
public:
	Terrain();
	Terrain(float f_minX, float f_maxX, float f_minZ, float f_maxZ)
	{
		minX = f_minX;
		maxX = f_maxX;
		minZ = f_minZ;
		maxZ = f_maxZ;
	}
	void GenerateTerrainData();
};

void Terrain::GenerateTerrainData()
{
	float stepX = 2.0;
	float stepZ = 2.0;

	int numX = int((maxX - minX)/stepX) + 1;

	// Create all vertices
	for (float incrZ = -minZ; incrZ >= -maxZ; incrZ -= stepZ)
	{
		float incrX = minX;
		for (int i = 0; i<numX; ++i )
		{
			/*if (incrX > -1 && incrX < 1)
				vertices.push_back(glm::vec3(incrX, -2, incrZ));
			else*/
			vertices.push_back(glm::vec3(incrX, 0, incrZ));
			normals.push_back(glm::vec3(0,1,0));
			incrX += stepX;
		}
	}

	// Create triangular faces
	for (unsigned short i=numX+1; i<(unsigned short)vertices.size(); ++i)
	{
		if (i%numX == 0)
			continue;
		faces.push_back(Triangle(i-1, (i-1)-numX, i-numX));
		faces.push_back(Triangle(i-numX, i, i-1));
	}
}