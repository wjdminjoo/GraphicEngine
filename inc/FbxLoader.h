#include <fbxsdk.h>
#include <vector>
#include <d3d12.h>

struct FBXVERTEX {
	float x;
	float y;
	float z;
};


class FbxLoader {
public:
	HRESULT LoadFBX(char* FileName);
	void PrintNode(FbxNode* pNode);
	void PrintTabs();
	FbxString GetAttributeTypeName(FbxNodeAttribute::EType type);
	void PrintAttribute(FbxNodeAttribute* pAttribute);
	void LoadNode(FbxNode* node);
	void ProcessControlPoints(FbxMesh* mesh);
	
	FBXVERTEX ReadNormal(const FbxMesh* mesh, int controlPointIndex, int vertexCounter);
private:
	int numTabs = 0;
};