///To be replacing the fbxExporter
///This plugin's main purpose is to gather all the object in the scene and send them as one model.
///The model will then be made into a binary file Ready to be loaded by an importer.


#include "ModelAssembler.h"

using namespace std;


EXPORT MStatus initializePlugin(MObject obj)
{
	MStatus res = MS::kSuccess;

	MFnPlugin myPlugin(obj, "Maya plugin", "1.0", "Any", &res);
	if (MFAIL(res)) {
		CHECK_MSTATUS(res);
		return res; //Plugin not loaded
	}

	MGlobal::displayInfo("Maya plugin loaded!");
	
	//This is where things hapen




	return MS::kSuccess;
	
}


EXPORT MStatus uninitializePlugin(MObject obj)
{
	// simply initialize the Function set with the MObject that represents
	// our plugin
	MFnPlugin plugin(obj);

	// if any resources have been allocated, release and free here before
	// returning...

	MGlobal::displayInfo("Maya plugin unloaded!");

	return MS::kSuccess;
}