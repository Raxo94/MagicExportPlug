#ifndef MODEL_ASSEMBLER_H
#define MODEL_ASSEMBLER_H

#include "maya_includes.h"
#include <iostream>
#include <vector>
#include <array>
using namespace std;


namespace assembleStructs
{
	struct vertex
	{
		std::array<float, 3> pos;
		std::array<float, 3> nor;
		std::array<float, 2> uv;
	};
	//Vertex Compare Used for indexing
	bool operator==(const assembleStructs::vertex& left, const assembleStructs::vertex& right);

	struct Material
	{
		vector<std::array<char, 256> > boundMeshes;
		bool hasTexture;
		char textureFilepath[256];
		float diffuse;
		float color[3];
	};

	//WorldMatrix

	struct Transform
	{
		std::array <float,3> translation;
		std::array <float,3> scale;
		double rotation[4];
	};

	struct Mesh
	{
		char MeshName[256];
		unsigned int vertexCount;
		vector<vertex> Vertices;
		vector<unsigned int> indexes;
		Transform transform;

	};

	struct Skeleton
	{
		int Nothing;
	};
}//End of assembleStructs
using namespace assembleStructs;

class ModelAssembler
{

public:
	ModelAssembler();
	~ModelAssembler();

	vector<Mesh>&GetMeshVector();

private:
	//Variables
	MStatus res;
	vector<Mesh> Meshes;
	vector<Material> materials;

	//Functions
	void AssembleMeshes();
	void AssembleSkeletalMesh();
	void AssembleMaterials();

	Transform GetTransform(MFnTransform &transform);

};
#endif