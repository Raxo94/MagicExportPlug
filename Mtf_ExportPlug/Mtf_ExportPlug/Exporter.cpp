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
	//Also no skeletons

	std::vector<assembleStructs::Material> mat = assamble->GetMaterialVector();
	std::vector<assembleStructs::Mesh> meshes = assamble->GetMeshVector();
	std::vector<assembleStructs::Skeleton> skel = assamble->GetSkeletonVector();
	std::vector<assembleStructs::SkeletalMesh> skelMeshes = skel.at(0).MeshVector;

	for (int i = 0; i < meshes.size(); i++)
	{
		std::string outPath = outFilePath + std::string(&meshes.at(i).MeshName[0]) + ".model";
		std::ofstream outFile(outFilePath + std::string(&meshes.at(i).MeshName[0]) + ".model", std::ofstream::binary); //output file stream
		//Dataheader
		int dataSize = 0;
		int bufferSize = 0;
		outFile.write((const char*)&dataSize, sizeof(int));
		outFile.write((const char*)&bufferSize, sizeof(int));

		hModel expModel = hModel{};
		expModel.numMeshes = 1;
		expModel.numBBoxes = 0;
		expModel.numSkeletons = 0;

		expModel.numJoints = 0;
		expModel.numAnimationStates = 0;
		expModel.numKeyframes = 0;
		
		expModel.numVertices = meshes.at(i).vertexCount;
		expModel.numSkeletonVertices = 0;
		expModel.numIndices = meshes.at(i).indexes.size();

		expModel.TYPE = eModelType::STATIC;

		const char* popo = &meshes.at(i).material.name[0];
		memcpy(expModel.materialName, &meshes.at(i).material.name[0], 22);
		memcpy(&expModel.materialName[strlen(popo)], ".material", 10);

		outFile.write((const char*)&expModel, sizeof(hModel));

		//Offsets
		sOffset currOffset = {};
		for (int j = 0; j < 1; j++)
		{
			dataSize += sizeof(sOffset);
			outFile.write((const char*)&currOffset, sizeof(sOffset));
			currOffset.vertex += meshes.at(i).vertexCount;
			currOffset.skeletonVertex += 0;
			currOffset.index += meshes.at(i).indexes.size();
		}

		//Meshes
		hMesh expMesh;
		for (int j = 0; j < 1; j++)
		{
			dataSize += sizeof(hMesh);
			expMesh.numAnimVertices = 0;
			expMesh.numVertices = meshes.at(i).vertexCount;
			expMesh.numIndexes = meshes.at(i).indexes.size();
			expMesh.parent = sHierarchy();
			
			memcpy(expMesh.pos, &meshes.at(i).transform.translation[0], 3);
			memcpy(expMesh.rot, &meshes.at(i).transform.rotation[0], 3);
			memcpy(expMesh.scale, &meshes.at(i).transform.scale[0], 3);

			expMesh.parentJoint = sJointChild();
			expMesh.parentMesh = sMeshChild();

			outFile.write((const char*)&expMesh, sizeof(hMesh));
		}

		//Boundingboxes
		//There are no boundingboxes

		//Vertices
		outFile.write((const char*)meshes.at(i).Vertices.data(), sizeof(assembleStructs::Vertex) * meshes.at(i).Vertices.size());
		bufferSize += sizeof(assembleStructs::Vertex) * meshes.at(i).Vertices.size();
		//Indices
		outFile.write((const char*)meshes.at(i).indexes.data(), sizeof(int) * meshes.at(i).indexes.size());
		bufferSize += sizeof(int) * meshes.at(i).indexes.size();

		//update dataHeader
		outFile.seekp(std::ios::beg);
		outFile.write((const char*)&dataSize, sizeof(int));
		outFile.write((const char*)&bufferSize, sizeof(int));

		outFile.close();
	}
	//One duplicate loop for skeletal meshes
	for (int i = 0; i < skelMeshes.size(); i++)
	{
		//std::ofstream outFile(outFilePath, std::ofstream::binary); //output file stream
		//int dataSize = 0;
		//int bufferSize = 0;
		////Skeletons
		//hSkeleton expSkeleton;
		//for (int j = 0; j < skel.size(); j++)
		//{
		//	expSkeleton.jointOffset = jointCounter * sizeof(hJoint);
		//	expSkeleton.jointCount = skel.at(j).jointVector.size();
		//	outFile.write((const char*)&expSkeleton, sizeof(hSkeleton));
		//	dataSize += sizeof(hSkeleton);
		//	jointCounter += skel.at(j).jointVector.size();
		//}
		//
		//outFile.close();
	}
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


