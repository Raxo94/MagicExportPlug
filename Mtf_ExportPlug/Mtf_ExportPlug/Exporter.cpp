#include "Exporter.h"
#include <fstream>
Exporter::Exporter()
{
	assamble = new ModelAssembler(); // get the data
	
	assembleMeshes = assamble->GetMeshVector();
	prepareMeshData(assembleMeshes.at(0));



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
	mesh.rotation[0] = 0;
	mesh.rotation[1] = 0;
	mesh.rotation[2] = 0;

	mesh.translation[0] = 0;
	mesh.translation[1] = 0;
	mesh.translation[2] = 0;
	mesh.vertexCount = assembleMesh.vertexCount;

	meshVector.push_back(mesh);
	sVertexVector temp;
	int i = 0; for each (assembleStructs::vertex vertex  in  assembleMesh.Vertices)
	{
		sVertex tempVertex;
		tempVertex.pos = vertex.pos;
		tempVertex.nor = vertex.nor;
		tempVertex.uv = vertex.uv;

		temp.vertices.push_back(tempVertex);
		i++;
	}


	VertexVectors.push_back(temp);
	dataHeader.meshes++;

}

void Exporter::writeToFile(string filepath)
{
	std::ofstream outfile(filepath, std::ofstream::binary); //output file stream

	outfile.write((const char*)&dataHeader, sizeof(sDataHeader)); //main header
	outfile.write((const char*)&meshVector[0], sizeof(sMesh));
	outfile.write((const char*)VertexVectors[0].vertices.data(), sizeof(sVertex) * VertexVectors[0].vertices.size());

	//outfile.write((const char*)meshChildHolder[meshCounter].meshChildList.data(), sizeof(sMeshChild) * childMeshCount); //so this contains a index to all child meshes 
	//outfile.write((const char*)&mMaterialList[i], sizeof(sMaterial));
	outfile.close();
	
}
