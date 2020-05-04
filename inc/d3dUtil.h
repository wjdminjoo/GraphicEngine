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


// �Լ� ���� ���� ������ ����, ���α׷� ��ü���� ��ȿ�ϰ� �ٸ� ���Ͽ����� ���� ����
// �ʱ�ȭ ���� ������ 0���� �ڵ� �ʱ�ȭ
// ���� �����Ϳ����� �Ҵ�.

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

	// ��� ���۴� �ּ� �ϵ������ ��� �����մϴ�
	// �Ҵ� ũ�� (�Ϲ������� 256 ����Ʈ). ���� ����� ���� �ݿø�
	// 256�� ���. 255�� ���� ���� ����ŷ�Ͽ��� �۾��� �����մϴ�.
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

// MeshGeometry���� ������ ���� ������ �����մϴ�. �̰��� ���� ����Դϴ�
// ������Ʈ���� �ϳ��� ���� �� �ε��� ���ۿ� ����˴ϴ�. �������� �����մϴ�
// ���� �� �ε����� ���� ������� ���� ������ �׸��� �� �ʿ��� ������
struct SubmeshGeometry
{
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	INT BaseVertexLocation = 0;


	//�� ���� �޽��� ���� ���� �� ������Ʈ���� ��� ����.
	DirectX::BoundingBox Bounds;
};

struct MeshGeometry
{
	// �̸����� ã�� �� �� �ֵ��� �̸��� ����
	std::string Name;

	// �ý��� �޸� �纻. ���� / �ε��� ������ �Ϲ��� �� �� �����Ƿ� Blob�� ���
	Microsoft::WRL::ComPtr<ID3DBlob> VertexBufferCPU = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> IndexBufferCPU = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferUploader = nullptr;

	// ���ۿ� ���� ������
	UINT VertexByteStride = 0;
	UINT VertexBufferByteSize = 0;
	DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
	UINT IndexBufferByteSize = 0;


	// MeshGeometry�� �ϳ��� ������ / �ε��� ���ۿ� ���� ������ ������ �� �ֽ��ϴ�.
	// �� �����̳ʸ� ����Ͽ� ���� �޽� ������Ʈ���� �����Ͽ�
	// ���������� ���� �޽�.
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


	// GPU�� ���ε带 ��ġ���� �޸𸮸� ��� �� �ֽ��ϴ�.
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

	// 	�ؽ�ó ���ο� ���
	DirectX::XMFLOAT4X4 MatTransform = MathHelper::Identity4x4();
};


//  ���δ��� 3D ����
// ��Ƽ������ Ŭ���� ������ ���� ���Դϴ�
struct Material
{
	// 	ã�� �������� ���� �� ��� �̸��Դϴ�.
	std::string Name;

	// �� ��ῡ �ش��ϴ� ��� ���۷� �����Ͻʽÿ�.
	int MatCBIndex = -1;

	// 	Ȯ�� �ؽ�ó�� ���� SRV ������ �����մϴ�
	int DiffuseSrvHeapIndex = -1;

	// �������� ������ ���� SRV ������ �����մϴ�.
	int NormalSrvHeapIndex = -1;


	// ������ ����Ǿ����� ��Ÿ���� Dirty �÷����̸� ��� ���۸� ������Ʈ�ؾ��մϴ�.
	// �� FrameResource�� ���� ��Ƽ���� ��� ���۰� �ֱ� ������
	// �� FrameResource�� ������Ʈ�մϴ�. ���� ��Ƽ������ ������ �� �����ؾ��մϴ�
	// �� ������ ���ҽ��� ������Ʈ�� �޵��� NumFramesDirty = gNumFrameResources.
	int NumFramesDirty = gNumFrameResources;


	// ���̵��� ���Ǵ� ��Ƽ���� ��� ���� ������.
	DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
	float Roughness = .25f;
	DirectX::XMFLOAT4X4 MatTransform = MathHelper::Identity4x4();
};

struct Texture
{
	
// ��ȸ������ ���� �� ��� �̸�.
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