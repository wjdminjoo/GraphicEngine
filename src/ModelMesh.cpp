
#include "ModelMesh.h"
#include <algorithm>

ModelMesh::ModelMesh() :
	m_mappedConstantBuffer(nullptr),
	m_pScene(nullptr)
{
	ZeroMemory(&m_constantBufferData, sizeof(m_constantBufferData));
}

ModelMesh::~ModelMesh()
{
	m_constantBuffer->Unmap(0, nullptr);
	m_mappedConstantBuffer = nullptr;

	Release();
}

XMFLOAT3X3 ModelMesh::aiMatrix3x3tToXMFloat3x3(const aiMatrix3x3t<float>& aiMe)
{
	XMFLOAT3X3 output;
	output._11 = aiMe.a1;
	output._12 = aiMe.a2;
	output._13 = aiMe.a3;

	output._21 = aiMe.b1;
	output._22 = aiMe.b2;
	output._23 = aiMe.b3;

	output._31 = aiMe.c1;
	output._32 = aiMe.c2;
	output._33 = aiMe.c3;

	return output;
}

XMFLOAT4X4 ModelMesh::aiMatrixToXMFloat4x4(const aiMatrix4x4& aiMe)
{
	XMFLOAT4X4 output;
	output._11 = aiMe.a1;
	output._12 = aiMe.a2;
	output._13 = aiMe.a3;
	output._14 = aiMe.a4;

	output._21 = aiMe.b1;
	output._22 = aiMe.b2;
	output._23 = aiMe.b3;
	output._24 = aiMe.b4;

	output._31 = aiMe.c1;
	output._32 = aiMe.c2;
	output._33 = aiMe.c3;
	output._34 = aiMe.c4;

	output._41 = aiMe.d1;
	output._42 = aiMe.d2;
	output._43 = aiMe.d3;
	output._44 = aiMe.d4;

	return output;
}

void ModelMesh::Release()
{
	// these should all be reclaimed automatically after the clear
	m_entries.clear();
	m_entries.shrink_to_fit();
	m_textures.clear();
	m_textures.shrink_to_fit();
	m_textureIndexedNames.clear();
	m_pScene = nullptr;

	ReleaseUpload();
}

void ModelMesh::ReleaseUpload()
{
	m_vertexBufferUpload = nullptr;
	m_indexBufferUpload = nullptr;
	m_texturesUpload.clear();
	m_texturesUpload.shrink_to_fit();
}

bool ModelMesh::LoadMesh(const std::string& filename, std::vector<XMFLOAT3> positions, std::vector<XMFLOAT3> normals, std::vector<XMFLOAT2> texCoords, std::vector<unsigned int> indices, int numVertices, int numIndices)
{
	
	bool ret = false;

	m_pScene = m_Importer.ReadFile(filename.c_str(), aiProcess_Triangulate
		| aiProcess_GenSmoothNormals
		| aiProcess_FlipUVs
		| aiProcess_JoinIdenticalVertices
		| aiPostProcessSteps::aiProcess_FlipWindingOrder
		//| aiPostProcessSteps::aiProcess_GenNormals
	);

	if (m_pScene)
	{
		m_globalInverseTransform = aiMatrixToXMFloat4x4(m_pScene->mRootNode->mTransformation);
		XMMATRIX globalTransformMatrix = XMLoadFloat4x4(&m_globalInverseTransform);
		globalTransformMatrix = XMMatrixInverse(nullptr, globalTransformMatrix);
		XMStoreFloat4x4(&m_globalInverseTransform, globalTransformMatrix);
		ret = InitFromScene(m_pScene, filename, positions, normals, texCoords, indices, numVertices, numIndices);
	}
	else
	{
		assert(m_Importer.GetErrorString());
	}

	return ret;
}


bool ModelMesh::InitFromScene(const aiScene* pScene, const std::string& filename, std::vector<XMFLOAT3> positions, std::vector<XMFLOAT3> normals, std::vector<XMFLOAT2> texCoords, std::vector<unsigned int> indices, int numVertices, int numIndices)
{
	m_entries.resize(pScene->mNumMeshes);

	numVertices = 0;
	numIndices = 0;

	// Count the number of vertices and indices
	for (size_t i = 0; i < m_entries.size(); i++) {
		m_entries[i].MaterialIndex = pScene->mMeshes[i]->mMaterialIndex;
		m_entries[i].NumIndices = pScene->mMeshes[i]->mNumFaces * 3;
		m_entries[i].BaseVertex = numVertices;
		m_entries[i].BaseIndex = numIndices;

		numVertices += pScene->mMeshes[i]->mNumVertices;
		numIndices += m_entries[i].NumIndices;
	}

	// Reserve space in the vectors for the vertex attributes and indices
	positions.reserve(numVertices);
	normals.reserve(numVertices);
	texCoords.reserve(numVertices);
	indices.reserve(numIndices);

	// Initialize the meshes in the scene one by one
	for (size_t i = 0; i < m_entries.size(); i++) {
		const aiMesh* paiMesh = pScene->mMeshes[i];
		InitMesh(i, paiMesh, positions, normals, texCoords, indices, numVertices, numIndices);
	}
	
	return true;
}


void ModelMesh::InitMesh(int meshIndex,
	const aiMesh* paiMesh,
	std::vector<XMFLOAT3>& positions,
	std::vector<XMFLOAT3>& normals,
	std::vector<XMFLOAT2>& texCoords,
	std::vector<unsigned int>& indices,
	int numVertices,
	int numIndices)
{
	const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

	// Populate the vertex attribute vectors
	for (unsigned int i = 0; i < paiMesh->mNumVertices; i++) {
		const aiVector3D* pPos = &(paiMesh->mVertices[i]);
		const aiVector3D* pNormal = &(paiMesh->mNormals[i]);
		const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ? &(paiMesh->mTextureCoords[0][i]) : &Zero3D;

		positions.push_back(XMFLOAT3(pPos->x, pPos->y, pPos->z));
		normals.push_back(XMFLOAT3(pNormal->x, pNormal->y, pNormal->z));
		texCoords.push_back(XMFLOAT2(pTexCoord->x, pTexCoord->y));
	}

	// Populate the index buffer
	for (unsigned int i = 0; i < paiMesh->mNumFaces; i++) {
		const aiFace& face = paiMesh->mFaces[i];
		assert(face.mNumIndices == 3);
		indices.push_back(face.mIndices[0]);
		indices.push_back(face.mIndices[1]);
		indices.push_back(face.mIndices[2]);
	}
}


void ModelMesh::RenderDirectX(ID3D12GraphicsCommandList* commandList, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuHandle, _Out_ int& offsetInDescriptors)
{
	offsetInDescriptors = m_textures.size();

	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->IASetIndexBuffer(&indexBufferView);

	// for each mesh
	for (size_t i = 0; i < m_entries.size(); i++)
	{
		ModelMesh::MeshEntry meshEntry = m_entries[i];

		// Bind the current frame's texture buffer to the pipeline.
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuMaterialHandle(gpuHandle, meshEntry.MaterialIndex, m_cbvDescriptorSize);
		commandList->SetGraphicsRootDescriptorTable(1, gpuMaterialHandle);
		commandList->DrawIndexedInstanced(meshEntry.NumIndices, 1, meshEntry.BaseIndex, meshEntry.BaseVertex, 0);
	}
}

void ModelMesh::Update(GameTimer const & timer, const int& frameIndex, XMFLOAT4X4 model, XMFLOAT4X4& view, XMFLOAT4X4& projection)
{
	XMMATRIX scale = XMLoadFloat4x4(&model);
	XMStoreFloat4x4(&model, XMMatrixTranspose(scale*XMMatrixScaling(1.0f, 1.0f, 1.0f)));

	m_constantBufferData.model = model;
	m_constantBufferData.view = view;
	m_constantBufferData.projection = projection;

	UINT8* destination = m_mappedConstantBuffer + (frameIndex * c_alignedConstantBufferSize);
	memcpy(destination, &m_constantBufferData, sizeof(m_constantBufferData));
}
