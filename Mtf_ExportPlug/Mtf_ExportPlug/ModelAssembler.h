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
		std::array<char, 256> textureFilepath;
		std::array<char, 256> specularFilepath;
		std::array<char, 256> diffuseFilepath;
		std::array<char, 256> normalFilepath;

		float diffuse;

		std::array <float,3> color;
		std::array <float, 3> specularColor;
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
	vector<Material>&GetMaterialVector();

private:
	//Variables
	MStatus res;
	vector<Mesh> Meshes;
	vector<Material> materials;

	//Functions
	void AssembleMeshes();
	void AssembleSkeletalMesh();
	void AssembleMaterials();

	std::array<char, 256> GetTexture(MPlugArray);

	Transform GetTransform(MFnTransform &transform);

};
#endif