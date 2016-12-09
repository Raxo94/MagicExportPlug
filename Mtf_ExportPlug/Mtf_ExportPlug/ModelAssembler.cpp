#include"ModelAssembler.h"

ModelAssembler::ModelAssembler()
{
	//AssembleMeshes();
	AssembleSkeletalMesh();
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

			vector<Vertex>nodeVertices;
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
	Skeleton skeleton;
	MPlugArray skinClusterArray;

	MItDag it(MItDag::kDepthFirst);

	for (; it.isDone() == false; it.next())
	{
		if (it.currentItem().hasFn(MFn::kMesh))
		{
			//get the cluster from the mesh
			MFnDependencyNode depNode(it.currentItem());
			MPlug inMesh = depNode.findPlug("inMesh", &res);
			inMesh.connectedTo(skinClusterArray, true, false, &res);
			MFnSkinCluster skinCluster(skinClusterArray[0].node(), &res);
			if (res == true)
			{
				MString name = skinCluster.name(&res);

				//Get the joints
				ProcessInverseBindpose(skinCluster,skeleton);
				ProcessSkeletalVertex(skinCluster, skeleton);
				ProcessKeyframes(skinCluster, skeleton);
				//i still need animationlayers

				//animationstatecount?

			}
			 
		} //End of Mesh
	} //End of Node
}

void ModelAssembler::ProcessInverseBindpose(MFnSkinCluster& skinCluster, Skeleton& skeleton)
{
	MDagPathArray jointArray;
	skinCluster.influenceObjects(jointArray, &res);

	for (size_t i = 0; i < jointArray.length(); i++)
	{
		MFnDependencyNode jointDep(jointArray[i].node());
		MString jointName = jointDep.name();
		MPlug bindPosePlug = jointDep.findPlug("bindPose", &res);

		MObject data;
		bindPosePlug.getValue(data);
		MFnMatrixData mData(data);
		MMatrix inverseBindPoseMatrix = mData.matrix(&res).inverse(); //this is the bindpose. It is called world matrix in plugs
		

		Joint joint;

		//MMatrix transfered to Joints float[16]
		for (size_t i = 0; i < 4; i++)
		{
			for (size_t j = 0; j < 4; j++)
				joint.bindPoseInverse[(i*4 + j)] = inverseBindPoseMatrix[i][j];
		}
		
	
		//Joint plug //parents 
		MPlug messagePlug = jointDep.findPlug("message", &res);
		MString nameJointMessage = messagePlug.name();

		MPlugArray messageArray;
		messagePlug.connectedTo(messageArray, false, true, &res);

		if (messageArray.length() > 0)
		{
			MPlug memberPlug(messageArray[0]);
			MString nameMember = memberPlug.name();
			joint.ID = memberPlug.logicalIndex(&res);

			MPlugArray parentArray;


			memberPlug.connectedTo(parentArray, false, true, &res);
			if (parentArray.length()>0)
			{
				MPlug parentPlug(parentArray[0]);
			
				joint.parentID = parentPlug.logicalIndex(&res);
				MString nameParent = parentPlug.name();
			}

		}
		skeleton.jointVector.push_back(joint);
		//globalinversebindpose?
	}
}

void ModelAssembler::ProcessSkeletalVertex(MFnSkinCluster& skinCluster, Skeleton& skeleton)
{
	unsigned int geometryCount = skinCluster.numOutputConnections();
	for (size_t i = 0; i < geometryCount; ++i)
	{
		unsigned int index = skinCluster.indexForOutputConnection(i, &res);

		MDagPath skinPath;
		res = skinCluster.getPathAtIndex(index, skinPath);

		MItGeometry geometryIterator(skinPath, &res);
		int vertexCount = geometryIterator.count();
		double debugViewOfWeights[5000];

		for (size_t VertexIndex = 0; geometryIterator.isDone() == false; geometryIterator.next(), VertexIndex++)
		{
			SkeletonVertex skeletalVertex; // to be appended

			MObject comp = geometryIterator.component(&res);
			MDoubleArray weights;

			unsigned int infCount;
			res = skinCluster.getWeights(skinPath, comp, weights, infCount);
			weights.get(debugViewOfWeights);

			for (size_t i = 0, influenceIndex = 0; i < skeleton.jointVector.size(); i++)
			{
				if (weights[i]>0)
				{
					skeletalVertex.influences[influenceIndex] = i;
					skeletalVertex.weights[influenceIndex] = weights[i];
					influenceIndex++;
					if (influenceIndex >= 4) //we dont support more than 4 influences 
					{
						break;
					}
				}
			} //End of jointCount

			skeleton.skeletalVertexVector.push_back(skeletalVertex);
		} //End of Current Geometry

	}//End of ALL Geometry
}

void ModelAssembler::ProcessKeyframes(MFnSkinCluster & skinCluster, Skeleton & skeleton)
{
	MDagPathArray jointArray;
	skinCluster.influenceObjects(jointArray, &res);

	int var;
	for (size_t i = 0; i < skeleton.jointVector.size(); i++)
	{
		MFnDependencyNode jointDep(jointArray[i].node());
		MString jointName = jointDep.name();
		MPlug Joint = jointDep.findPlug("translateX", &res);

		//we need to find the animationLAYERS
		sKeyFrame keframes;

		keframes.keyRotate;
		keframes.keyScale;
		keframes.keyTime;
		keframes.keyTranslate;
		
		var = 10;
		bool exists;
		MGlobal::executePythonCommand("var=5");
		MGlobal::executePythonCommand("print (var)");
		var = MGlobal::optionVarIntValue("var",&exists); //no work
		MGlobal::executePythonCommand("var", var);


		def start(self):
			curr = first = pm.findKeyframe(self.sourceRootNode, which = 'first')
			last = pm.findKeyframe(self.sourceRootNode, which = 'last')
			


		//executePythonCommand(const MString& command,int& result, bool displayEnabled = false, bool undoEnabled = false);



		//static int			optionVarIntValue(const MString& name, bool *exists = NULL);
		//MAnimControl;
		//MFnAnimCurve;


	}
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
		try
		{
			MPlug normalCamera = materialNode.findPlug("normalCamera");
			normalCamera.connectedTo(bmpGroup, true, false, &res);
			if(bmpGroup.length()>0)
			{
				MFnDependencyNode bmpMap(bmpGroup[0].node());
				MPlug bumpValue = bmpMap.findPlug("bumpValue");

				////normal Texture
				bumpValue.connectedTo(textureGroup, true, false, &res);
				tempMaterial.normalFilepath = GetTexture(textureGroup);
				//tempMaterial.normalFilepath = { 0 };
			}
			
		}
		catch (int e)
		{
			cout << "An exception occurred. Exception Nr. " << e << '\n';
			tempMaterial.normalFilepath = { 0 };
		}
		

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


bool assembleStructs::operator==(const assembleStructs::Vertex & left, const assembleStructs::Vertex & right)
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
