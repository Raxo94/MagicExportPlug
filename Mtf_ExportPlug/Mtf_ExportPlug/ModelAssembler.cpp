#include"ModelAssembler.h"

ModelAssembler::ModelAssembler()
{
	AssembleMeshes();
	AssembleMaterials();
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

void ModelAssembler::AssembleSkeletalMesh()
{
	;//Nothing here
}

void ModelAssembler::AssembleMaterials()
{
	MItDependencyNodes itDepNode(MFn::kLambert); //Is used inorder to go trough materials
	while (itDepNode.isDone() == false)
	{
		std::array<char, 256> meshName;
		MObject mNode = itDepNode.item();
		Material tempMaterial;

		MPlugArray textureGroup;
		MPlugArray shadingGoupArray; //our own variable
		MPlugArray dagSetMemberConnections;
		MPlugArray objInstArray;
		MFnDependencyNode materialNode(mNode);
		MString materialName = materialNode.name();

		//Get MaterialNode Plugs
		MPlug outColor = materialNode.findPlug("outColor"); //to go further in the plugs
		MPlug color = materialNode.findPlug("color"); //to get the color values
		MPlug diffuse = materialNode.findPlug("diffuse"); //to get the diffuse of the material
		

		MObject data;
		color.getValue(data);
		MFnNumericData nData(data);
		nData.getData(tempMaterial.color[0], tempMaterial.color[1], tempMaterial.color[2]);
		diffuse.getValue(tempMaterial.diffuse);

		tempMaterial.hasTexture = false;
		
		color.connectedTo(textureGroup, true, false, &res); //color is connected to a destination
		//if the length is not 0 then we have a texture
		for (int i = 0; i < textureGroup.length(); i++)
		{
			MFnDependencyNode textureNode(textureGroup[i].node());
			MString filename;
			textureNode.findPlug("fileTextureName").getValue(filename);
			memcpy(&tempMaterial.textureFilepath, filename.asChar(), sizeof(const char[256]));
			tempMaterial.hasTexture = true;
		}
		
		outColor.connectedTo(shadingGoupArray, false, true, &res);
		for (int i = 0; i < shadingGoupArray.length(); i++)  //for all shading groups
		{
			if (shadingGoupArray[i].node().hasFn(MFn::kShadingEngine)) 
			{
				MFnDependencyNode shadingNode(shadingGoupArray[i].node());
				if (strcmp(shadingNode.name().asChar(), "initialParticleSE") != 0) //We want to ignore this shading group 
				{

					MPlug dagSetMember = shadingNode.findPlug("dagSetMembers", &res);
					for (int child = 0; child < dagSetMember.numElements(); child++) 
					{
						dagSetMember[child].connectedTo(dagSetMemberConnections, true, false, &res); //true = connection to destination

						for (int d = 0; d < dagSetMemberConnections.length(); d++) 
						{
							MFnDependencyNode dagSetMemberNode(dagSetMemberConnections[d].node()); //in order to get materials meshes
							if (strcmp(dagSetMemberNode.name().asChar(), "shaderBallGeom1") != 0) 
							{
								MFnMesh mesh(dagSetMemberNode.object()); //get the mesh in order to get the name of the mesh
								memcpy(&meshName, mesh.name().asChar(), sizeof(char[256])); //get the name of mesh bound to material
								tempMaterial.boundMeshes.push_back(meshName);

							}
						}
					}
				}
			}
		}

		itDepNode.next();
	}
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
