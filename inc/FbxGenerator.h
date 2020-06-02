#pragma once
#include "d3dUtil.h"
#include "MathHelper.h"
#include "FrameResource.h"
class Textures;
class Materials;
class FBXGenerator
{
public:
	FBXGenerator();
	~FBXGenerator();

	void Begin(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, ID3D12DescriptorHeap* cbvHeap);
	void End();

	void BuildFBXTexture(std::vector<Material>& outMaterial, std::string inTextureName, std::string inMaterialName, Textures& mTextures, Textures& mTexturesNormal, Materials& mMaterials);

	void LoadFBXArchitecture(std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& mGeometries, Textures& mTextures, Materials& mMaterials);

	void BuildArcheGeometry(const std::vector<std::vector<Vertex>>& outVertices, const std::vector<std::vector<std::uint32_t>>& outIndices, const std::vector<std::string>& geoName, std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& mGeometries);

private:
	ID3D12Device* mDevice;
	ID3D12GraphicsCommandList* mCommandList;
	ID3D12DescriptorHeap* mCbvHeap;

	bool mInBeginEndPair;
};
