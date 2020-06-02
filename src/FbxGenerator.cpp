//#include "FbxGenerator.h"
//#include "FbxLoader.h"
//
//FBXGenerator::FBXGenerator()
//{
//}
//
//FBXGenerator::~FBXGenerator()
//{
//}
//
//void FBXGenerator::Begin(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, ID3D12DescriptorHeap* cbvHeap)
//{
//	if (mInBeginEndPair)
//		throw std::exception("Cannot nest Begin calls on a FBX Generator");
//
//	mDevice = device;
//	mCommandList = cmdList;
//	mCbvHeap = cbvHeap;
//
//	mInBeginEndPair = true;
//}
//
//void FBXGenerator::End()
//{
//	if (!mInBeginEndPair)
//		throw std::exception("Begin must be called before End");
//
//	mDevice = nullptr;
//	mCommandList = nullptr;
//	mCbvHeap = nullptr;
//
//	mInBeginEndPair = false;
//}
////
////void FBXGenerator::BuildFBXTexture(std::vector<Material>& outMaterial, std::string inTextureName, std::string inMaterialName, Textures& mTextures, Textures& mTexturesNormal, Materials& mMaterials)
////{
////	// Begin
////	mTextures.Begin(mDevice, mCommandList, mCbvHeap);
////	mTexturesNormal.Begin(mDevice, mCommandList, mCbvHeap);
////
////	// Load Texture and Material
////	int MatIndex = mMaterials.GetSize();
////	for (int i = 0; i < outMaterial.size(); ++i)
////	{
////		std::string TextureName;
////		// Load Texture 
////		if (!outMaterial[i].Name.empty())
////		{
////			// Texture
////			TextureName = inTextureName;
////			TextureName.push_back(i + 48);
////			std::wstring TextureFileName;
////			TextureFileName.assign(outMaterial[i].Name.begin(), outMaterial[i].Name.end());
////			mTextures.SetTexture(
////				TextureName,
////				TextureFileName);
////
////			// Normal Map
////			std::wstring TextureNormalFileName;
////			TextureNormalFileName = TextureFileName.substr(0, TextureFileName.size() - 11);
////			TextureNormalFileName.append(L"normal.jpg");
////			struct stat buffer;
////			std::string fileCheck;
////			fileCheck.assign(TextureNormalFileName.begin(), TextureNormalFileName.end());
////			if (stat(fileCheck.c_str(), &buffer) == 0)
////			{
////				mTexturesNormal.SetTexture(
////					TextureName,
////					TextureNormalFileName);
////			}
////		}
////
////		// Load Material
////		std::string MaterialName = inMaterialName;
////		MaterialName.push_back(i + 48);
////
////		mMaterials.SetMaterial(
////			MaterialName,
////			outMaterial[i].DiffuseAlbedo,
////			outMaterial[i].FresnelR0,
////			outMaterial[i].Roughness,
////			MatIndex++,
////			mTextures.GetTextureIndex(TextureName),
////			mTexturesNormal.GetTextureIndex(TextureName));
////	}
////	mTextures.End();
////	mTexturesNormal.End();
////}
//
//void FBXGenerator::LoadFBXArchitecture(std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& mGeometries, Textures& mTextures, Materials& mMaterials)
//{
//	// Architecture FBX
//	FbxLoader fbx;
//	std::vector<Vertex> outVertices;
//	std::vector<std::uint32_t> outIndices;
//	std::vector<Material> outMaterial;
//	std::string FileName;
//
//	std::vector<std::vector<Vertex>> archVertex;
//	std::vector<std::vector<uint32_t>> archIndex;
//	std::vector<std::string> archName;
//
//	FileName = "Resource/Architecture/houseA/house";
//	fbx.LoadFBX(outVertices, outIndices, outMaterial, FileName);
//	archVertex.push_back(outVertices);
//	archIndex.push_back(outIndices);
//	archName.push_back("house");
//
//	outVertices.clear();
//	outIndices.clear();
//	fbx.clear();
//
//	FileName = "Resource/Architecture/Rocks/RockCluster/RockCluster";
//	fbx.LoadFBX(outVertices, outIndices, outMaterial, FileName);
//	archVertex.push_back(outVertices);
//	archIndex.push_back(outIndices);
//	archName.push_back("RockCluster");
//
//	outVertices.clear();
//	outIndices.clear();
//	fbx.clear();
//
//	FileName = "Resource/Architecture/Canyon/Canyon0";
//	fbx.LoadFBX(outVertices, outIndices, outMaterial, FileName);
//	archVertex.push_back(outVertices);
//	archIndex.push_back(outIndices);
//	archName.push_back("Canyon0");
//
//	outVertices.clear();
//	outIndices.clear();
//	fbx.clear();
//
//	FileName = "Resource/Architecture/Canyon/Canyon1";
//	fbx.LoadFBX(outVertices, outIndices, outMaterial, FileName);
//	archVertex.push_back(outVertices);
//	archIndex.push_back(outIndices);
//	archName.push_back("Canyon1");
//
//	outVertices.clear();
//	outIndices.clear();
//	fbx.clear();
//
//	FileName = "Resource/Architecture/Canyon/Canyon2";
//	fbx.LoadFBX(outVertices, outIndices, outMaterial, FileName);
//	archVertex.push_back(outVertices);
//	archIndex.push_back(outIndices);
//	archName.push_back("Canyon2");
//
//	outVertices.clear();
//	outIndices.clear();
//	fbx.clear();
//
//	FileName = "Resource/Architecture/Rocks/Rock/Rock";
//	fbx.LoadFBX(outVertices, outIndices, outMaterial, FileName);
//	archVertex.push_back(outVertices);
//	archIndex.push_back(outIndices);
//	archName.push_back("Rock0");
//
//	outVertices.clear();
//	outIndices.clear();
//	fbx.clear();
//
//	FileName = "Resource/Architecture/Tree/Tree";
//	fbx.LoadFBX(outVertices, outIndices, outMaterial, FileName);
//	archVertex.push_back(outVertices);
//	archIndex.push_back(outIndices);
//	archName.push_back("Tree");
//
//	outVertices.clear();
//	outIndices.clear();
//	fbx.clear();
//
//	FileName = "Resource/FBX/Architecture/Tree/Leaf";
//	fbx.LoadFBX(outVertices, outIndices, outMaterial, FileName);
//	archVertex.push_back(outVertices);
//	archIndex.push_back(outIndices);
//	archName.push_back("Leaf");
//
//	BuildArcheGeometry(archVertex, archIndex, archName, mGeometries);
//	//BuildFBXTexture(outMaterial, "archiTex", "archiMat");
//	//BuildFBXTexture(outMaterial, "archiTex", "archiMat", mTextures, mMaterials);
//
//	outVertices.clear();
//	outIndices.clear();
//	outMaterial.clear();
//}
