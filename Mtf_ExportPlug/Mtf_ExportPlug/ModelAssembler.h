#ifndef MODEL_ASSEMBLER_H
#define MODEL_ASSEMBLER_H

#include "maya_includes.h"
#include <iostream>
#include <vector>
#include <array>
#include <sstream>


using namespace std;


namespace assembleStructs
{
	struct Vertex
	{
		std::array<float, 3> pos;
		std::array<float, 3> nor;
		std::array<float, 2> uv;
	};

	struct SkeletonVertex
	{
		std::array<float, 3> pos;
		std::array<float, 3> normal;
		std::array<float, 2> uv;
		
		std::array<int, 4> influences;
		std::array<float, 4> weights;
	};

	struct sKeyFrame
	{
		float keyTime;
		float keyTranslate[3];
		float keyRotate[4];
		float keyScale[3];
	};

	struct sImAnimationState
	{
		std::vector<sKeyFrame> keyList;
	};


	struct Joint
	{
		int ID, parentID;
		//std::array<float, 3> pos, rot, scale;
		std::array<float, 16> bindPoseInverse, globalBindPoseInverse;
		MString name;
		int animationStateCount;
		std::vector<sImAnimationState> animationState;
		//joints har koll på olika keyframes för olika lager
	};

	struct Skeleton
	{
		vector<Joint> jointVector;
		vector<SkeletonVertex> skeletalVertexVector;
	};

	//Vertex Compare Used for indexing
	bool operator==(const assembleStructs::Vertex& left, const assembleStructs::Vertex& right);

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
		vector<Vertex> Vertices;
		vector<unsigned int> indexes;
		Transform transform;

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
	void ProcessInverseBindpose(MFnSkinCluster&, Skeleton&);
	void ProcessSkeletalVertex (MFnSkinCluster& skinCluster, Skeleton& skeleton);
	void GetJointParentID(MFnDependencyNode& jointDep,Joint& joint);
	void ProcessKeyframes (MFnSkinCluster& skinCluster, Skeleton& skeleton);
	std::array<char, 256> GetTexture(MPlugArray);

	vector<MString> GetAnimLayers(const MString baseLayer);
	void MuteAllLayersExcept(vector<MString>allLayers,MString ExceptLayer);
	Transform GetTransform(MFnTransform &transform);

};
#endif