#ifndef MODEL_ASSEMBLER_H
#define MODEL_ASSEMBLER_H

#include "maya_includes.h"
#include <iostream>


struct HeaderMesh
{
	unsigned int VertexCount = 0;
};

class ModelAssembler
{


public:

	ModelAssembler();
	~ModelAssembler();

private:
	MStatus res;

};
#endif