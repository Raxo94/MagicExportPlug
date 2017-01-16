#include "Exporter.h"
#include <fstream>
Exporter::Exporter()
{
	assamble = new ModelAssembler(); // get the data
	
	assembleMeshes = assamble->GetMeshVector();
	if (assembleMeshes.size() > 0)
	{
	//	prepareMeshData(assembleMeshes.at(0)); //problemet ligger i att det inte finns en mesh
	}

	//prepareMaterialData();


}

Exporter::~Exporter()
{
	delete assamble;
}

void Exporter::writeModelsToFile(string outFilePath)
{
	int jointCounter = 0;
	int keyCounter = 0;
	int animationLayerCounter = 0;

	//There are no "Models" right now... So a temporary solution where each mesh is thought of as a model ensues
	//There are also no boundingboxes... So boundingoxes is set to 0 for the moment

	std::vector<assembleStructs::Material> mat = assamble->GetMaterialVector();
	std::vector<assembleStructs::Mesh> meshes = assamble->GetMeshVector();
	std::vector<assembleStructs::Skeleton> skel = assamble->GetSkeletonVector();
	//std::vector<assembleStructs::SkeletalMesh> skelMeshes = skel.at(0).MeshVector;

	//std::string outPath = outFilePath + std::string(&meshes.at(0).name + ".model");
	std::ofstream outFile(outFilePath + std::string(&meshes.at(0).name[0]) + ".model", std::ofstream::binary); //output file stream

	//Dataheader
	#pragma region dataHeader

	int dataSize = 0;
	int bufferSize = 0;
	outFile.write((const char*)&dataSize, sizeof(int));
	outFile.write((const char*)&bufferSize, sizeof(int));

	//Modelheader
	hModel expModel = hModel{};
	expModel.numMeshes = assamble->GetMeshVector().size();
	expModel.numBBoxes = assamble->GetBoundingBoxVector().size();
	expModel.numSkeletons = assamble->GetSkeletonVector().size();
	expModel.TYPE = eModelType::STATIC;

	for (Skeleton skeleton : assamble->GetSkeletonVector())
	{
		expModel.TYPE = ANIMATED;
		expModel.numJoints += skeleton.jointVector.size();
		for (Joint joint : skeleton.jointVector)
		{
			expModel.numAnimationStates = skeleton.jointVector.at(0).animationState.size();
			for (int i = 0; i < joint.animationState.size(); i++)
			{
				expModel.numKeyframes += joint.animationState.at(i).keyList.size();
			}
		}
	}
	for (assembleStructs::Mesh standardMesh:assamble->GetMeshVector())
	{
		expModel.numVertices += standardMesh.vertList.size();
		expModel.numIndices += standardMesh.indexList.size();
	}

	for (assembleStructs::Mesh skeletalMesh : assamble->GetMeshVector())
	{
		expModel.numSkeletonVertices += skeletalMesh.skelVertList.size();
		expModel.numIndices += skeletalMesh.indexList.size();
	}
	
		

	//Materials are in the importer stored in meshes, for the engine they are stored in models.
	const char* NewMaterialName = &meshes.at(0).material.name[0];
	memcpy(expModel.materialName, &meshes.at(0).material.name[0], 22);
	memcpy(&expModel.materialName[strlen(NewMaterialName)], ".material", 10);

	outFile.write((const char*)&expModel, sizeof(hModel));

	#pragma endregion END OF DATAHEADER

	//Offsets
	sOffset currOffset = {};
	
	for (assembleStructs::Mesh mesh : meshes)
	{
		dataSize += sizeof(sOffset);
		outFile.write((const char*)&currOffset, sizeof(sOffset));
		currOffset.vertex += mesh.vertList.size();
		currOffset.skeletonVertex += 0;
		currOffset.index += mesh.indexList.size();
	}

	//Meshes
	hMesh expMesh;
	for (assembleStructs::Mesh mesh : meshes)
	{
		dataSize += sizeof(hMesh);
		expMesh.numAnimVertices = 0;
		expMesh.numVertices = mesh.vertList.size();
		expMesh.numIndexes = mesh.indexList.size();
		expMesh.parent = sHierarchy();
			
		memcpy(expMesh.pos, &mesh.transform.pos, 3);
		memcpy(expMesh.rot, &mesh.transform.rot, 3);
		memcpy(expMesh.scale,&mesh.transform.scale, 3);

		expMesh.parentJoint = sJointChild();
		expMesh.parentMesh = sMeshChild();

		outFile.write((const char*)&expMesh, sizeof(hMesh));
	}

	//Boundingboxes
	//There are no boundingboxes

	////Vertices
	//outFile.write((const char*)meshes.at(0).Vertices.data(), sizeof(assembleStructs::Vertex) * meshes.at(0).Vertices.size());
	//bufferSize += sizeof(assembleStructs::Vertex) * meshes.at(i).Vertices.size();
	//
	////Indices
	//outFile.write((const char*)meshes.at(i).indexes.data(), sizeof(int) * meshes.at(i).indexes.size());
	//bufferSize += sizeof(int) * meshes.at(i).indexes.size();

	//update dataHeader
	outFile.seekp(std::ios::beg);
	outFile.write((const char*)&dataSize, sizeof(int));
	outFile.write((const char*)&bufferSize, sizeof(int));
	outFile.close();
}


void Exporter::writeMaterialsToFile(string outFilePath)
{
	std::vector<Material> materials = assamble->GetMaterialVector();
	for (int i = 0; i < materials.size(); i++)
	{
		if (materials.at(i).boundMeshes.size() != 0)
		{
			const char* popo = &materials.at(i).name[0];
			string shit = popo;
			std::ofstream outFile(outFilePath + shit + ".material", std::ofstream::binary);

			char num[25];
			std::string txt;
			std::string str;

			txt = "Ambient color: ";
			sprintf(num, "%f", materials.at(i).color[0]);
			str = txt + num + " ";
			sprintf(num, "%f", materials.at(i).color[1]);
			str += std::string(num) + " ";
			sprintf(num, "%f", materials.at(i).color[2]);
			str += std::string(num) + "\r\n";

			outFile.write(str.c_str(), str.length());

			txt = "Diffuse color: ";
			sprintf(num, "%f", materials.at(i).diffuse * materials.at(i).color[0]);
			str = txt + num + " ";
			sprintf(num, "%f", materials.at(i).diffuse * materials.at(i).color[1]);
			str += std::string(num) + " ";
			sprintf(num, "%f", materials.at(i).diffuse * materials.at(i).color[2]);
			str += std::string(num) + "\r\n";

			outFile.write(str.c_str(), str.length());

			txt = "Specular color: ";
			sprintf(num, "%f", materials.at(i).specularColor[0]);
			str = txt + num + " ";
			sprintf(num, "%f", materials.at(i).specularColor[1]);
			str += std::string(num) + " ";
			sprintf(num, "%f", materials.at(i).specularColor[2]);
			str += std::string(num) + "\r\n";

			outFile.write(str.c_str(), str.length());

			txt = "Shiny factor: ";
			sprintf(num, "%f", 1.0);
			str = txt + std::string(num) + "\r\n";
			outFile.write(str.c_str(), str.length());

			popo = &materials.at(i).diffuseFilepath[0];
			txt = "Diffuse texture: ";
			str = txt + std::string(popo) + "\r\n";
			outFile.write(str.c_str(), str.length());

			popo = &materials.at(i).specularFilepath[0];
			txt = "Specular texture: ";
			str = txt + std::string(popo) + "\r\n";
			outFile.write(str.c_str(), str.length());

			popo = &materials.at(i).normalFilepath[0];
			txt = "Normal texture: ";
			str = txt + std::string(popo);
			outFile.write(str.c_str(), str.length());

			outFile.close();
		}
	}
}

//void Exporter::prepareMeshData(assembleStructs::Mesh assembleMesh)
//{
//	sMesh mesh;
//	mesh.isAnimated = false;
//	mesh.isBoundingBox = false;
//
//	//change this
//	mesh.rotation[0] = 0;
//	mesh.rotation[1] = 0;
//	mesh.rotation[2] = 0;
//
//	mesh.translation = assembleMesh.transform.translation;
//	mesh.scale = assembleMesh.transform.scale;
//	mesh.vertexCount = assembleMesh.vertexCount;
//	mesh.indexCount = assembleMesh.indexes.size();
//	mesh.materialID = 0;
//	mesh.parentJointID = 0;
//	mesh.parentMeshID = 0;
//	mesh.meshChildCount = 0;
//
//	mesh.skeletonVertexCount = 0;
//	mesh.jointCount = 0;
//
//	meshVector.push_back(mesh);
//
//
//	sVertexVector vTemp;
//	int i = 0; for each (assembleStructs::Vertex vertex  in  assembleMesh.Vertices)
//	{
//		sVertex tempVertex;
//		tempVertex.pos = vertex.pos;
//		tempVertex.nor = vertex.nor;
//		tempVertex.uv = vertex.uv;
//
//		vTemp.vertices.push_back(tempVertex);
//		i++;
//	}
//	vertexVectors.push_back(vTemp);
//
//	sIndexVector iTemp;
//	i = 0; for each (unsigned int index  in  assembleMesh.indexes)
//	{
//		iTemp.indexes.push_back(index);
//		i++;
//	}
//
//	indexVectors.push_back(iTemp);
//	dataHeader.meshes++;
//	dataHeader.vertices += mesh.vertexCount;
//	dataHeader.indexes = mesh.indexCount;
//
//	MainHeader.meshCount++;
//}
//
//void Exporter::prepareMaterialData()
//{
//	/*get the material from maya*/
//
//	assembleStructs::Material mayaMaterial; 
//
//	if( assamble->GetMaterialVector().size() >0)
//		mayaMaterial = assamble->GetMaterialVector().at(0);
//	
//	sMaterial newMaterial; // our own material
//
//	newMaterial.ambientColor = mayaMaterial.color;			//get ambientColor
//	newMaterial.specularColor = mayaMaterial.specularColor; //get specularColor
//
//	for (size_t i = 0; i < 3; i++) //get diffuse Color
//		newMaterial.diffuseColor[i] = newMaterial.ambientColor[i] * mayaMaterial.diffuse;
//
//	
//	//newMaterial.shinyFactor;
//	newMaterial.Texture = mayaMaterial.textureFilepath;
//	newMaterial.diffuseTexture = mayaMaterial.diffuseFilepath;
//	newMaterial.specularTexture = mayaMaterial.specularFilepath;
//	newMaterial.normalTexture = mayaMaterial.normalFilepath;
//
//	MaterialVector.push_back(newMaterial);
//	dataHeader.materials++; 
//	MainHeader.materialCount++;
//
//	
//	
//}


