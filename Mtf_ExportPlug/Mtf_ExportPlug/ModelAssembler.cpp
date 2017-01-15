#include"ModelAssembler.h"

ModelAssembler::ModelAssembler()
{
	
	AssembleSkeletonsAndMeshes();
	AssembleMaterials();
	ConnectMaterialsToMeshes();

	Skeletons;
	standardMeshes;
}

ModelAssembler::~ModelAssembler()
{
}

vector<Mesh>& ModelAssembler::GetMeshVector()
{
	return standardMeshes;
}

vector<Skeleton>& ModelAssembler::GetSkeletonVector()
{
	return Skeletons;
}

vector<Material>& ModelAssembler::GetMaterialVector()
{
	return materials;
}

void ModelAssembler::AssembleMesh(MObject MObjectMeshNode,MObject Parent)
{
	MFnMesh meshNode(MObjectMeshNode); 
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
	MFloatVectorArray tangents;
	MIntArray triangleCountsOffsets;
	MIntArray triangleIndices;
	MIntArray triangleCounts;
	MIntArray triangleVertexIDs;
	MVector vertexNormal;
	MIntArray normalList, normalCount;
	MIntArray tangentList, tangentCount;

	meshNode.getPoints(pts, MSpace::kObject);
	meshNode.getUVs(u, v, 0);
	meshNode.getAssignedUVs(uvCounts, uvIDs); //indices for UV:s

	meshNode.getTriangleOffsets(triangleCountsOffsets, triangleIndices);
	meshNode.getVertices(vertexCounts, polygonVertexIDs); //get vertex polygon indices


	meshNode.getNormals(normals, MSpace::kObject);
	meshNode.getTangents(tangents, MSpace::kObject);

	nodeVertices.resize(triangleIndices.length());


	meshNode.getNormalIds(normalCount, normalList);

	//jag beh�ver en lista av faces och jag beh�ver f� ut genom att ge vertex
	//meshNode.getTangentId

	for (unsigned int i = 0; i < triangleIndices.length(); i++)
	{ //for each triangle index (36) in a box

		nodeVertices.at(i).pos[0] = pts[polygonVertexIDs[triangleIndices[i]]].x;
		nodeVertices.at(i).pos[1] = pts[polygonVertexIDs[triangleIndices[i]]].y;
		nodeVertices.at(i).pos[2] = pts[polygonVertexIDs[triangleIndices[i]]].z;
		
		//first is faceId then is vertex id.
		// i need to know what face i am on...
		//nodeVertices.at(i).tan[0] = tangents[polygonVertexIDs[triangleIndices[i]]].x;


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
		//end of indexing


	}//Vertex END

	tempMesh.vertexCount = tempMesh.Vertices.size();
	MFnTransform transform(Parent); //the parent is the transform
	tempMesh.transform = GetTransform(transform);
	this->standardMeshes.push_back(tempMesh);
}

void ModelAssembler::AssembleSkeletonsAndMeshes()
{

	Skeleton skeleton;
	MPlugArray skinClusterArray;
	MObject parent;
	MItDag it(MItDag::kDepthFirst);


	for (; it.isDone() == false; it.next())
	{
		if (it.currentItem().hasFn(MFn::kMesh))
		{
			if (parent.hasFn(MFn::kTransform))
			{
				MFnDependencyNode parentNode(parent);
				MFnDependencyNode depNode(it.currentItem());
				MPlug inMesh = depNode.findPlug("inMesh", &res);
				inMesh.connectedTo(skinClusterArray, true, false, &res);
				//if we want several skinClusters. We could probably make a loop here
				MFnSkinCluster skinCluster(skinClusterArray[0].node(), &res); //maybe we should make this loop trough all until it finds a skinCluster
				if (res == true)
				{

					//Make sure it really is a skinCluster
					MString name = skinCluster.name(&res);

					//Get the joints
					
					ProcessInverseBindpose(skinCluster, skeleton, parentNode);
					ProcessSkeletalVertex(skinCluster, skeleton);
					//go trough every mesh and get indices
					for (size_t i = 0; i < skeleton.MeshVector.size(); i++)
					{
						ProcessSkeletalIndexes(skeleton.MeshVector.at(i).skeletalVertexVector, skeleton.MeshVector.at(i).indexes);
						
						MFnTransform transform= skeleton.MeshVector.at(i).Meshpath.transform();
						skeleton.MeshVector.at(i).transform = GetTransform(transform);


					}
					
					ProcessKeyframes(skinCluster, skeleton);
				}
				else //check if there is no skinCluster
				{
					AssembleMesh(it.currentItem(),parent); //make this assamble mesh instead
				}

				this->Skeletons.push_back(skeleton);

			} //End of Mesh
		}
		parent = it.currentItem();
	} //End of Node

}

void ModelAssembler::ProcessInverseBindpose(MFnSkinCluster& skinCluster, Skeleton& skeleton, MFnDependencyNode& parentNode)
{
	MDagPathArray jointArray;
	skinCluster.influenceObjects(jointArray, &res);

	for (size_t i = 0; i < jointArray.length(); i++)
	{
		Joint joint;
		MFnDependencyNode jointDep(jointArray[i].node());
		joint.name = jointDep.name();
		MString parentName = parentNode.name();

		MPlug bindPosePlug = jointDep.findPlug("bindPose", &res);
		MPlug inverseModelMatrixPlug = parentNode.findPlug("worldInverseMatrix", &res);

		

		MObject data;
		res = bindPosePlug.getValue(data);
		MFnMatrixData bindPoseData(data,&res);
		MMatrix inverseGlobalBindPoseMatrix = bindPoseData.matrix(&res).inverse(); //this is the bindpose. It is called world matrix in plugs



		//MObject data2;
		//res= inverseModelMatrixPlug.getValue(data2);
		//MFnMatrixData ModelMatrixData(data2,&res); //here the issue arises.
		//MMatrix inverseModelMatrix = ModelMatrixData.matrix(&res); //this returns false?
		
		
		//MMatrix inversebindPoseMatrix = inverseGlobalBindPoseMatrix * inverseModelMatrix; //World space * inverseModelSpace
		
		MMatrix inverseBindPoseMatrix = inverseGlobalBindPoseMatrix;

		//MMatrix transfered to Joints float[16]
		for (size_t i = 0; i < 4; i++) 
		{
			for (size_t j = 0; j < 4; j++)
			{
				joint.globalBindPoseInverse[(i * 4 + j)] = inverseGlobalBindPoseMatrix[i][j];
				joint.bindPoseInverse[(i * 4 + j)] = inverseBindPoseMatrix[i][j];
			}
		}

		GetJointParentID(jointDep, joint,skeleton.jointVector); //FIX ME MAYBE;
		skeleton.jointVector.push_back(joint);
	} // end of joint
}


void ModelAssembler::GetJointParentID(MFnDependencyNode & jointDep, Joint & currentJoint,vector<Joint>OtherJoints)
{
	//if we want child index that is possible too. using scale instead of inverse scale
	MPlugArray jointArray;
	MPlug inverseScale = jointDep.findPlug("inverseScale", &res); //just inorder to get parent
	MString parentName;
	currentJoint.ID = OtherJoints.size();
	currentJoint.parentID = 0;
	
	
	
	inverseScale.connectedTo(jointArray, true, false, &res);

	if (jointArray.length() > 0) //this gets the parent
	{
		MFnDependencyNode parent(jointArray[0].node());
		parentName = parent.name();
	}

	for (size_t i = 0; i < OtherJoints.size(); i++)
	{
		if ((parentName == OtherJoints.at(i).name))
		{
			currentJoint.parentID = OtherJoints.at(i).ID;
			break;
		}
	}


}

vector<vertexDeform> ModelAssembler::GetSkinWeightsList(MDagPath skinPath,MFnSkinCluster& skinCluster, vector<Joint>joints)
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
		SkeletalMesh skeletalMesh;
		unsigned int index = skinCluster.indexForOutputConnection(i, &res); 

		MDagPath skinPath;  res = skinCluster.getPathAtIndex(index, skinPath); //get weights for each vertex {8 in a cube}
		MFnMesh meshNode(skinPath.node()); //the meshNode
		skeletalMesh.Meshpath = skinPath;

		//getMEshName
		MFnDependencyNode meshnode(skinPath.node());
		MString name = meshnode.name(); //for debuging purposes
		std::array<char, 256> meshName;
		memcpy(&meshName, name.asChar(), name.length() * sizeof(char)); 
		skeletalMesh.meshName = meshName;

		//weights
		vector<vertexDeform>VertexDeformVector = GetSkinWeightsList(skinPath, skinCluster, skeleton.jointVector);

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
		//triangle indice �r l�ngre �n polygon ids...Men triangle indice refererar till polygon ids.
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

			nodeVertices.at(i).nor[0] = normals[normalList[triangleIndices[i]]].x;
			nodeVertices.at(i).nor[1] = normals[normalList[triangleIndices[i]]].y;
			nodeVertices.at(i).nor[2] = normals[normalList[triangleIndices[i]]].z;

			nodeVertices.at(i).uv[0] = u[uvIDs[triangleIndices[i]]];
			nodeVertices.at(i).uv[1] = v[uvIDs[triangleIndices[i]]];

			nodeVertices.at(i).deformer = VertexDeformVector[ polygonVertexIDs[ triangleIndices[i]]];

			//Vi kollar till vilken vertex trianglen pekar p� och h�mtar den vikten
		}
		skeletalMesh.skeletalVertexVector = nodeVertices;
		skeleton.MeshVector.push_back(skeletalMesh);
		//skeleton.skeletalVertexVector = nodeVertices; //so far im only planing on having one list per skeleton.
	} //End of Current Mesh Looping to next
}



void ModelAssembler::ProcessSkeletalIndexes(vector<SkeletonVertex>& unfilteredVertexVector, vector<unsigned int>& indexes)
{
	vector<SkeletonVertex> UniqueVertexes;

	for (unsigned int i = 0; i < unfilteredVertexVector.size(); i++)
	{
		bool VertexIsUnique = true; //if this remains true the vertex needs to be saved.

		for (size_t j = 0; j < UniqueVertexes.size(); j++)
		{	
			if (unfilteredVertexVector.at(i) == UniqueVertexes.at(j))
			{
				indexes.push_back(j);
				VertexIsUnique = false;
				break;
				//if there unfiltered i is not the same as any of the uiniques add it.
				//else put a note to the index it was the same as-
			}
		}
		if (VertexIsUnique == true)
		{
			indexes.push_back(UniqueVertexes.size());
			UniqueVertexes.push_back(unfilteredVertexVector.at(i));
		}
	}
	unfilteredVertexVector = UniqueVertexes;
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
	MItDependencyNodes itDepNode(MFn::kLambert); //used inorder to go trough materials
	itDepNode.reset(MFn::kLambert); // Nuk computers have an issue- lamberts will not be found.

	while (itDepNode.isDone() == false)
	{
		MObject mNode = itDepNode.item();
		Material tempMaterial;

		MPlugArray textureGroup;
		MPlugArray shadingGoupArray;
		MPlugArray dagSetMemberConnections;
		MPlugArray objInstArray;
		MPlugArray bmpGroup;
		MObject data;

		MFnDependencyNode materialNode(mNode);
		MString materialName = materialNode.name();
		memcpy(&tempMaterial.name, materialName.asChar(), materialName.length() * sizeof(char));

		//Get MaterialNode Plugs
		MPlug outColor = materialNode.findPlug("outColor"); //to go further in the plugs
		MPlug color = materialNode.findPlug("color"); //to get the color values
		MPlug diffuse = materialNode.findPlug("diffuse"); //to get the diffuse of the material
		MPlug specularColor = materialNode.findPlug("specularColor"); //to get the specular color of the material
		MPlug ShinyFactor = materialNode.findPlug("specularRollOff"); //to get the specular color of the material
		//GetNormalMap plug
		MPlug normalCamera = materialNode.findPlug("normalCamera");
		normalCamera.connectedTo(bmpGroup, true, false, &res);
		if(bmpGroup.length()>0)
		{
			MFnDependencyNode bmpMap(bmpGroup[0].node());
			MPlug bumpValue = bmpMap.findPlug("bumpValue");

			////normal Texture
			bumpValue.connectedTo(textureGroup, true, false, &res);
			tempMaterial.normalFilepath = GetTexture(textureGroup); //make
		}
		else //there where no bumpMap
		{
			memcpy(&tempMaterial.normalFilepath, "NOTEXTURE", sizeof(const char[10]));
		}
		

		//color
		color.getValue(data);
		MFnNumericData nData(data);
		nData.getData(tempMaterial.color[0], tempMaterial.color[1], tempMaterial.color[2]);

		//texture
		color.connectedTo(textureGroup, true, false, &res); 
		tempMaterial.textureFilepath = GetTexture(textureGroup); 
		if (textureGroup.length() == 0)
			memcpy(&tempMaterial.textureFilepath, "NOTEXTURE", sizeof(const char[10]));
	
		//specular color
		specularColor.getValue(data);
		MFnNumericData specularData(data);
		specularData.getData(tempMaterial.specularColor[0], tempMaterial.specularColor[1], tempMaterial.specularColor[2]);
		
		//specularRollOff
		ShinyFactor.getValue(tempMaterial.shinyFactor);

		//diffuse Texture
		diffuse.connectedTo(textureGroup, true, false, &res);
		tempMaterial.diffuseFilepath = GetTexture(textureGroup);
		if (textureGroup.length() == 0)
			memcpy(&tempMaterial.diffuseFilepath, "NOTEXTURE", sizeof(const char[10]));
		//specular texture
		specularColor.connectedTo(textureGroup, true, false, &res); 
		tempMaterial.specularFilepath = GetTexture(textureGroup); 
		if (textureGroup.length() == 0)
			memcpy(&tempMaterial.specularFilepath, "NOTEXTURE", sizeof(const char[10]));
		
		//diffuse
		diffuse.getValue(tempMaterial.diffuse);
		//diffuse Texture
		diffuse.connectedTo(textureGroup, true, false, &res); 
		tempMaterial.diffuseFilepath = GetTexture(textureGroup); 
		if (textureGroup.length() == 0)
			memcpy(&tempMaterial.diffuseFilepath, "NOTEXTURE", sizeof(const char[10]));

		
		
		//Get connected Meshes
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
								std::array<char, 256> meshName;
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

void ModelAssembler::ConnectMaterialsToMeshes()
{
	Skeletons;

	for (Material material : materials)
	{
		for (std::array<char, 256> boundMeshName : material.boundMeshes)
		{

			for (size_t i = 0; i < Skeletons.size(); i++)
			{
				for (size_t j = 0; j < Skeletons.at(i).MeshVector.size(); j++)
				{
					if (boundMeshName == Skeletons.at(i).MeshVector.at(j).meshName)
						Skeletons.at(i).MeshVector.at(j).material = material;
				} //end of skeletal meshes

			}// end of skeletons


			for (size_t j = 0; j <standardMeshes.size(); j++)
			{
				if (boundMeshName == standardMeshes.at(j).MeshName)
				{
					standardMeshes.at(j).material = material;
				}
			}//end of standard meshes

		}
	
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
		memcpy(&result, filename.asChar(), filename.length() * sizeof(const char));
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

bool assembleStructs::operator==(const assembleStructs::SkeletonVertex & left, const assembleStructs::SkeletonVertex & right)
{
	if (left.pos == right.pos)
	{
		if (left.nor == right.nor)
		{
			if (left.uv == right.uv)
			{
				if (left.deformer.influences == right.deformer.influences)
				{
					if (left.deformer.weights == right.deformer.weights)
					{
						//Deformer check may be uneccesary- need to be proven first
						return true;
					}
				}
			}	
		}
	}

	return false;
}
