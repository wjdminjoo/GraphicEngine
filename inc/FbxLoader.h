#include <fbxsdk.h>
#include <vector>
#include <d3d12.h>
#include <assert.h>
#include <DirectXMath.h>

//
//	Routines to create and destroy a single SDK manager for the
//	program
//
struct VertexData
{
    DirectX::XMFLOAT3 pos;		// position
    DirectX::XMFLOAT4 normal;	// normal
    DirectX::XMFLOAT2 uv;		// UV position
};

typedef std::vector<VertexData>	VertexDataArray;
bool LoadFBXConvertToVertexData(const char* filename, VertexDataArray& outVertexData);
