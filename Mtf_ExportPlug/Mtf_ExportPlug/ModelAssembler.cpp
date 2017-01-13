#include"ModelAssembler.h"

ModelAssembler::ModelAssembler()
{
	//AssembleMeshes();
	AssembleSkeletalMesh();
	//AssembleMaterials();

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
			//JAG MÅSTE GÖRA EN LISTA MED INFLUENSERNA OCH TYNGDERNA
			
			MFnDependencyNode meshnode(it.currentItem());
			MItMeshVertex VertexData(it.currentItem());
			MString name = meshnode.name(); // we have the mesh name! how are we going to get the transform;
			MItGeometry geometryIterator(it.currentItem(), &res);

			//indices
			MFnMesh meshNode(it.currentItem()); //the meshNode
			MIntArray triangleIndices;
			MIntArray triangleCountsOffsets;
			meshNode.getTriangleOffsets(triangleCountsOffsets, triangleIndices);
			vector<unsigned int> indexes;
			vector<unsigned int> Count;
			vector<unsigned int> List;
			//test indexes:: FAILURE

			for (size_t i = 0; i < triangleIndices.length(); i++)
			{
				indexes.push_back(triangleIndices[i]);
			}

			MIntArray vertexCount;
			MIntArray vertexList;
			meshNode.getVertices(vertexCount,vertexList);

			for (size_t i = 0; i < vertexCount.length(); i++)
			{
				Count.push_back(vertexCount[i]);
			}
			for (size_t i = 0; i < vertexList.length(); i++)
			{
				List.push_back(vertexList[i]);
			}


			vector<SkeletonVertex>vertexVector;
			for (size_t VertexIndex = 0; geometryIterator.isDone() == false; geometryIterator.next(), VertexData.next(), VertexIndex++)
			{
				float2 UV;
				SkeletonVertex vertex;
				int UVCount; res = VertexData.numUVs(UVCount);

				for (size_t i = 0; i < UVCount; i++)
				{
					res = VertexData.getUV(UV);
				}
				MPoint position = VertexData.position();
				MVector normal; res = VertexData.getNormal(normal);
				res = VertexData.getUV(UV);


				for (size_t i = 0; i < 3; i++)
				{
					vertex.pos[i] = position[i];
					vertex.normal[i] = normal[i];
				}

				for (size_t i = 0; i < 2; i++)
					vertex.uv[i] = UV[i];

				vertexVector.push_back(vertex);
			}

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
	//check against skinClusters
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
				ProcessSkeletalVertex(skinCluster, skeleton); //this is not done
				ProcessKeyframes(skinCluster, skeleton);

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
		Joint joint;
		MFnDependencyNode jointDep(jointArray[i].node());
		joint.name = jointDep.name();
		MPlug bindPosePlug = jointDep.findPlug("bindPose", &res);

		MObject data;
		bindPosePlug.getValue(data);
		MFnMatrixData mData(data);
		MMatrix inverseBindPoseMatrix = mData.matrix(&res).inverse(); //this is the bindpose. It is called world matrix in plugs

		
		for (size_t i = 0; i < 4; i++) //MMatrix transfered to Joints float[16]
		{
			for (size_t j = 0; j < 4; j++)
			{
				joint.bindPoseInverse[(i * 4 + j)] = inverseBindPoseMatrix[i][j];
			}
		}
		
	
		GetJointParentID(jointDep, joint);
		//globalinversebindpose?
		skeleton.jointVector.push_back(joint);
		
	}
}

void ModelAssembler::GetJointParentID(MFnDependencyNode& jointDep, Joint& joint)
{
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
	
	//globalinversebindpose?
}

vector<vertexDeform> ModelAssembler::GetSkinWeights(MDagPath skinPath,MFnSkinCluster& skinCluster, vector<Joint>joints)
{
	vector<vertexDeform> vertexDeformVector;
	MItGeometry geometryIterator(skinPath, &res);

	for (size_t VertexIndex = 0; geometryIterator.isDone() == false; geometryIterator.next(), VertexIndex++)
	{
		MObject comp = geometryIterator.component(&res);
		MDoubleArray weights;
		unsigned int infCount;
		res = skinCluster.getWeights(skinPath, comp, weights, infCount);

		vertexDeform deform;
		for (size_t i = 0, influenceIndex = 0; i < joints.size(); i++)
		{
			
			if (weights[i] > 0)
			{
				deform.influences[influenceIndex] = i;
				deform.weights[influenceIndex] = weights[i];
				influenceIndex++;
				if (influenceIndex >= 4) //Currently dont support only 4 influences 
				{
					break;
				}

			}
		}
		vertexDeformVector.push_back(deform);

	}// This gives a list of indexes. We probably can use this with the vertexes that give position.
	return vertexDeformVector;
}

void ModelAssembler::ProcessSkeletalVertex(MFnSkinCluster& skinCluster, Skeleton& skeleton)
{

	//right now we dont have the parent.

	unsigned int geometryCount = skinCluster.numOutputConnections();

	for (size_t i = 0; i < geometryCount; ++i) //if several meshes are connected go trough them all.
	{
		unsigned int index = skinCluster.indexForOutputConnection(i, &res); 


		MDagPath skinPath;  res = skinCluster.getPathAtIndex(index, skinPath); //get weights for each vertex {8 in a cube}
		MFnMesh meshNode(skinPath.node()); //the meshNode
		MFnDependencyNode meshnode(skinPath.node());
		MString name = meshnode.name(); //for debuging purposes

		//weights
		vector<vertexDeform>VertexDeformVector = GetSkinWeights(skinPath, skinCluster, skeleton.jointVector);

		//Positions Normals UVs.
		vector<SkeletonVertex>nodeVertices;
		MFloatPointArray pts; //this is used for positions
		MIntArray vertexCounts;
		MIntArray polygonVertexIDs; 
		MFloatArray u, v;
		MIntArray uvCounts;
		MIntArray uvIDs; 
		MFloatVectorArray normals;
		MIntArray triangleCounts;
		MIntArray triangleVertexIDs;
		MVector vertexNormal;
		MIntArray normalList, normalCount; // here is the normals

		meshNode.getPoints(pts, MSpace::kObject);
		meshNode.getUVs(u, v, 0);
		meshNode.getAssignedUVs(uvCounts, uvIDs); //indices for UV:s

		meshNode.getVertices(vertexCounts, polygonVertexIDs); //get vertex polygon indices


		meshNode.getNormals(normals, MSpace::kObject);
		
		meshNode.getNormalIds(normalCount, normalList);

		//indices
		MIntArray triangleIndices;
		MIntArray triangleCountsOffsets;
		meshNode.getTriangleOffsets(triangleCountsOffsets, triangleIndices);
		nodeVertices.resize(triangleIndices.length());

	
		//for debuging purposes
		//triangle indice är längre än polygon ids...Men triangle indice refererar till polygon ids.
		vector<unsigned int> indexes;
		vector<unsigned int> PolygonIDs;
		int TriangleIndicerSize = triangleIndices.length();
		int PolygonIDsSize = polygonVertexIDs.length();
		for (size_t i = 0; i < triangleIndices.length(); i++)
			indexes.push_back(triangleIndices[i]);
		
		for (size_t i = 0; i < polygonVertexIDs.length(); i++)
			PolygonIDs.push_back(polygonVertexIDs[i]);
		//end of debugingPurposes


		//now lets get all triangleVertexes.
		for (unsigned int i = 0; i < triangleIndices.length(); i++)
		{ 

			int JAG = polygonVertexIDs[triangleIndices[i]];

			nodeVertices.at(i).pos[0] = pts[polygonVertexIDs[triangleIndices[i]]].x;
			nodeVertices.at(i).pos[1] = pts[polygonVertexIDs[triangleIndices[i]]].y;
			nodeVertices.at(i).pos[2] = pts[polygonVertexIDs[triangleIndices[i]]].z;

			nodeVertices.at(i).normal[0] = normals[normalList[triangleIndices[i]]].x;
			nodeVertices.at(i).normal[1] = normals[normalList[triangleIndices[i]]].y;
			nodeVertices.at(i).normal[2] = normals[normalList[triangleIndices[i]]].z;

			nodeVertices.at(i).uv[0] = u[uvIDs[triangleIndices[i]]];
			nodeVertices.at(i).uv[1] = v[uvIDs[triangleIndices[i]]];

			nodeVertices.at(i).deformer = VertexDeformVector[ polygonVertexIDs[ triangleIndices[i]]];
			//Vi kollar till vilken vertex trianglen pekar på och hämtar den vikten
		}

		skeleton.skeletalVertexVector = nodeVertices; //so far im only planing on having one list per skeleton.
	
	} //End of Current Mesh Looping to next
}



void ModelAssembler::ProcessKeyframes(MFnSkinCluster & skinCluster, Skeleton & skeleton)
{
	MDagPathArray jointArray;
	skinCluster.influenceObjects(jointArray, &res);
	//Get all animationlayers

	//pymel will be used
	MGlobal::executePythonCommandStringResult("import maya.mel as mel", true, true);
	MGlobal::executePythonCommand("import Keyframes as k");
	MGlobal::executePythonCommand("k = reload(Keyframes)");


	vector<MString> animLayers = GetAnimLayers("BaseAnimation");
	sImAnimationState keyList;
	for (Joint joint : skeleton.jointVector)
	{

		joint.animationStateCount = animLayers.size();
		for (MString layer : animLayers)
		{
			MuteAllLayersExcept(animLayers, layer);
			MGlobal::executePythonCommand("keyframes = []");
			MGlobal::executePythonCommand("k.GetJointKeyframes(\"" + joint.name + " \", keyframes)");
			MString amountOfKeyframes = MGlobal::executePythonCommandStringResult("len(keyframes)");
			
			for (int keyIndex = 0; keyIndex < amountOfKeyframes.asInt(); keyIndex++)
			{
				MString keyIndexStringed; keyIndexStringed = keyIndex;
				sKeyFrame keyframe;

				MString time = MGlobal::executePythonCommandStringResult("keyframes[" + keyIndexStringed + "].time");

				MString scaleX = MGlobal::executePythonCommandStringResult("keyframes[" + keyIndexStringed + "].scale[0]");
				MString scaleY = MGlobal::executePythonCommandStringResult("keyframes[" + keyIndexStringed + "].scale[1]");
				MString scaleZ = MGlobal::executePythonCommandStringResult("keyframes[" + keyIndexStringed + "].scale[2]");

				MString translateX = MGlobal::executePythonCommandStringResult("keyframes[" + keyIndexStringed + "].translate[0]");
				MString translateY = MGlobal::executePythonCommandStringResult("keyframes[" + keyIndexStringed + "].translate[1]");
				MString translateZ = MGlobal::executePythonCommandStringResult("keyframes[" + keyIndexStringed + "].translate[2]");

				MString rotationX = MGlobal::executePythonCommandStringResult("keyframes[" + keyIndexStringed + "].rotation[0]");
				MString rotationY = MGlobal::executePythonCommandStringResult("keyframes[" + keyIndexStringed + "].rotation[1]");
				MString rotationZ = MGlobal::executePythonCommandStringResult("keyframes[" + keyIndexStringed + "].rotation[2]");
				MEulerRotation rotation(rotationX.asDouble(), rotationY.asDouble(), rotationZ.asDouble());
				MQuaternion quaternion = rotation.asQuaternion();

				keyframe.keyTime = time.asInt();
				keyframe.keyScale[0] = scaleX.asDouble();
				keyframe.keyScale[1] = scaleY.asDouble();
				keyframe.keyScale[2] = scaleZ.asDouble();
				keyframe.keyTranslate[0] = translateX.asDouble();
				keyframe.keyTranslate[1] = translateY.asDouble();
				keyframe.keyTranslate[2] = translateZ.asDouble();

				keyframe.keyRotate[0] = quaternion.w;
				keyframe.keyRotate[1] = quaternion.x;
				keyframe.keyRotate[2] = quaternion.y;
				keyframe.keyRotate[3] = quaternion.z;

				keyList.keyList.push_back(keyframe);
			}
			joint.animationState.push_back(keyList);

		}
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

vector<MString> ModelAssembler::GetAnimLayers(const MString baseLayer)
{
	MStatus res;

	MGlobal::executePythonCommand("Layers = mel.eval('animLayer - query - children \"BaseAnimation\";')", true, true);
	MString animationLayerCount = MGlobal::executePythonCommandStringResult("len(Layers)", true, true, &res);

	vector<MString> Layers;
	for (int i = 0; i < animationLayerCount.asInt(); i++)
	{
		MString index; index += i;
		MString temp = MGlobal::executePythonCommandStringResult("Layers["+ index +"]");
		Layers.push_back(temp);
	}
	
	return Layers;
	

	
}

void ModelAssembler::MuteAllLayersExcept(vector<MString> allLayers, MString ExceptLayer)
{
	for (MString layer : allLayers)
	{
		MGlobal::executeCommand("animLayer - edit - mute true " + layer + ";");
		MGlobal::executeCommand("animLayerEditorOnSelect \"" + layer +"\" 0;");
	}

	MGlobal::executeCommand("animLayer - edit - mute false " + ExceptLayer + ";");
	MGlobal::executeCommand("animLayerEditorOnSelect \"" + ExceptLayer + "\" 1;");
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
