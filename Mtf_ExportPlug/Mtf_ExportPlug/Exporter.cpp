#include "Exporter.h"
#include <fstream>
Exporter::Exporter()
{
	assamble = new ModelAssembler(); // get the data
	
	assembleMeshes = assamble->GetMeshVector();
	prepareMeshData(assembleMeshes.at(0));
	prepareMaterialData();


}

Exporter::~Exporter()
{
	delete assamble;
}

void Exporter::prepareMeshData(assembleStructs::Mesh assembleMesh)
{
	sMesh mesh;
	mesh.isAnimated = false;
	mesh.isBoundingBox = false;
	
	//change this
	mesh.rotation[0] = 0;
	mesh.rotation[1] = 0;
	mesh.rotation[2] = 0;

	mesh.translation = assembleMesh.transform.translation;
	mesh.scale = assembleMesh.transform.scale;
	mesh.vertexCount = assembleMesh.vertexCount;
	mesh.indexCount = assembleMesh.indexes.size();
	mesh.materialID = 0;
	mesh.parentJointID = 0;
	mesh.parentMeshID = 0;
	mesh.meshChildCount = 0;

	mesh.skeletonVertexCount = 0;
	mesh.jointCount = 0;

	meshVector.push_back(mesh);

	sVertexVector vTemp;
	int i = 0; for each (assembleStructs::vertex vertex  in  assembleMesh.Vertices)
	{
		sVertex tempVertex;
		tempVertex.pos = vertex.pos;
		tempVertex.nor = vertex.nor;
		tempVertex.uv = vertex.uv;

		vTemp.vertices.push_back(tempVertex);
		i++;
	}
	vertexVectors.push_back(vTemp);

	sIndexVector iTemp;
	i = 0; for each (unsigned int index  in  assembleMesh.indexes)
	{
		iTemp.indexes.push_back(index);
		i++;
	}

	indexVectors.push_back(iTemp);
	dataHeader.meshes++;
	dataHeader.vertices += mesh.vertexCount;
	dataHeader.indexes = mesh.indexCount;
}

void Exporter::prepareMaterialData()
{
	//get the material from maya
	//assembleStructs::Material mayaMaterial; 
	//mayaMaterial = assamble->GetMaterialVector().at(0); 
	//
	//
	//sMaterial newMaterial; // our own material

	//newMaterial.ambientColor = mayaMaterial.color; //get ambientColor
	//
	//for (size_t i = 0; i < 3; i++) //get diffuse Color
	//	newMaterial.diffuseColor[i] = newMaterial.ambientColor[i] * mayaMaterial.diffuse;
	
	



}

void Exporter::writeToFile(string filepath)
{

	MainHeader.materialCount = 0;
	MainHeader.meshCount = 1;

	offsetHeader.joint = 0;
	offsetHeader.skeletonVertex = 0;
	offsetHeader.vertex = 0;//dataHeader.vertices * sizeof(sVertex);
	offsetHeader.index = 0;//dataHeader.indexes * sizeof(unsigned int);

	dataHeader.datasize = sizeof(sMesh) + sizeof(sOffset) + sizeof(sHeader); //mystery
	
	dataHeader.buffersize += sizeof(sVertex) * meshVector[0].vertexCount;
	dataHeader.buffersize += sizeof(unsigned int) * meshVector[0].indexCount;
	
	std::ofstream outfile(filepath, std::ofstream::binary); //output file stream

	outfile.write((const char*)&dataHeader, sizeof(sDataHeader)); //main header
	outfile.write((const char*)&MainHeader, sizeof(sHeader)); //main header
	outfile.write((const char*)&offsetHeader, sizeof(sOffset)); //main header
	outfile.write((const char*)&meshVector[0], sizeof(sMesh));
	outfile.write((const char*)vertexVectors[0].vertices.data(), sizeof(sVertex) * vertexVectors[0].vertices.size());
	outfile.write((const char*)indexVectors[0].indexes.data(), sizeof(unsigned int) * indexVectors[0].indexes.size());

	//outfile.write((const char*)meshChildHolder[meshCounter].meshChildList.data(), sizeof(sMeshChild) * childMeshCount); //so this contains a index to all child meshes 
	//outfile.write((const char*)&mMaterialList[i], sizeof(sMaterial));
	outfile.close();
	
}
