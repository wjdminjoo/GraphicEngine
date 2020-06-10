#include <fbxsdk.h>
#include <vector>
#include <d3d12.h>
#include <assert.h>

//
//	Routines to create and destroy a single SDK manager for the
//	program
//

class FbxLoader {
public:
    FbxLoader(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ID3D12DescriptorHeap* cbvHeap) 
        :mInBeginEndPair(false), mDevice(device), mCommandList(commandList), mCbvHeap(cbvHeap) {}

    FbxLoader() :mInBeginEndPair(false){}
    ~FbxLoader() { }

    FbxManager* globalSdkMgr();
    void Begin(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, ID3D12DescriptorHeap* cbvHeap);
    void End();
    
    void globalMgrDestroy();
    FbxNode* CreateCamera(FbxScene* pScene, FbxVector4 vecFrom, FbxVector4 vecTo);
    FbxNode* CreateLight(FbxScene* pScene, FbxDouble3 position);
    void CreateUserProperties(FbxNode* pNode, FbxObject* pObj);
    void PrintLevel(int level, bool bPlus = false);
    void GetConnections(FbxNode* currentNode, int level = 0);
    FbxNode* CreateCube();
    void MaterialCube(FbxNode* pCube, FbxScene* scene);
    void AnimateCube(FbxNode* pCube, FbxScene* scene);
    FbxNode* CreateNurbsSphere(FbxScene* pScene, char* pName);
    void MapSphereTexture(FbxScene* pScene, FbxNode* pNurbs);
    void MapSphereMaterial(FbxScene* pScene, FbxNode* pNurbs);
    void CreateTextureSphere(FbxScene* scene);
    int findAsciiFormat(FbxManager* mgr);
    void ExportFbxScene(FbxManager* mgr, FbxScene* scene, char* pFileName);
    void useFbxLoader();

private:
    ID3D12Device* mDevice;
    ID3D12GraphicsCommandList* mCommandList;
    ID3D12DescriptorHeap* mCbvHeap;

    bool mInBeginEndPair;
};