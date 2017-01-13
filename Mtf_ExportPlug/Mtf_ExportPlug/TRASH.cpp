//void ModelAssembler::AssembleMeshes()
//{
//	MItDag it(MItDag::kDepthFirst);
//	MObject parent;
//
//	while (it.isDone() == false)
//	{
//		if (it.currentItem().hasFn(MFn::kMesh))
//		{
//			MFnMesh meshNode(it.currentItem()); //the meshNode
//			Mesh tempMesh;
//
//			memcpy(&tempMesh.MeshName, meshNode.name().asChar(), sizeof(const char[256]));//MeshName
//
//			vector<Vertex>nodeVertices;
//			MFloatPointArray pts;
//			MIntArray vertexCounts;
//			MIntArray polygonVertexIDs;
//			MFloatArray u, v;
//			MIntArray uvCounts;
//			MIntArray uvIDs;
//			MFloatVectorArray normals;
//			MIntArray triangleCountsOffsets;
//			MIntArray triangleIndices;
//			MIntArray triangleCounts;
//			MIntArray triangleVertexIDs;
//			MVector vertexNormal;
//			MIntArray normalList, normalCount;
//
//			meshNode.getPoints(pts, MSpace::kObject);
//			meshNode.getUVs(u, v, 0);
//			meshNode.getAssignedUVs(uvCounts, uvIDs); //indices for UV:s
//
//			meshNode.getTriangleOffsets(triangleCountsOffsets, triangleIndices);
//			meshNode.getVertices(vertexCounts, polygonVertexIDs); //get vertex polygon indices
//
//
//			meshNode.getNormals(normals, MSpace::kObject);
//
//			nodeVertices.resize(triangleIndices.length());
//
//			meshNode.getNormalIds(normalCount, normalList);
//
//
//			for (unsigned int i = 0; i < triangleIndices.length(); i++)
//			{ //for each triangle index (36)
//
//				nodeVertices.at(i).pos[0] = pts[polygonVertexIDs[triangleIndices[i]]].x;
//				nodeVertices.at(i).pos[1] = pts[polygonVertexIDs[triangleIndices[i]]].y;
//				nodeVertices.at(i).pos[2] = pts[polygonVertexIDs[triangleIndices[i]]].z;
//
//				nodeVertices.at(i).nor[0] = normals[normalList[triangleIndices[i]]].x;
//				nodeVertices.at(i).nor[1] = normals[normalList[triangleIndices[i]]].y;
//				nodeVertices.at(i).nor[2] = normals[normalList[triangleIndices[i]]].z;
//
//				nodeVertices.at(i).uv[0] = u[uvIDs[triangleIndices[i]]];
//				nodeVertices.at(i).uv[1] = v[uvIDs[triangleIndices[i]]];
//
//
//				//indexing: if the tempMesh contains current vertex we list the earlier one in the indexlist;
//				bool VertexIsUnique = true;
//				for (size_t j = 0; j < tempMesh.Vertices.size(); j++)
//				{
//
//					if (nodeVertices.at(i) == tempMesh.Vertices.at(j))
//					{
//						tempMesh.indexes.push_back(j);
//						VertexIsUnique = false;
//						break; // get out of loop
//					}
//
//				}
//				if (VertexIsUnique == true)
//				{
//					tempMesh.indexes.push_back(tempMesh.Vertices.size());
//					tempMesh.Vertices.push_back(nodeVertices.at(i));
//				}
//
//
//			}//Vertex END
//			tempMesh.vertexCount = tempMesh.Vertices.size();
//			if (parent.hasFn(MFn::kTransform))
//			{
//				MFnTransform transformNode(parent);
//				tempMesh.transform = GetTransform(transformNode);
//			}
//
//			Meshes.push_back(tempMesh);
//
//		}//Mesh END
//		parent = it.currentItem();
//		it.next();
//	}//Loop END 
//}








/*SkeletonVertex vertex;
int UVCount; res = VertexData.numUVs(UVCount);
MPoint position = VertexData.position();
MVector normal; res = VertexData.getNormal(normal);
float2 UV; res = VertexData.getUV(UV);

for (size_t i = 0; i < 3; i++)
{
vertex.pos[i] = position[i];
vertex.normal[i] = normal[i];
}

for (size_t i = 0; i < 2; i++)
vertex.uv[i] = UV[i];
*/



//Get weights