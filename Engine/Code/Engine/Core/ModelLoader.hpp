#pragma once

#include "Engine/Math/Vector3.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

#include <vector>

bool LoadObj(std::string fp,
	std::vector<Vector3>& verts,
	std::vector<Vector3>& uvs,
	std::vector<Vector3>& normals)
{
	std::vector<uint> vertIndices;
	std::vector<uint> uvIndices;
	std::vector<uint> normalIndices;
	std::vector<Vector3> allVertices;
	std::vector<Vector3> allUVs;
	std::vector<Vector3> allNormals;

	FILE* file;
	//errno_t err;

	//err = fopen_s(&file, fp.c_str(), "r");
	//ASSERT_OR_DIE(err == 0, "Cannot open obj file!");
	file = fopen(fp.c_str(), "r");
	ASSERT_OR_DIE(file != NULL, "Cannot open obj file!");

	// end of line
	bool eol = false;

	while (!eol)
	{
		char header[256] = "";
		int res = fscanf(file, "%s", header);
		if (res == EOF)
		{
			eol = true;
			continue;
		}

		if (strcmp(header, "v") == 0)
		{
			Vector3 vert = Vector3::ZERO;
			fscanf(file, "%f %f %f\n", &vert.x, &vert.y, &vert.z);
			vert.x *= -1.f;
			allVertices.push_back(vert);
		}
		else if (strcmp(header, "vt") == 0)
		{
			Vector3 uv = Vector3::ZERO;
			fscanf(file, "%f %f %f\n", &uv.x, &uv.y, &uv.z);
			allUVs.push_back(uv);
		}
		else if (strcmp(header, "vn") == 0)
		{
			Vector3 n = Vector3::ZERO;
			fscanf(file, "%f %f %f\n", &n.x, &n.y, &n.z);
			n.x *= -1.f;
			allNormals.push_back(n);
		}
		else if (strcmp(header, "f") == 0)
		{
			uint quadVertexIndex[4];
			uint quadUVIndex[4];
			uint quadNormalIndex[4];

			int numMatch = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d\n",
				&quadVertexIndex[0], &quadUVIndex[0], &quadNormalIndex[0],
				&quadVertexIndex[1], &quadUVIndex[1], &quadNormalIndex[1],
				&quadVertexIndex[2], &quadUVIndex[2], &quadNormalIndex[2],
				&quadVertexIndex[3], &quadUVIndex[3], &quadNormalIndex[3]);

			if (numMatch == 12)
			{
				// Triangle
				vertIndices.push_back(quadVertexIndex[0]);
				vertIndices.push_back(quadVertexIndex[1]);
				vertIndices.push_back(quadVertexIndex[3]);
				vertIndices.push_back(quadVertexIndex[1]);
				vertIndices.push_back(quadVertexIndex[2]);
				vertIndices.push_back(quadVertexIndex[3]);

				uvIndices.push_back(quadUVIndex[0]);
				uvIndices.push_back(quadUVIndex[1]);
				uvIndices.push_back(quadUVIndex[3]);
				uvIndices.push_back(quadUVIndex[1]);
				uvIndices.push_back(quadUVIndex[2]);
				uvIndices.push_back(quadUVIndex[3]);

				normalIndices.push_back(quadNormalIndex[0]);
				normalIndices.push_back(quadNormalIndex[1]);
				normalIndices.push_back(quadNormalIndex[3]);
				normalIndices.push_back(quadNormalIndex[1]);
				normalIndices.push_back(quadNormalIndex[2]);
				normalIndices.push_back(quadNormalIndex[3]);
			}
			else if (numMatch == 9)
			{
				// Triangle
				vertIndices.push_back(quadVertexIndex[0]);
				vertIndices.push_back(quadVertexIndex[1]);
				vertIndices.push_back(quadVertexIndex[2]);

				uvIndices.push_back(quadUVIndex[0]);
				uvIndices.push_back(quadUVIndex[1]);
				uvIndices.push_back(quadUVIndex[2]);

				normalIndices.push_back(quadNormalIndex[0]);
				normalIndices.push_back(quadNormalIndex[1]);
				normalIndices.push_back(quadNormalIndex[2]);
			}
		}
		else
		{
			// Ignore all other headers
			char toEndLine[1024];
			fgets(toEndLine, 1024, file);
		}
	}

	// Populate final vertices, ...
	for( uint i = 0; i < vertIndices.size(); ++i ){
		// Index for reference
		uint vIndex = vertIndices[i];
		uint texIndex = uvIndices[i];
		uint nIndex = normalIndices[i];

		// Get data itself - vertex, uv or normal
		Vector3 vertex = allVertices[ vIndex - 1 ];
		Vector3 uv = allUVs[ texIndex - 1 ];
		Vector3 normal = allNormals[ nIndex - 1 ];

		// Put data in output array
		verts.push_back(vertex);
		uvs.push_back(uv);
		normals.push_back(normal);
	}

	// End of operations. Close file.
	fclose(file);
	return true;
}