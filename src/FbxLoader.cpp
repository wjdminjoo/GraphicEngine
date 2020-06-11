#include "FbxLoader.h"
#include <assert.h>

FbxManager* gFbxManager = NULL;



FbxManager* FbxLoader::globalSdkMgr()
{
    if (gFbxManager == NULL)
    {
        gFbxManager = FbxManager::Create();
        assert(gFbxManager != NULL);
    }
    return gFbxManager;
}

// 값이 제대로 들어가지 않는다.
// 어째서 NULL값일까.
void FbxLoader::Begin(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, ID3D12DescriptorHeap* cbvHeap)
{
   if (mInBeginEndPair)
       throw std::exception("Cannot Begin calls on a FBX Loader");

    mDevice = device;
    mCommandList = cmdList;
    mCbvHeap = cbvHeap;

    mInBeginEndPair = true;
}

void FbxLoader::End()
{
    if (!mInBeginEndPair)
       throw std::exception("Begin must be called before End");

    mDevice = nullptr;
    mCommandList = nullptr;
    mCbvHeap = nullptr;

    mInBeginEndPair = false;
}

void FbxLoader::globalMgrDestroy()
{
    gFbxManager->Destroy();
    gFbxManager = NULL;
}

// Create a directional camera with target
// 
FbxNode* FbxLoader::CreateCamera(FbxScene* pScene, FbxVector4 vecFrom, FbxVector4 vecTo)
{
    FbxCamera* lCamera = FbxCamera::Create(pScene, "Camera");

    // Modify some camera default settings.
    lCamera->SetApertureMode(FbxCamera::eVertical);
    lCamera->SetApertureWidth(0.816);
    lCamera->SetApertureHeight(0.612);
    lCamera->SetSqueezeRatio(0.5);

    FbxNode* lNodeCamera = FbxNode::Create(pScene, "CameraNode");
    lNodeCamera->LclTranslation.Set(vecFrom);
    lNodeCamera->SetNodeAttribute(lCamera);

    FbxMarker* lMarker = FbxMarker::Create(pScene, "TestCameraTarget");

    FbxNode* lNodeTarget = FbxNode::Create(pScene, "TestCameraTarget");
    lNodeTarget->LclTranslation.Set(vecTo);
    lNodeTarget->SetNodeAttribute(lMarker);

    lNodeCamera->SetTarget(lNodeTarget);

    return lNodeCamera;
}

// Create a light.
//
FbxNode* FbxLoader::CreateLight(FbxScene* pScene, FbxDouble3 position)
{
    FbxString lLightName;
    lLightName = "PointLight";

    FbxLight* lFbxLight = FbxLight::Create(pScene, lLightName.Buffer());

    lFbxLight->LightType.Set(FbxLight::ePoint);

    // Light Color...
    //
    FbxDouble3 lColor;
    lColor[0] = 1;
    lColor[1] = 1;
    lColor[2] = 1;
    lFbxLight->Color.Set(lColor);

    // Light Intensity...
    //
    lFbxLight->Intensity.Set(100.0);

    FbxNode* lNode = FbxNode::Create(pScene, lLightName + "Node");

    // set the node attribute
    lNode->SetNodeAttribute(lFbxLight);
    lNode->LclTranslation.Set(position);

    return lNode;
}

// Create user defined properties
//
void FbxLoader::CreateUserProperties(FbxNode* pNode, FbxObject* pObj)
{
    // Now we create the user properties 
    FbxProperty custPropBool = FbxProperty::Create(pNode, FbxBoolDT, "ADN_Bool", "ADN Sample Bool Property");
    FbxProperty custPropInt = FbxProperty::Create(pNode, FbxIntDT, "ADN_Integer", "ADN Sample Integer Property");
    FbxProperty custPropStrList = FbxProperty::Create(pNode, FbxStringListDT, "ADN_StringList", "");

    custPropBool.ConnectSrcObject(pObj, FbxConnection::eReference);
    custPropInt.ConnectSrcObject(pObj, FbxConnection::eReference);
    custPropStrList.ConnectSrcObject(pObj, FbxConnection::eReference);

    /*
    NOTE: The properties labels exists only while the property object is in memory.
    The label is not saved in the FBX file. When loading properties from the FBX file
    it will take the same value as the property name.
    */

    // we now fill the properties. All the properties are user properties so we set the
    // correct flag
    custPropBool.ModifyFlag(FbxPropertyFlags::eUserDefined, true);
    custPropInt.ModifyFlag(FbxPropertyFlags::eUserDefined, true);
    custPropStrList.ModifyFlag(FbxPropertyFlags::eUserDefined, true);

    // we set the default values
    custPropBool.Set(false);
    custPropInt.Set(0);

    // and some limits
    custPropInt.SetLimits(0, 10);

    // add elements to the list
    custPropStrList.AddEnumValue("one");
    custPropStrList.AddEnumValue("two");
    custPropStrList.AddEnumValue("three");
    custPropStrList.AddEnumValue("Four");
    custPropStrList.InsertEnumValue(0, "zero");
}

void FbxLoader::PrintLevel(int level, bool bPlus)
{
    // Level indentation
    for (int i = 0; i < level; i++)
    {
        if ((i + 1 == level) && (bPlus))
            FBXSDK_printf("+-");
        else
            FBXSDK_printf("  ");
    }
}

// Iterate the scene, and for each node and 
// object look at the connections
//
void FbxLoader::GetConnections(FbxNode* currentNode, int level)
{

    //	Print out the node name
    //
    PrintLevel(level, true);
    FBXSDK_printf("Node %s \n", currentNode->GetName());

    FbxObject* currentObj = currentNode->GetNodeAttribute();
    if (currentObj)
    {
        int numSrcProps = currentObj->GetSrcPropertyCount();
        for (int j = 0; j < numSrcProps; j++)
        {
            FbxProperty propSrc = currentObj->GetSrcProperty(j);
            PrintLevel(level, false);
            FBXSDK_printf(" Src Prop: %s\n", propSrc.GetNameAsCStr());
        }
        int numDstProps = currentObj->GetDstPropertyCount();
        for (int j = 0; j < numDstProps; j++)
        {
            FbxProperty propDst = currentObj->GetDstProperty(j);
            PrintLevel(level, false);
            FBXSDK_printf(" Dst Prop: %s\n", propDst.GetNameAsCStr());
        }
    }

    //	Get and Print out the Source Objects
    //
    int numSrcObjs = currentNode->GetSrcObjectCount();
    for (int i = 0; i < numSrcObjs; i++)
    {
        FbxObject* objSrc = currentNode->GetSrcObject(i);
        PrintLevel(level, false);
        FBXSDK_printf(" Src Obj: %s\n", objSrc->GetName());
    }

    //	Get and Print out the Destination Objects
    //
    int numDstObjs = currentNode->GetDstObjectCount();
    for (int i = 0; i < numDstObjs; i++)
    {
        FbxObject* objDst = currentNode->GetDstObject(i);
        PrintLevel(level, false);
        FBXSDK_printf(" Dst Obj: %s\n", objDst->GetName());
    }

    // Recurse on the node children
    int numKids = currentNode->GetChildCount();
    for (int k = 0; k < numKids; k++)
    {
        FbxNode* child = currentNode->GetChild(k);
        GetConnections(child, level + 1);
    }
}

FbxNode* FbxLoader::CreateCube()
{
    const int numFaces = 6;
    const int vertsPerFace = 4;

    FbxVector4 points[] = {
        FbxVector4(-50, -50, 50),
        FbxVector4(50, -50, 50),
        FbxVector4(50, 50, 50),
        FbxVector4(-50, 50, 50),
        FbxVector4(-50, -50, -50),
        FbxVector4(50, -50, -50),
        FbxVector4(50, 50, -50),
        FbxVector4(-50, 50, -50) };

    FbxVector4 normals[] = {
        FbxVector4(0, 0, 1),
        FbxVector4(1, 0, 0),
        FbxVector4(0, 0, -1),
        FbxVector4(-1, 0, 0),
        FbxVector4(0, 1, 0),
        FbxVector4(0, -1, 0) };

    int faceVerts[] = { 0, 1, 2, 3,
                     1, 5, 6, 2,
                     5, 4, 7, 6,
                     4, 0, 3, 7,
                     3, 2, 6, 7,
                     4, 5, 1, 0 };

    FbxMesh* myMesh = NULL;
    myMesh = FbxMesh::Create(globalSdkMgr(), "myCube");

    myMesh->InitControlPoints(numFaces * vertsPerFace);
    FbxVector4* cpArray = myMesh->GetControlPoints();

  
    for (int f = 0; f < numFaces; f++)
    {
        myMesh->BeginPolygon();
        for (int v = 0; v < vertsPerFace; v++)
        {
            int vIndex = vertsPerFace * f + v;

            myMesh->AddPolygon(vIndex);
            cpArray[vIndex] = points[faceVerts[vIndex]];
        }
        myMesh->EndPolygon();
    }

  
    FbxLayer* layer0 = myMesh->GetLayer(0);
    if (layer0 == NULL)
    {
        myMesh->CreateLayer();
        layer0 = myMesh->GetLayer(0);
    }

   
    FbxLayerElementNormal* normalLayer = FbxLayerElementNormal::Create(myMesh, "");
    normalLayer->SetMappingMode(FbxLayerElement::eByControlPoint);
    normalLayer->SetReferenceMode(FbxLayerElement::eDirect);
    layer0->SetNormals(normalLayer);

    FbxLayerElementArrayTemplate<FbxVector4>& normalArray = normalLayer->GetDirectArray();
    for (int f = 0; f < numFaces; f++)
    {
        for (int v = 0; v < vertsPerFace; v++)
        {
            normalArray.Add(normals[f]);
        }
    }

    FbxNode* myNode = FbxNode::Create(globalSdkMgr(), "myNode");
    myNode->SetNodeAttribute(myMesh);

    return myNode;
}

void FbxLoader::MaterialCube(FbxNode* pCube, FbxScene* scene)
{
    FbxMesh* mesh = pCube->GetMesh();
    FbxGeometryElementMaterial* mat = mesh->CreateElementMaterial();
    mat->SetMappingMode(FbxLayerElement::eByPolygon);
    mat->SetReferenceMode(FbxGeometryElement::eIndexToDirect);

    int count = mesh->GetPolygonCount();

    mat->GetIndexArray().SetCount(count);
    for (int i = 0; i < count; i++)
    {
        FbxString lMaterialName = "TestMaterial";
        FbxString lShadingModelName = i % 2 == 0 ? "Lambert" : "Phong";
        lMaterialName += i;
        FbxDouble3 lEmissive(0.0, 0.0, 0.0);
        FbxDouble3 lAmbient(1.0, 0.0, 0.0);
        FbxDouble3 lColor;
        FbxSurfaceLambert* lMaterial = FbxSurfaceLambert::Create(scene, lMaterialName.Buffer());

        lMaterial->Emissive.Set(lEmissive);
        lMaterial->Ambient.Set(lAmbient);
        lColor = FbxDouble3(i > 2 ? 1.0 : 0.0,
            i > 0 && i < 4 ? 1.0 : 0.0,
            i % 2 ? 0.0 : 1.0);
        lMaterial->Diffuse.Set(lColor);

        lMaterial->TransparencyFactor.Set(0.0);
        lMaterial->ShadingModel.Set(lShadingModelName);

        mat->GetIndexArray().SetAt(i, i);

        pCube->AddMaterial(lMaterial);

    }
}


void FbxLoader::AnimateCube(FbxNode* pCube, FbxScene* scene)
{
    FbxAnimStack* animStack = FbxAnimStack::Create(scene, "Cube Animation Stack");
    FbxAnimLayer* animLayer = FbxAnimLayer::Create(scene, "Base Layer");
    animStack->AddMember(animLayer);

    FbxTime lTime;
    int lKeyIndex = 0;

    FbxAnimCurve* acurve = pCube->LclTranslation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Z, true);
    if (acurve)
    {
        acurve->KeyModifyBegin();

        lTime.SetSecondDouble(0.0);
        lKeyIndex = acurve->KeyAdd(lTime);
        acurve->KeySet(lKeyIndex, lTime, 0.0, FbxAnimCurveDef::eInterpolationLinear);

        lTime.SetSecondDouble(2.0);
        lKeyIndex = acurve->KeyAdd(lTime);
        acurve->KeySet(lKeyIndex, lTime, 300.0, FbxAnimCurveDef::eInterpolationLinear);

        acurve->KeyModifyEnd();
    }

}

FbxNode* FbxLoader::CreateNurbsSphere(FbxScene* pScene, char* pName)
{
    FbxNurbs* lNurbs = FbxNurbs::Create(pScene, pName);

    lNurbs->SetOrder(4, 4);
    lNurbs->SetStep(2, 2);
    lNurbs->InitControlPoints(8, FbxNurbs::ePeriodic, 7, FbxNurbs::eOpen);

    double lUKnotVector[] = { -3.0, -2.0, -1.0, 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0 };
    memcpy(lNurbs->GetUKnotVector(), lUKnotVector, lNurbs->GetUKnotCount() * sizeof(double));

    double lVKnotVector[] = { 0.0, 0.0, 0.0, 0.0, 1.0, 2.0, 3.0, 4.0, 4.0, 4.0, 4.0 };
    memcpy(lNurbs->GetVKnotVector(), lVKnotVector, lNurbs->GetVKnotCount() * sizeof(double));

    FbxVector4* lVector4 = lNurbs->GetControlPoints();
    int i, j;
    double lScale = 20.0;
    double lPi = 3.14159;
    double lYAngle[] = { 90.0, 90.0, 52.0, 0.0, -52.0, -90.0, -90.0 };
    double lRadius[] = { 0.0, 0.283, 0.872, 1.226, 0.872, 0.283, 0.0 };

    for (i = 0; i < 7; i++)
    {
        for (j = 0; j < 8; j++)
        {
            double lX = lScale * lRadius[i] * cos(lPi / 4 * j);
            double lY = lScale * sin(2 * lPi / 360 * lYAngle[i]);
            double lZ = lScale * lRadius[i] * sin(lPi / 4 * j);
            double lWeight = 1.0;

            lVector4[8 * i + j].Set(lX, lY, lZ, lWeight);
        }
    }


    FbxNode* lNode = FbxNode::Create(pScene, pName);

    lNode->SetNodeAttribute(lNurbs);

    return lNode;
}

void FbxLoader::MapSphereTexture(FbxScene* pScene, FbxNode* pNurbs)
{
    FbxFileTexture* lTexture = FbxFileTexture::Create(pScene, "eye.jpg");

    pNurbs->SetShadingMode(FbxNode::eTextureShading);

    lTexture->SetFileName("eye.jpg");
    lTexture->SetTextureUse(FbxTexture::eStandard);
    lTexture->SetMappingType(FbxTexture::eSpherical);
    lTexture->SetMaterialUse(FbxFileTexture::eModelMaterial);
    lTexture->SetSwapUV(false);
    lTexture->SetTranslation(0.0, 0.0);
    lTexture->SetScale(1.0, 2.0);
    lTexture->SetRotation(45.0, 0.0);

    FbxSurfacePhong* lMaterial = pNurbs->GetSrcObject<FbxSurfacePhong>(0);
    if (lMaterial)
        lMaterial->Diffuse.ConnectSrcObject(lTexture);

}

void FbxLoader::MapSphereMaterial(FbxScene* pScene, FbxNode* pNurbs)
{
    FbxSurfacePhong* lMaterial = FbxSurfacePhong::Create(pScene, "scene02");
    FbxDouble3 lBlue(0.05, 0.05, 0.05);
    FbxDouble3 lBlack(0.0, 0.0, 0.0);

    lMaterial->Emissive.Set(lBlue);
    lMaterial->Ambient.Set(lBlack);
    lMaterial->Specular.Set(lBlack);
    lMaterial->TransparencyFactor.Set(0.0);
    lMaterial->Shininess.Set(0.0);
    lMaterial->ReflectionFactor.Set(0.0);

    FbxNurbs* lNurbs = pNurbs->GetNurbs();
    FbxGeometryElementMaterial* lGeometryElementMaterial = lNurbs->GetElementMaterial(0);

    if (!lGeometryElementMaterial)
    {
        lGeometryElementMaterial = lNurbs->CreateElementMaterial();
    }

    lGeometryElementMaterial->SetMappingMode(FbxGeometryElement::eAllSame);

    lGeometryElementMaterial->SetReferenceMode(FbxGeometryElement::eDirect);
    pNurbs->AddMaterial(lMaterial);
}

void FbxLoader::CreateTextureSphere(FbxScene* scene)
{
    FbxNode* lNurbs = CreateNurbsSphere(scene, "TestSphere");

    MapSphereMaterial(scene, lNurbs);
    MapSphereTexture(scene, lNurbs);

    FbxNode* lRootNode = scene->GetRootNode();
    lRootNode->AddChild(lNurbs);
    lNurbs->LclTranslation.Set(FbxVector4(0.0, 0.0, 200.0));
}

int FbxLoader::findAsciiFormat(FbxManager* mgr)
{
    int fileFormat = mgr->GetIOPluginRegistry()->GetNativeWriterFormat();

    int lFormatIndex, lFormatCount = mgr->GetIOPluginRegistry()->GetWriterFormatCount();

    for (lFormatIndex = 0; lFormatIndex < lFormatCount; lFormatIndex++)
    {
        if (mgr->GetIOPluginRegistry()->WriterIsFBX(lFormatIndex))
        {
            FbxString lDesc = mgr->GetIOPluginRegistry()->GetWriterFormatDescription(lFormatIndex);
            char* lASCII = "ascii";
            if (lDesc.Find(lASCII) >= 0)
            {
                fileFormat = lFormatIndex;
                break;
            }
        }
    }

    return fileFormat;

}

void FbxLoader::ExportFbxScene(FbxManager* mgr, FbxScene* scene, char* pFileName)
{
    FbxIOPluginRegistry* pluginRegistry = mgr->GetIOPluginRegistry();
    assert(pluginRegistry != NULL);

    FbxExporter* exporter = FbxExporter::Create(mgr, "ADN_Exporter");

    FbxIOSettings* ios = FbxIOSettings::Create(mgr, IOSROOT);
    mgr->SetIOSettings(ios);


    int fileFormat = findAsciiFormat(mgr);

    if (exporter->Initialize(pFileName, fileFormat, mgr->GetIOSettings()) == false)
    {
        FBXSDK_printf("Call to FbxExporter::Initialize() failed.\n");
        FBXSDK_printf("Error returned: Error Code UnKnown");
        return;
    }

    if (!exporter->Export(scene))
        FBXSDK_printf("Call to FbxExporter::Export() failed.\n");

    return;
}

void FbxLoader::useFbxLoader()
{
  
    FbxManager* fbxSdkManager = globalSdkMgr();

   
    FbxScene* scene = FbxScene::Create(fbxSdkManager, "ADN_Scene");

    
    FbxNode* root = scene->GetRootNode();

   
    FbxNode* nodeCamera = CreateCamera(scene, FbxVector4(850.0, 700.0, 200.0), FbxVector4(0.0, 0.0, 0.0));
    root->AddChild(nodeCamera);

   
    FbxNode* nodeLight = CreateLight(scene, FbxDouble3(1000, 1000, 1000));
    root->AddChild(nodeLight);

   
    CreateUserProperties(nodeLight, nodeCamera->GetNodeAttribute());

    FbxProperty propFromLightNode = nodeLight->FindProperty("ADN_StringList", false);
    if (propFromLightNode.IsValid())
        FBXSDK_printf("\nFound Light Node Prop: %s", propFromLightNode.GetNameAsCStr());
    
    FbxProperty propFromCamera = nodeCamera->GetNodeAttribute()->FindProperty("ADN_StringList", false);
    if (propFromCamera.IsValid())
        FBXSDK_printf("\nFound Camera Prop: %s", propFromCamera.GetNameAsCStr());

   
    FBXSDK_printf("\n\n\nObject Connections\n");
    GetConnections(root);

   
    FbxNode* nodeCube = CreateCube();
    root->AddChild(nodeCube);

    MaterialCube(nodeCube, scene);

    CreateTextureSphere(scene);

    AnimateCube(nodeCube, scene);

    ExportFbxScene(fbxSdkManager, scene, "output.fbx");

    // globalMgrDestroy();
}
