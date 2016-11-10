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
			MFnMesh meshNode(it.currentItem()); //the meshNode
			Mesh tempMesh;
			vector<vertex>nodeVertices;

			MFloatPointArray pts;
			MIntArray vertexCounts;
			MIntArray polygonVertexIDs;
			MFloatArray u, v;
			MIntArray uvCounts;
			MIntArray uvIDs;
			MFloatVectorArray normals;
			MIntArray triangleCountsOffsets;
			MIntArray triangleIndices;
			MIntArray triangleCounts;
			MIntArray triangleVertexIDs;
			MVector vertexNormal;
			MIntArray normalList, normalCount;

			meshNode.getPoints(pts, MSpace::kObject);
			meshNode.getUVs(u, v, 0);
			meshNode.getAssignedUVs(uvCounts, uvIDs); //indices for UV:s

			meshNode.getTriangleOffsets(triangleCountsOffsets, triangleIndices);
			meshNode.getVertices(vertexCounts, polygonVertexIDs); //get vertex polygon indices
			meshNode.getNormals(normals, MSpace::kObject);

			nodeVertices.resize(triangleIndices.length());

			meshNode.getNormalIds(normalCount, normalList);


			for (unsigned int i = 0; i < triangleIndices.length(); i++) 
			{ //for each triangle index (36)

				nodeVertices.at(i).pos[0] = pts[polygonVertexIDs[triangleIndices[i]]].x;
				nodeVertices.at(i).pos[1] = pts[polygonVertexIDs[triangleIndices[i]]].y;
				nodeVertices.at(i).pos[2] = pts[polygonVertexIDs[triangleIndices[i]]].z;

				nodeVertices.at(i).nor[0] = normals[normalList[triangleIndices[i]]].x;
				nodeVertices.at(i).nor[1] = normals[normalList[triangleIndices[i]]].y;
				nodeVertices.at(i).nor[2] = normals[normalList[triangleIndices[i]]].z;

				nodeVertices.at(i).uv[0] = u[uvIDs[triangleIndices[i]]];
				nodeVertices.at(i).uv[1] = v[uvIDs[triangleIndices[i]]];


				//indexing: if the tempMesh contains current vertex we list the earlier one in the indexlist;
				bool VertexIsUnique = true;
				for (size_t j = 0; j < tempMesh.Vertices.size(); j++) 
				{

					if (nodeVertices.at(i) == tempMesh.Vertices.at(j))
					{
						tempMesh.indexes.push_back(j);
						VertexIsUnique = false;
						break; // get out of loop
					}

				}
					if (VertexIsUnique == true)
					{
						tempMesh.indexes.push_back(tempMesh.Vertices.size());
						tempMesh.Vertices.push_back(nodeVertices.at(i));
					}


			}//Vertex END

			Meshes.push_back(tempMesh);

		}//Mesh END
		it.next();
	}//Loop END 
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
