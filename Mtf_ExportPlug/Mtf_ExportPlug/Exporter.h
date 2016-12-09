#pragma once
#include "ModelAssembler.h"
class Exporter
{

	struct sHeader
	{
		unsigned int meshCount = 0 ;
		unsigned int materialCount = 0 ;
	};

	struct sOffset
	{
		int joint, vertex, index, skeletonVertex;
	};

	struct sMesh
	{
		unsigned int materialID;
		int parentMeshID, parentJointID;
		std::array<float,3> translation, rotation, scale;
		bool isBoundingBox, isAnimated;

		unsigned int meshChildCount = 0;
		unsigned int vertexCount = 0;
		unsigned int indexCount = 0;
		unsigned int skeletonVertexCount = 0;
		unsigned int jointCount = 0;
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
	struct sIndexVector
	{
		vector<unsigned int> indexes;
	};

	struct sMaterial
	{
		std::array <float, 3> ambientColor; //color values
		std::array <float, 3> diffuseColor; //color values times diffuse
		std::array <float, 3> specularColor;  // simply specular color
		float shinyFactor = 1;

		std::array<char, 256> Texture; //filepath
		std::array<char, 256> diffuseTexture; //filepath
		std::array<char, 256> specularTexture; //filepath
		std::array<char, 256> normalTexture; //filepath
	};

	struct sKeyFrame
	{
		float keyTime;
		float keyTranslate[3];
		float keyRotate[3];
		float keyScale[3];
	};

	struct hJoint
	{
		int parentJointID;
		float globalBindposeInverse[16];

		int animationStateCount;
	};

	struct sJointChild
	{
		int parentSkeletonIndex;
		int parentJointIndex;
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
		int indexes = 0;
		int skeletonVertices = 0;
	};

public:
	Exporter();
	~Exporter();

	void writeToFile(string filepath);
	

private:
	sDataHeader dataHeader;
	sHeader MainHeader;
	sOffset offsetHeader;

	ModelAssembler* assamble;
	vector<assembleStructs::Mesh> assembleMeshes;

	vector<sVertexVector> vertexVectors; //this contains all vertexLists
	vector<sIndexVector> indexVectors; //this contains all indexLists;

	vector<sMesh> meshVector;
	vector<sMaterial> MaterialVector;
	void prepareMeshData(assembleStructs::Mesh);
	void prepareMaterialData();
	



};