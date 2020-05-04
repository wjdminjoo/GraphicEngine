#pragma once

#include <windows.h>
#include <wrl.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include <string>
#include <memory>
#include <algorithm>
#include <vector>
#include <array>
#include <unordered_map>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <cassert>
#include "d3dx12.h"
#include "DDSTextureLoader.h"
#include "MathHelper.h"


// 함수 밖의 전역 범위에 선언, 프로그램 전체에서 유효하고 다른 파일에서도 참조 가능
// 초기화 하지 않을시 0으로 자동 초기화
// 정적 데이터영역에 할당.

extern const int gNumFrameResources;

inline void d3dSetDebugName(IDXGIObject* obj, const char* name) {
	if (obj) {
		obj->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(name), name);
	}
}

inline std::wstring AnsiToWString(const std::string& str) {
	WCHAR buffer[512];
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
	return std::wstring(buffer);
}

class d3dUtil {
public:
	static bool IsKeyDown(int vKeyCode);
	
	static std::string ToString(HRESULT hr);

	// 상수 버퍼는 최소 하드웨어의 배수 여야합니다
	// 할당 크기 (일반적으로 256 바이트). 가장 가까운 수로 반올림
	// 256의 배수. 255를 더한 다음 마스킹하여이 작업을 수행합니다.
	static UINT CalcConstantBufferByteSize(UINT byteSize) {
		return (byteSize + 255) & ~255;
	}

	static Microsoft::WRL::ComPtr<ID3DBlob> LoadBinary(const std::wstring& filename);

	static Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBuffer(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		const void* initData,
		UINT64 byteSize,
		Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer);

	static Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(
		const std::wstring& filename,
		const D3D_SHADER_MACRO* defines,
		const std::string& entrypoint,
		const std::string& target);
};

class DxException
{
public:
	DxException() = default;
	DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber);

	std::wstring ToString()const;

	HRESULT ErrorCode = S_OK;
	std::wstring FunctionName;
	std::wstring Filename;
	int LineNumber = -1;
};

// MeshGeometry에서 형상의 하위 범위를 정의합니다. 이것은 여러 경우입니다
// 지오메트리는 하나의 정점 및 인덱스 버퍼에 저장됩니다. 오프셋을 제공합니다
// 정점 및 인덱스에 형상 저장소의 하위 집합을 그리는 데 필요한 데이터
struct SubmeshGeometry
{
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	INT BaseVertexLocation = 0;


	//이 서브 메쉬에 의해 정의 된 지오메트리의 경계 상자.
	DirectX::BoundingBox Bounds;
};

struct MeshGeometry
{
	// 이름으로 찾아 볼 수 있도록 이름을 지정
	std::string Name;

	// 시스템 메모리 사본. 정점 / 인덱스 형식이 일반적 일 수 있으므로 Blob을 사용
	Microsoft::WRL::ComPtr<ID3DBlob> VertexBufferCPU = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> IndexBufferCPU = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferUploader = nullptr;

	// 버퍼에 대한 데이터
	UINT VertexByteStride = 0;
	UINT VertexBufferByteSize = 0;
	DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
	UINT IndexBufferByteSize = 0;


	// MeshGeometry는 하나의 꼭짓점 / 인덱스 버퍼에 여러 도형을 저장할 수 있습니다.
	// 이 컨테이너를 사용하여 서브 메시 지오메트리를 정의하여
	// 개별적으로 서브 메시.
	std::unordered_map<std::string, SubmeshGeometry> DrawArgs;

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView()const
	{
		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = VertexBufferGPU->GetGPUVirtualAddress();
		vbv.StrideInBytes = VertexByteStride;
		vbv.SizeInBytes = VertexBufferByteSize;

		return vbv;
	}

	D3D12_INDEX_BUFFER_VIEW IndexBufferView()const
	{
		D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();
		ibv.Format = IndexFormat;
		ibv.SizeInBytes = IndexBufferByteSize;

		return ibv;
	}


	// GPU에 업로드를 마치면이 메모리를 비울 수 있습니다.
	void DisposeUploaders()
	{
		VertexBufferUploader = nullptr;
		IndexBufferUploader = nullptr;
	}
};

struct Light
{
	DirectX::XMFLOAT3 Strength = { 0.5f, 0.5f, 0.5f };
	float FalloffStart = 1.0f;                          // point/spot light only
	DirectX::XMFLOAT3 Direction = { 0.0f, -1.0f, 0.0f };// directional/spot light only
	float FalloffEnd = 10.0f;                           // point/spot light only
	DirectX::XMFLOAT3 Position = { 0.0f, 0.0f, 0.0f };  // point/spot light only
	float SpotPower = 64.0f;                            // spot light only
};

#define MaxLights 16

struct MaterialConstants
{
	DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
	float Roughness = 0.25f;

	// 	텍스처 매핑에 사용
	DirectX::XMFLOAT4X4 MatTransform = MathHelper::Identity4x4();
};


//  프로덕션 3D 엔진
// 머티리얼의 클래스 계층을 만들 것입니다
struct Material
{
	// 	찾아 보기위한 고유 한 재료 이름입니다.
	std::string Name;

	// 이 재료에 해당하는 상수 버퍼로 색인하십시오.
	int MatCBIndex = -1;

	// 	확산 텍스처를 위해 SRV 힙으로 색인합니다
	int DiffuseSrvHeapIndex = -1;

	// 정상적인 질감을 위해 SRV 힙으로 색인합니다.
	int NormalSrvHeapIndex = -1;


	// 재질이 변경되었음을 나타내는 Dirty 플래그이며 상수 버퍼를 업데이트해야합니다.
	// 각 FrameResource에 대한 머티리얼 상수 버퍼가 있기 때문에
	// 각 FrameResource로 업데이트합니다. 따라서 머티리얼을 수정할 때 설정해야합니다
	// 각 프레임 리소스가 업데이트를 받도록 NumFramesDirty = gNumFrameResources.
	int NumFramesDirty = gNumFrameResources;


	// 쉐이딩에 사용되는 머티리얼 상수 버퍼 데이터.
	DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
	float Roughness = .25f;
	DirectX::XMFLOAT4X4 MatTransform = MathHelper::Identity4x4();
};

struct Texture
{
	
// 조회를위한 고유 한 재료 이름.
	std::string Name;

	std::wstring Filename;

	Microsoft::WRL::ComPtr<ID3D12Resource> Resource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadHeap = nullptr;
};

#ifndef ThrowIfFailed
#define ThrowIfFailed(x)                                              \
{                                                                     \
    HRESULT hr__ = (x);                                               \
    std::wstring wfn = AnsiToWString(__FILE__);                       \
    if(FAILED(hr__)) { throw DxException(hr__, L#x, wfn, __LINE__); } \
}
#endif

#ifndef ReleaseCom
#define ReleaseCom(x) { if(x){ x->Release(); x = 0; } }
#endif