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
    DirectX::XMFLOAT3 pos;		// 座標
    DirectX::XMFLOAT4 normal;	// ?線
    DirectX::XMFLOAT2 uv;		// UV座標
};

typedef std::vector<VertexData>	VertexDataArray;
bool LoadFBXConvertToVertexData(const char* filename, VertexDataArray& outVertexData);
