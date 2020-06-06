#include <fbxsdk.h>
#include <vector>
#include <d3d12.h>

struct FBXVERTEX {
	float pos[3];
};

class FbxLoader {
public:
	HRESULT LoadFBX(std::vector<FBXVERTEX>* pOutVertexVector);

	
};