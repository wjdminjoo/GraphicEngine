#include "FbxLoader.h"
#include <assert.h>
#include <iostream>

FbxManager* _FbxManager = nullptr;
FBXVERTEX* positions = nullptr;

HRESULT FbxLoader::LoadFBX(char* FileName)
{
	_FbxManager = FbxManager::Create();

	if (_FbxManager == nullptr)
		return false;

	FbxIOSettings* _FbxIOSettings = FbxIOSettings::Create(_FbxManager, IOSROOT);
	_FbxManager->SetIOSettings(_FbxIOSettings);


	FbxImporter* _FbxImporter = FbxImporter::Create(_FbxManager, "FirstImporter");

	if (_FbxImporter->Initialize(FileName, -1, _FbxManager->GetIOSettings())) {
		printf("Call to FbxImporter::Initialize() failed.\n");
		printf("Error returned: %s\n\n", _FbxImporter->GetStatus().GetErrorString());
		exit(-1);
	}

	FbxScene* _FbxScene = FbxScene::Create(_FbxManager, "FirstScene");

	_FbxImporter->Import(_FbxScene);
	int lFileMajor, lFileMinor, lFileRevision;

	_FbxImporter->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);


	_FbxImporter->Destroy();

	FbxAxisSystem sceneAxisSystem = _FbxScene->GetGlobalSettings().GetAxisSystem(); // ¾À ³»ÀÇ ÁÂÇ¥ÃàÀ» ¹Ù²Û´Ù. 
	FbxAxisSystem::DirectX.ConvertScene(_FbxScene); // ¾À ³»¿¡¼­ »ï°¢ÇüÈ­ ÇÒ ¼ö ÀÖ´Â ¸ðµç ³ëµå¸¦ »ï°¢ÇüÈ­ ½ÃÅ²´Ù. 
	FbxGeometryConverter geometryConverter(_FbxManager);
	geometryConverter.Triangulate(_FbxScene, true);

	FbxNode* lRootNode = _FbxScene->GetRootNode();
	FbxNode* meshNode = FbxNode::Create(_FbxScene, "meshNode");

	lRootNode->AddChild(meshNode);

	LoadNode(lRootNode); // Àç±Í·Î ³ëµå Å½»ö

	FbxMesh* mesh = FbxMesh::Create(_FbxScene, "mesh");
	
	mesh = meshNode->GetMesh();

	meshNode->SetNodeAttribute(mesh);

	

	ProcessControlPoints(mesh);

	unsigned int triangleCount = mesh->GetPolygonCount();
	unsigned int vertexCount = 0;

	for (unsigned int i = 0; i < triangleCount; ++i) {
		for (unsigned int j = 0; j < 3; ++j) {
			int controlPointIndex = mesh->GetPolygonVertex(i, j);

			FBXVERTEX& position = positions[controlPointIndex];
			FBXVERTEX normal = ReadNormal(mesh, controlPointIndex, vertexCount);

			vertexCount++;
		}
	}

	FbxExporter* _fbxExporter = FbxExporter::Create(_FbxManager, "FirstExporter");

	bool exportStatus = _fbxExporter->Initialize(FileName, -1, _FbxManager->GetIOSettings());

	if (!exportStatus) {
		printf("Call to FbxExporter::Initialize() failed.\n");
		printf("Error returned: %s\n\n", _fbxExporter->GetStatus().GetErrorString());
		return false;
	}

	_fbxExporter->Export(_FbxScene);

	
	_fbxExporter->Destroy();
	_FbxManager->Destroy();

}

FbxString FbxLoader::GetAttributeTypeName(FbxNodeAttribute::EType type)
{
	switch (type) {
	case FbxNodeAttribute::eUnknown: return "unidentified";
	case FbxNodeAttribute::eNull: return "null";
	case FbxNodeAttribute::eMarker: return "marker";
	case FbxNodeAttribute::eSkeleton: return "skeleton";
	case FbxNodeAttribute::eMesh: return "mesh";
	case FbxNodeAttribute::eNurbs: return "nurbs";
	case FbxNodeAttribute::ePatch: return "patch";
	case FbxNodeAttribute::eCamera: return "camera";
	case FbxNodeAttribute::eCameraStereo: return "stereo";
	case FbxNodeAttribute::eCameraSwitcher: return "camera switcher";
	case FbxNodeAttribute::eLight: return "light";
	case FbxNodeAttribute::eOpticalReference: return "optical reference";
	case FbxNodeAttribute::eOpticalMarker: return "marker";
	case FbxNodeAttribute::eNurbsCurve: return "nurbs curve";
	case FbxNodeAttribute::eTrimNurbsSurface: return "trim nurbs surface";
	case FbxNodeAttribute::eBoundary: return "boundary";
	case FbxNodeAttribute::eNurbsSurface: return "nurbs surface";
	case FbxNodeAttribute::eShape: return "shape";
	case FbxNodeAttribute::eLODGroup: return "lodgroup";
	case FbxNodeAttribute::eSubDiv: return "subdiv";
	default: return "unknown";
	}
}

void FbxLoader::PrintAttribute(FbxNodeAttribute* pAttribute)
{
	if (!pAttribute) return;

	FbxString typeName = GetAttributeTypeName(pAttribute->GetAttributeType());
	FbxString attrName = pAttribute->GetName();
	PrintTabs();
	// Note: to retrieve the character array of a FbxString, use its Buffer() method.
	printf("<attribute type='%s' name='%s'/>\n", typeName.Buffer(), attrName.Buffer());
}

void FbxLoader::LoadNode(FbxNode* node)
{
	FbxNodeAttribute* nodeAttribute = node->GetNodeAttribute();

	if (nodeAttribute) {
		if (nodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh) {
			FbxMesh* mesh = node->GetMesh();
		}
	}

	const int childCount = node->GetChildCount();
	for (unsigned int i = 0; i < childCount; ++i) {
		LoadNode(node->GetChild(i));
	}
}

void FbxLoader::ProcessControlPoints(FbxMesh* mesh)
{

	unsigned int count = mesh->GetControlPointsCount();
	positions = new FBXVERTEX[count];

	for (unsigned int i = 0; i < count; ++i) {
		FBXVERTEX position;
		position.x = static_cast<float>(mesh->GetControlPointAt(i).mData[0]);
		position.y = static_cast<float>(mesh->GetControlPointAt(i).mData[1]);
		position.z = static_cast<float>(mesh->GetControlPointAt(i).mData[2]);
	}

	// Define the eight corners of the cube.
// The cube spans from
//    -50 to  50 along the X axis
//      0 to 100 along the Y axis
//    -50 to  50 along the Z axis
	FbxVector4 vertex0(-50, 0, 50);
	FbxVector4 vertex1(50, 0, 50);
	FbxVector4 vertex2(50, 100, 50);
	FbxVector4 vertex3(-50, 100, 50);
	FbxVector4 vertex4(-50, 0, -50);
	FbxVector4 vertex5(50, 0, -50);
	FbxVector4 vertex6(50, 100, -50);
	FbxVector4 vertex7(-50, 100, -50);

	// Initialize the control point array of the mesh.
	mesh->InitControlPoints(24);
	FbxVector4* lControlPoints = mesh->GetControlPoints();

	// Define each face of the cube.
	// Face 1
	lControlPoints[0] = vertex0;
	lControlPoints[1] = vertex1;
	lControlPoints[2] = vertex2;
	lControlPoints[3] = vertex3;
	// Face 2
	lControlPoints[4] = vertex1;
	lControlPoints[5] = vertex5;
	lControlPoints[6] = vertex6;
	lControlPoints[7] = vertex2;
	// Face 3
	lControlPoints[8] = vertex5;
	lControlPoints[9] = vertex4;
	lControlPoints[10] = vertex7;
	lControlPoints[11] = vertex6;
	// Face 4
	lControlPoints[12] = vertex4;
	lControlPoints[13] = vertex0;
	lControlPoints[14] = vertex3;
	lControlPoints[15] = vertex7;
	// Face 5
	lControlPoints[16] = vertex3;
	lControlPoints[17] = vertex2;
	lControlPoints[18] = vertex6;
	lControlPoints[19] = vertex7;
	// Face 6
	lControlPoints[20] = vertex1;
	lControlPoints[21] = vertex0;
	lControlPoints[22] = vertex4;
	lControlPoints[23] = vertex5;
}

FBXVERTEX FbxLoader::ReadNormal(const FbxMesh* mesh, int controlPointIndex, int vertexCounter)
{
	if (mesh->GetElementNormalCount() < 1)
		std::cout << "No Normals" << std::endl;

	const FbxGeometryElementNormal* vertexNormal = mesh->GetElementNormal(0); // ³ë¸» È¹µæ
	FBXVERTEX result;

	switch (vertexNormal->GetMappingMode()) {
	case FbxGeometryElement::eByControlPoint:
		switch (vertexNormal->GetReferenceMode()) {

		case FbxGeometryElement::eDirect:
		{
			result.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(controlPointIndex).mData[0]);
			result.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(controlPointIndex).mData[1]);
			result.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(controlPointIndex).mData[2]);
		}
			break;
		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexNormal->GetIndexArray().GetAt(controlPointIndex);

			result.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[0]);
			result.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[1]);
			result.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[2]);
		}
			break;
		break;
		}
	case FbxGeometryElement::eByPolygonVertex:
		switch (vertexNormal->GetReferenceMode()) {
		case FbxGeometryElement::eDirect:
		{
			result.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(controlPointIndex).mData[0]);
			result.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(controlPointIndex).mData[1]);
			result.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(controlPointIndex).mData[2]);
		}
			break;
		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexNormal->GetIndexArray().GetAt(controlPointIndex);

			result.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[0]);
			result.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[1]);
			result.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[2]);
		}
			break;
		}
		break;
	}
	return result;
}

void FbxLoader::PrintNode(FbxNode* pNode)
{
	PrintTabs();
	const char* nodeName = pNode->GetName();
	FbxDouble3 translation = pNode->LclTranslation.Get();
	FbxDouble3 rotation = pNode->LclRotation.Get();
	FbxDouble3 scaling = pNode->LclScaling.Get();

	// Print the contents of the node.
	printf("<node name='%s' translation='(%f, %f, %f)' rotation='(%f, %f, %f)' scaling='(%f, %f, %f)'>\n",
		nodeName,
		translation[0], translation[1], translation[2],
		rotation[0], rotation[1], rotation[2],
		scaling[0], scaling[1], scaling[2]
	);
	numTabs++;

	// Print the node's attributes.
	for (int i = 0; i < pNode->GetNodeAttributeCount(); i++)
		PrintAttribute(pNode->GetNodeAttributeByIndex(i));

	// Recursively print the children.
	for (int j = 0; j < pNode->GetChildCount(); j++)
		PrintNode(pNode->GetChild(j));

	numTabs--;
	PrintTabs();
	printf("</node>\n");

}

void FbxLoader::PrintTabs()
{
	for (int i = 0; i < numTabs; i++)
		printf("\t");
}


