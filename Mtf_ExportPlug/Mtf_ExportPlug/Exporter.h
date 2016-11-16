#pragma once
#include "ModelAssembler.h"
class Exporter
{

	struct sHeader
	{
		unsigned int meshCount;
		unsigned int materialCount;
	};

	struct sMesh
	{
		unsigned int materialID;
		int parentMeshID, parentJointID;
		float translation[3], rotation[3], scale[3];
		bool isBoundingBox, isAnimated;

		unsigned int meshChildCount;
		unsigned int vertexCount;
		unsigned int skeletonVertexCount;
		unsigned int jointCount;
	};

	struct sVertex
	{
		std::array<float, 3> pos;
		std::array<float, 3> nor;
		std::array<float, 2> uv;
		float tangentNormal[3];
		float biTangentNormal[3];
	};

	struct sVertexVector
	{
		vector<sVertex> vertices;
	};

	struct sMaterial
	{
		float ambientColor[3];
		float diffuseColor[3];
		float specularColor[3];
		float shinyFactor;

		char diffuseTexture[256];
		char specularTexture[256];
		char normalTexture[256];
	};

	struct sDataHeader
	{
		int datasize = 0;
		int buffersize = 0;
		int meshes = 0;
		int materials = 0;
		int joints = 0;
		int keycounts = 0;
		int keyframes = 0;
		int vertices = 0;
		int skeletonVertices = 0;
	};

public:
	Exporter();
	~Exporter();

	void writeToFile(string filepath);
	

private:
	sDataHeader dataHeader;

	ModelAssembler* assamble;
	vector<assembleStructs::Mesh> assembleMeshes;
	vector<sVertexVector> VertexVectors; //this contains all vertexLists
	vector<sMesh> meshVector;
	void prepareMeshData(assembleStructs::Mesh);
	



};