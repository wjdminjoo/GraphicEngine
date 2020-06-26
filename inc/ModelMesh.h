#ifndef AAA_HEADER
#define AAA_HEADER

#include <map>
#include <vector>
#include "pch.h"
#include "GameTimer.h"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing fla


using namespace DirectX;

class ModelMesh
{
private:

#pragma region Structures
	// Constant buffer used to send MVP matrices to the vertex shader.
	struct ConstantVSBuffer
	{
		DirectX::XMFLOAT4X4 model;
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
	};

	struct VertexShaderVertices
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
		XMFLOAT3 normal;
	};

#define INVALID_MATERIAL 0xFFFFFFFF
	struct MeshEntry {
		MeshEntry()
		{
			NumIndices = 0;
			BaseVertex = 0;
			BaseIndex = 0;
			MaterialIndex = INVALID_MATERIAL;
		}

		unsigned int NumIndices;
		unsigned int BaseVertex;
		unsigned int BaseIndex;
		unsigned int MaterialIndex;
	};

	static XMFLOAT4X4 aiMatrixToXMFloat4x4(const aiMatrix4x4& aiMe);
	static XMFLOAT3X3 aiMatrix3x3tToXMFloat3x3(const aiMatrix3x3t<float>& aiMe);

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;

	// Vertex Constant Buffer
	static const UINT c_alignedConstantBufferSize = (sizeof(ConstantVSBuffer) + 255) & ~255;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_constantBuffer;
	ConstantVSBuffer m_constantBufferData;
	UINT8*	m_mappedConstantBuffer;

	// upload resources
	Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBufferUpload;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_indexBufferUpload;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_texturesUpload;

	unsigned int m_cbvDescriptorSize;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_indexBuffer;

	const aiScene* m_pScene;
	Assimp::Importer m_Importer;

	std::vector<MeshEntry> m_entries;
	XMFLOAT4X4 m_globalInverseTransform;

	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_textures;
	std::map<unsigned int, std::string> m_textureIndexedNames;

#pragma region Methods,
	bool InitFromScene(
		const aiScene* pScene,
		const std::string& Filename,
		std::vector<XMFLOAT3> positions,
		std::vector<XMFLOAT3> normals,
		std::vector<XMFLOAT2> TexCoords,
		std::vector<unsigned int> indices,
		int numVertices,
		int numIndices
	);
	void InitMesh(int MeshIndex,
		const aiMesh* paiMesh,
		std::vector<XMFLOAT3>& Positions,
		std::vector<XMFLOAT3>& Normals,
		std::vector<XMFLOAT2>& TexCoords,
		std::vector<unsigned int>& Indices,
		int numVertices,
		int numIndices
	);
#pragma endregion

public:
	ModelMesh();
	~ModelMesh();

	bool LoadMesh(const std::string& Filename, std::vector<XMFLOAT3> positions, std::vector<XMFLOAT3> normals, std::vector<XMFLOAT2> texCoords, std::vector<unsigned int> indices, int numVertices, int numIndices);
	void RenderDirectX(ID3D12GraphicsCommandList* commandList, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuHandle, _Out_ int& offsetInDescriptors);
	void Update(GameTimer const& timer, const int& frameIndex, XMFLOAT4X4 model, XMFLOAT4X4& view, XMFLOAT4X4& projection);
	void Release();
	void ReleaseUpload();
};

#endif