#ifndef MODEL_ASSEMBLER_H
#define MODEL_ASSEMBLER_H

#include "maya_includes.h"
#include <iostream>
#include <vector>
#include <array>
using namespace std;



struct vertex
{
	std::array<float, 3> pos;
	std::array<float, 3> nor;
	std::array<float, 2> uv;
};

bool operator==(const vertex& left, const vertex& right);

struct Mesh
{
	vector<vertex> Vertices;
	vector<unsigned int> indexes;

};

class ModelAssembler
{

public:
	ModelAssembler();
	~ModelAssembler();

private:
	//Variables
	MStatus res;
	vector<Mesh> Meshes;

	//Functions
	void AssembleMeshes();

};
#endif