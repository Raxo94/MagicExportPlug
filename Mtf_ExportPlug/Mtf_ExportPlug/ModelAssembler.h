#ifndef MODEL_ASSEMBLER_H
#define MODEL_ASSEMBLER_H

#include "maya_includes.h"
#include <iostream>
#include <vector>
#include <array>
using namespace std;



struct vertex
{
	std::array<float, 3> pos;
	std::array<float, 3> nor;
	std::array<float, 2> uv;
};
//Vertex Compare Used for indexing
bool operator==(const vertex& left, const vertex& right);

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
	float translation[3];
	double scale[3];
	double rotation[4];
};

struct Mesh
{
	char MeshName[256];
	vector<vertex> Vertices;
	vector<unsigned int> indexes;
	Transform transform;

};

struct Skeleton
{
	int Nothing;
};

class ModelAssembler
{

public:
	ModelAssembler();
	~ModelAssembler();

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