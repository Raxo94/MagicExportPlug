#include "Exporter.h"

Exporter::Exporter()
{
	assamble = new ModelAssembler(); // get the data
	prepareMeshData();

}

Exporter::~Exporter()
{
	delete assamble;
}

void Exporter::prepareMeshData()
{
	//first mesh header. Then there is the vertex array
	sMesh mesh;
	mesh.isAnimated = false;
	mesh.isBoundingBox = false;
	dataHeader.meshes++;

	


}
