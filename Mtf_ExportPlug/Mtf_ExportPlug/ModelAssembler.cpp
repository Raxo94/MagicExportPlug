#include"ModelAssembler.h"

ModelAssembler::ModelAssembler()
{
	AssembleMeshes();
}

ModelAssembler::~ModelAssembler()
{
}

void ModelAssembler::AssembleMeshes()
{
	MItDag it(MItDag::kDepthFirst);
	MObject parent;

	while (it.isDone() == false)
	{
		if (it.currentItem().hasFn(MFn::kMesh))
		{
			MFnMesh mayaMesh(it.currentItem());
			Mesh tempMesh;


			MFloatPointArray pts;
			MFloatArray u, v;
			MFloatVectorArray normals;
			MIntArray triangleCountsOffsets;
			MIntArray triangleIndices;
			MIntArray triangleCounts;
			MIntArray triangleVertexIDs;
			MVector vertexNormal;
			MIntArray normalList, normalCount;


			mayaMesh.getPoints(pts, MSpace::kObject);
			mayaMesh.getUVs(u, v, 0);
			mayaMesh.getTriangleOffsets(triangleCountsOffsets, triangleIndices);
			mayaMesh.getTriangles(triangleCounts, triangleVertexIDs);
			mayaMesh.getNormals(normals, MSpace::kObject);

			std::vector<vertex> VertexVectorTemp;
			VertexVectorTemp.resize(triangleVertexIDs.length());

			mayaMesh.getNormalIds(normalCount, normalList);


			for (size_t i = 0; i < triangleVertexIDs.length(); i++)
			{

				VertexVectorTemp.at(i).pos[0] = pts[triangleVertexIDs[i]].x;
				VertexVectorTemp.at(i).pos[1] = pts[triangleVertexIDs[i]].y;
				VertexVectorTemp.at(i).pos[2] = pts[triangleVertexIDs[i]].z;

				VertexVectorTemp.at(i).nor[0] = normals[triangleVertexIDs[i]].x;
				VertexVectorTemp.at(i).nor[1] = normals[triangleVertexIDs[i]].y;
				VertexVectorTemp.at(i).nor[2] = normals[triangleVertexIDs[i]].z;

				VertexVectorTemp.at(i).uv[0] = u[triangleVertexIDs[i]];
				VertexVectorTemp.at(i).uv[1] = v[triangleVertexIDs[i]];

				bool VertexIsUnique = true;
				for (size_t PassedVertex = 0; PassedVertex < tempMesh.Vertices.size(); PassedVertex++)
				{
					
					if (VertexVectorTemp.at(i) == tempMesh.Vertices.at(PassedVertex))
					{
						tempMesh.indexes.push_back(PassedVertex);
						VertexIsUnique = false;
						break; // get out of loop
					}
					
				}
				if (VertexIsUnique == true)
				{
					tempMesh.indexes.push_back(tempMesh.Vertices.size());
					tempMesh.Vertices.push_back(VertexVectorTemp.at(i));
				}
				
			}//VERTEXLIST


			Meshes.push_back(tempMesh);


		}// END OF Mesh
		it.next();
		
	}//End of loop
}

bool operator==(const vertex & left, const vertex & right)
{
	if (left.pos == right.pos)
	{
		if (left.nor == right.nor)
		{
			if (left.uv == right.uv)
				return true;
		}
	}

	return false;
}
