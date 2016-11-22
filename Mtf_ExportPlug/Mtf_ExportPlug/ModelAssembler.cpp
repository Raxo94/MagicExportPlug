#include"ModelAssembler.h"

ModelAssembler::ModelAssembler()
{
	AssembleMeshes();
	AssembleMaterials();

}

ModelAssembler::~ModelAssembler()
{
}

vector<Mesh>& ModelAssembler::GetMeshVector()
{
	return Meshes;
}

vector<Material>& ModelAssembler::GetMaterialVector()
{
	return materials;
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

			memcpy(&tempMesh.MeshName, meshNode.name().asChar(), sizeof(const char[256]));//MeshName

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
			tempMesh.vertexCount = tempMesh.Vertices.size();
			if (parent.hasFn(MFn::kTransform))
			{
				MFnTransform transformNode(parent);
				tempMesh.transform = GetTransform(transformNode);
			}
			
			Meshes.push_back(tempMesh);

		}//Mesh END
		parent = it.currentItem();
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
	itDepNode.reset(MFn::kLambert); //not helping the lambert issue. We find the shading groups but not the nodes

	while (itDepNode.isDone() == false)
	{
		std::array<char, 256> meshName;
		MObject mNode = itDepNode.item();
		Material tempMaterial;

		MPlugArray textureGroup;
		MPlugArray shadingGoupArray;
		MPlugArray dagSetMemberConnections;
		MPlugArray objInstArray;
		MPlugArray bmpGroup;

		MFnDependencyNode materialNode(mNode);
		MString materialName = materialNode.name();

		//Get MaterialNode Plugs
		MPlug outColor = materialNode.findPlug("outColor"); //to go further in the plugs
		MPlug color = materialNode.findPlug("color"); //to get the color values
		MPlug diffuse = materialNode.findPlug("diffuse"); //to get the diffuse of the material
		MPlug specularColor = materialNode.findPlug("specularColor"); //to get the specular color of the material
		
		//GetNormalMap plug
		MPlug normalCamera = materialNode.findPlug("normalCamera");
		normalCamera.connectedTo(bmpGroup, true, false, &res);
		MFnDependencyNode bmpMap (bmpGroup[0].node());
		MPlug bumpValue = bmpMap.findPlug("bumpValue");



		MObject data;

		//color
		color.getValue(data);
		MFnNumericData nData(data);
		nData.getData(tempMaterial.color[0], tempMaterial.color[1], tempMaterial.color[2]);

		//texture
		color.connectedTo(textureGroup, true, false, &res); 
		tempMaterial.textureFilepath = GetTexture(textureGroup); 
		if (textureGroup.length() > 0) tempMaterial.hasTexture = true; 
		else tempMaterial.hasTexture = false;
		
		//specular color
		specularColor.getValue(data);
		MFnNumericData specularData(data);
		specularData.getData(tempMaterial.specularColor[0], tempMaterial.specularColor[1], tempMaterial.specularColor[2]);

		//specular texture
		specularColor.connectedTo(textureGroup, true, false, &res); 
		tempMaterial.specularFilepath = GetTexture(textureGroup); 
		
		//diffuse
		diffuse.getValue(tempMaterial.diffuse);

		//diffuse Texture
		diffuse.connectedTo(textureGroup, true, false, &res); 
		tempMaterial.diffuseFilepath = GetTexture(textureGroup); 

		//normal Texture
		bumpValue.connectedTo(textureGroup, true, false, &res);
		tempMaterial.normalFilepath = GetTexture(textureGroup);
		
		
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
		materials.push_back(tempMaterial);
		itDepNode.next();
	}
}

std::array<char, 256> ModelAssembler::GetTexture(MPlugArray textureGroup)
{
	std::array<char, 256> result = {0};

	for (int i = 0; i < textureGroup.length(); i++) //this is basicaly a check if there is any texture
	{
		MFnDependencyNode textureNode(textureGroup[i].node());
		MString filename;
		textureNode.findPlug("fileTextureName").getValue(filename);
		memcpy(&result, filename.asChar(), sizeof(const char[256]));
	}
	return result;
}

Transform ModelAssembler::GetTransform(MFnTransform & transform)

{
	Transform result;
	result.translation[0] = transform.getTranslation(MSpace::kTransform).x;
	result.translation[1] = transform.getTranslation(MSpace::kTransform).y;
	result.translation[2] = transform.getTranslation(MSpace::kTransform).z;

	double dScale[3]; float fScale[3];
	transform.getScale(dScale);

	result.scale[0] = dScale[0];
	result.scale[1] = dScale[1];
	result.scale[2] = dScale[2];

	transform.getRotationQuaternion(result.rotation[0], result.rotation[1], result.rotation[2], result.rotation[3], MSpace::kTransform);

	return result;
}


bool assembleStructs::operator==(const assembleStructs::vertex & left, const assembleStructs::vertex & right)
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
