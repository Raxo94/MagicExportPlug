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
	struct vertexDeform
	{
		std::array<int, 4> influences;
		std::array<float, 4> weights;
	};


	struct Vertex
	{
		std::array<float, 3> pos;
		std::array<float, 3> nor;
		std::array<float, 3> tan;
		std::array<float, 2> uv;


	};

	struct SkeletonVertex
	{
		std::array<float, 3> pos;
		std::array<float, 3> nor;
		std::array<float, 2> uv;
		
		vertexDeform deformer;
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
		std::array<float, 16> bindPoseInverse, globalBindPoseInverse;
		MString name;
		int animationStateCount;
		std::vector<sImAnimationState> animationState;
		MDagPath dagPath; //to be used only in assambler
		//joints har koll p� olika keyframes f�r olika lager
	};

	

	//Vertex Compare Used for indexing
	bool operator==(const assembleStructs::Vertex& left, const assembleStructs::Vertex& right);
	bool operator == (const assembleStructs::SkeletonVertex& left, const assembleStructs::SkeletonVertex& right);

	struct Material
	{
		std::array<char, 256> name;
		vector<std::array<char, 256> > boundMeshes;
		std::array<char, 256> textureFilepath;
		std::array<char, 256> specularFilepath;
		std::array<char, 256> diffuseFilepath;
		std::array<char, 256> normalFilepath;

		float diffuse;
		float shinyFactor;
		std::array <float,3> color;
		std::array <float, 3> specularColor;
	};

	struct Transform
	{
		std::array <float,3> translation;
		std::array <float,3> scale;
		double rotation[4];
	};

	

	

	

	struct sHierarchy
	{
		bool hasParentJoint = false;
		bool hasParentMesh = false;
	};
	struct Mesh
	{
		Material material;
		std::array<char, 256> MeshName;
		unsigned int vertexCount;
		vector<Vertex> Vertices;
		vector<unsigned int> indexes;
		Transform transform;
		MDagPath Meshpath; //to be used only in assambler
		sHierarchy parent;

	};
	struct SkeletalMesh
	{
		Material material;
		std::array<char, 256> meshName;
		vector<unsigned int> indexes;
		vector<SkeletonVertex> skeletalVertexVector;
		Transform transform;
		MDagPath Meshpath; //to be used only in assambler
	};
	struct sJointChild
	{
		int parentSkeletonIndex = 0; //needed change hope still works
		int parentJointIndex = 0; //needed change hope still works 
	};
	struct sMeshChild
	{
		int parentMeshIndex = 0; //this was changed
	};

	struct sPos
	{
		float x, y, z;
	};

	struct sBBox
	{
		sPos pos[8];
		sHierarchy parent;
		sJointChild jointParent;
		sMeshChild meshParent;
	};
	struct Skeleton
	{
		vector<SkeletalMesh> MeshVector;
		vector<Joint> jointVector;

	};
	

	
}//End of assembleStructs
using namespace assembleStructs;

class ModelAssembler
{

public:
	ModelAssembler();
	~ModelAssembler();

	vector<Mesh>&GetMeshVector();
	vector<Skeleton>&GetSkeletonVector();
	vector<Material>&GetMaterialVector();

private:
	//Variables
	MStatus res;
	vector<Mesh> standardMeshes;
	vector<Skeleton> Skeletons;
	vector<Material> materials;

	//Functions
	void AssembleMesh(MObject MObjectMeshNode,MObject Parent);
	void AssembleSkeletonsAndMeshes();
	void AssembleMaterials();
	void ConnectMaterialsToMeshes();
	void GetChildrenBoundingBoxes(vector<Joint> joints);

	void ProcessInverseBindpose(MFnSkinCluster&, Skeleton&, MFnDependencyNode& parentNode); //gets inversebindPose and globalInverseBindpose
	void ProcessSkeletalVertex (MFnSkinCluster& skinCluster, Skeleton& skeleton); //Gets vertices and weights for each triangleIndex
	void ProcessSkeletalIndexes(vector<SkeletonVertex>& vertexVector, vector<unsigned int>& indexes); //Modifys vertexList and Adds indexes
	void GetJointParentID(MFnDependencyNode & jointDep, Joint & currentJoint, vector<Joint>OtherJoints); //gets the JointList index index of the joints parent

	vector<vertexDeform> GetSkinWeightsList(MDagPath skinPath, MFnSkinCluster& skinCluster, vector<Joint>joints);
	void ProcessKeyframes (MFnSkinCluster& skinCluster, Skeleton& skeleton);
	std::array<char, 256> GetTexture(MPlugArray);
	
	
	vector<MString> GetAnimLayers(const MString baseLayer);
	void MuteAllLayersExcept(vector<MString>allLayers,MString ExceptLayer);
	Transform GetTransform(MFnTransform &transform);

};
#endif