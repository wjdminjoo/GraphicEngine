#include <fbxsdk.h>
#include <vector>
#include <d3d12.h>
#include <assert.h>
#include <FrameResource.h>

FbxManager* g_pFbxSdkManager = nullptr;

HRESULT LoadFBX(std::vector<Vertex>& pOutVertexVector)
{
    if (g_pFbxSdkManager == nullptr)
    {
        g_pFbxSdkManager = FbxManager::Create();

        FbxIOSettings* pIOsettings = FbxIOSettings::Create(g_pFbxSdkManager, IOSROOT);
        g_pFbxSdkManager->SetIOSettings(pIOsettings);
    }

    FbxImporter* pImporter = FbxImporter::Create(g_pFbxSdkManager, "");
    FbxScene* pFbxScene = FbxScene::Create(g_pFbxSdkManager, "");

    bool bSuccess = pImporter->Initialize("D:\\Engine\\graphicengine\\Resource\\Architecture\\Canyon\\canyon0.fbx", -1, g_pFbxSdkManager->GetIOSettings());
    if (!bSuccess) return E_FAIL;

    bSuccess = pImporter->Import(pFbxScene);
    if (!bSuccess) return E_FAIL;

    pImporter->Destroy();

    FbxNode* pFbxRootNode = pFbxScene->GetRootNode();

    if (pFbxRootNode)
    {
        for (int i = 0; i < pFbxRootNode->GetChildCount(); i++)
        {
            FbxNode* pFbxChildNode = pFbxRootNode->GetChild(i);

            if (pFbxChildNode->GetNodeAttribute() == NULL)
                continue;

            FbxNodeAttribute::EType AttributeType = pFbxChildNode->GetNodeAttribute()->GetAttributeType();

            if (AttributeType != FbxNodeAttribute::eMesh)
                continue;

            FbxMesh* pMesh = (FbxMesh*)pFbxChildNode->GetNodeAttribute();

            FbxVector4* pVertices = pMesh->GetControlPoints();

            for (int j = 0; j < pMesh->GetPolygonCount(); j++)
            {
                int iNumVertices = pMesh->GetPolygonSize(j);
                assert(iNumVertices == 3);

                for (int k = 0; k < iNumVertices; k++) {
                    int iControlPointIndex = pMesh->GetPolygonVertex(j, k);

                    Vertex vertex;
                    vertex.Pos.x = (float)pVertices[iControlPointIndex].mData[0];
                    vertex.Pos.y = (float)pVertices[iControlPointIndex].mData[1];
                    vertex.Pos.z = (float)pVertices[iControlPointIndex].mData[2];
                    pOutVertexVector.push_back(vertex);
                }
            }

        }

    }
    pOutVertexVector.size();
    return S_OK;
}