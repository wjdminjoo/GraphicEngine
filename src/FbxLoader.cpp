//#include <FbxLoader.h>
//
//FbxManager* gFbxManager = nullptr;
//
//FbxLoader::FbxLoader()
//{
//}
//
//FbxLoader::~FbxLoader()
//{
//}
//
//HRESULT FbxLoader::LoadFBX(std::vector<Vertex>& outVertexVector, std::vector<uint32_t>& outIndexVector, std::vector<Material>& outMaterial, std::string fileName)
//{
//	// if exported animation exist
//	if (LoadMesh(fileName, outVertexVector, outIndexVector, &outMaterial)) return S_OK;
//	if (LoadMesh(fileName, outVertexVector, outIndexVector)) return S_OK;
//
//	if (gFbxManager == nullptr)
//	{
//		gFbxManager = FbxManager::Create();
//
//		FbxIOSettings* pIOsettings = FbxIOSettings::Create(gFbxManager, IOSROOT);
//		gFbxManager->SetIOSettings(pIOsettings);
//	}
//
//	FbxImporter* pImporter = FbxImporter::Create(gFbxManager, "");
//	std::string fbxFileName = fileName + ".fbx";
//	bool bSuccess = pImporter->Initialize(fbxFileName.c_str(), -1, gFbxManager->GetIOSettings());
//	if (!bSuccess) return E_FAIL;
//
//	FbxScene* pFbxScene = FbxScene::Create(gFbxManager, "");
//	bSuccess = pImporter->Import(pFbxScene);
//	if (!bSuccess) return E_FAIL;
//
//	pImporter->Destroy();
//
//	FbxAxisSystem sceneAxisSystem = pFbxScene->GetGlobalSettings().GetAxisSystem();
//	FbxAxisSystem::MayaZUp.ConvertScene(pFbxScene); // Delete?
//
//													// Convert quad to triangle
//	FbxGeometryConverter geometryConverter(gFbxManager);
//	geometryConverter.Triangulate(pFbxScene, true);
//
//	// Start to RootNode
//	FbxNode* pFbxRootNode = pFbxScene->GetRootNode();
//	if (pFbxRootNode)
//	{
//		// Bone offset, Control point, Vertex, Index Data
//		int ii = pFbxRootNode->GetChildCount();
//		for (int i = 0; i < pFbxRootNode->GetChildCount(); i++)
//		{
//			FbxNode* pFbxChildNode = pFbxRootNode->GetChild(i);
//			FbxMesh* pMesh = (FbxMesh*)pFbxChildNode->GetNodeAttribute();
//			if (!pMesh) { continue; }
//			FbxNodeAttribute::EType AttributeType = pMesh->GetAttributeType();
//			if (!AttributeType) { continue; }
//
//			if (AttributeType == FbxNodeAttribute::eMesh)
//			{
//				GetControlPoints(pFbxChildNode);
//
//				// Get Vertices and indices info
//				GetVerticesAndIndice(pMesh, outVertexVector, outIndexVector);
//
//				GetMaterials(pFbxChildNode, outMaterial);
//
//				break;
//			}
//		}
//	}
//
//	ExportMesh(outVertexVector, outIndexVector, outMaterial, fileName);
//
//	return S_OK;
//}
//
//bool FbxLoader::LoadMesh(std::string fileName, std::vector<Vertex>& outVertexVector, std::vector<uint32_t>& outIndexVector, std::vector<Material>* outMaterial)
//{
//	fileName = fileName + ".mesh";
//	std::ifstream fileIn(fileName);
//
//	uint32_t vertexSize, indexSize;
//	uint32_t materialSize;
//
//	std::string ignore;
//	if (fileIn)
//	{
//		fileIn >> ignore >> vertexSize;
//		fileIn >> ignore >> indexSize;
//		fileIn >> ignore >> materialSize;
//
//		if (vertexSize == 0 || indexSize == 0)
//			return false;
//
//		// Material Data
//		fileIn >> ignore;
//		for (uint32_t i = 0; i < materialSize; ++i)
//		{
//			Material tempMaterial;
//
//			fileIn >> ignore >> tempMaterial.Name;
//			fileIn >> ignore >> tempMaterial.Ambient.x >> tempMaterial.Ambient.y >> tempMaterial.Ambient.z;
//			fileIn >> ignore >> tempMaterial.DiffuseAlbedo.x >> tempMaterial.DiffuseAlbedo.y >> tempMaterial.DiffuseAlbedo.z >> tempMaterial.DiffuseAlbedo.w;
//			fileIn >> ignore >> tempMaterial.FresnelR0.x >> tempMaterial.FresnelR0.y >> tempMaterial.FresnelR0.z;
//			fileIn >> ignore >> tempMaterial.Specular.x >> tempMaterial.Specular.y >> tempMaterial.Specular.z;
//			fileIn >> ignore >> tempMaterial.Emissive.x >> tempMaterial.Emissive.y >> tempMaterial.Emissive.z;
//			fileIn >> ignore >> tempMaterial.Roughness;
//			fileIn >> ignore;
//			for (int i = 0; i < 4; ++i)
//			{
//				for (int j = 0; j < 4; ++j)
//				{
//					fileIn >> tempMaterial.MatTransform.m[i][j];
//				}
//			}
//			(*outMaterial).push_back(tempMaterial);
//		}
//
//		// Vertex Data
//		for (uint32_t i = 0; i < vertexSize; ++i)
//		{
//			Vertex vertex;
//			fileIn >> ignore >> vertex.Pos.x >> vertex.Pos.y >> vertex.Pos.z;
//			fileIn >> ignore >> vertex.Normal.x >> vertex.Normal.y >> vertex.Normal.z;
//			fileIn >> ignore >> vertex.TexC.x >> vertex.TexC.y;
//
//			// push_back
//			outVertexVector.push_back(vertex);
//		}
//
//		// Index Data
//		fileIn >> ignore;
//		for (uint32_t i = 0; i < indexSize; ++i)
//		{
//			uint32_t index;
//			fileIn >> index;
//			outIndexVector.push_back(index);
//		}
//
//		return true;
//	}
//
//	return false;
//}
//
//void FbxLoader::GetControlPoints(fbxsdk::FbxNode* pFbxRootNode)
//{
//	FbxMesh* pCurrMesh = (FbxMesh*)pFbxRootNode->GetNodeAttribute();
//
//	unsigned int ctrlPointCount = pCurrMesh->GetControlPointsCount();
//	for (unsigned int i = 0; i < ctrlPointCount; ++i)
//	{
//		CtrlPoint* currCtrlPoint = new CtrlPoint();
//
//		DirectX::XMFLOAT3 currPosition;
//		currPosition.x = static_cast<float>(pCurrMesh->GetControlPointAt(i).mData[0]);
//		currPosition.y = static_cast<float>(pCurrMesh->GetControlPointAt(i).mData[1]);
//		currPosition.z = static_cast<float>(pCurrMesh->GetControlPointAt(i).mData[2]);
//
//		currCtrlPoint->mPosition = currPosition;
//		mControlPoints[i] = currCtrlPoint;
//	}
//}
//
//void FbxLoader::GetVerticesAndIndice(fbxsdk::FbxMesh* pMesh, std::vector<Vertex>& outVertexVector, std::vector<uint32_t>& outIndexVector)
//{
//}
//
//void FbxLoader::ExportMesh(std::vector<Vertex>& outVertexVector, std::vector<uint32_t>& outIndexVector, std::vector<Material>& outMaterial, std::string fileName)
//{
//	std::ofstream fileOut(fileName + ".mesh");
//
//	if (outVertexVector.empty() || outIndexVector.empty())
//		return;
//
//	if (fileOut)
//	{
//		uint32_t vertexSize = outVertexVector.size();
//		uint32_t indexSize = outIndexVector.size();
//		uint32_t materialSize = outMaterial.size();
//
//		fileOut << "VertexSize " << vertexSize << "\n";
//		fileOut << "IndexSize " << indexSize << "\n";
//		fileOut << "MaterialSize " << materialSize << "\n";
//
//		fileOut << "Material " << "\n";
//		for (auto& e : outMaterial)
//		{
//			fileOut << "Name " << e.Name << "\n";
//			fileOut << "Ambient " << e.Ambient.x << " " << e.Ambient.y << " " << e.Ambient.z << "\n";
//			fileOut << "Diffuse " << e.DiffuseAlbedo.x << " " << e.DiffuseAlbedo.y << " " << e.DiffuseAlbedo.z << " " << e.DiffuseAlbedo.w << "\n";
//			fileOut << "Fresnel " << e.FresnelR0.x << " " << e.FresnelR0.y << " " << e.FresnelR0.z << "\n";
//			fileOut << "Specular " << e.Specular.x << " " << e.Specular.y << " " << e.Specular.z << "\n";
//			fileOut << "Emissive " << e.Emissive.x << " " << e.Emissive.y << " " << e.Emissive.z << "\n";
//			fileOut << "Roughness " << e.Roughness << "\n";
//			fileOut << "MatTransform ";
//			for (int i = 0; i < 4; ++i)
//			{
//				for (int j = 0; j < 4; ++j)
//				{
//					fileOut << e.MatTransform.m[i][j] << " ";
//				}
//			}
//			fileOut << "\n";
//		}
//
//		for (auto& e : outVertexVector)
//		{
//			fileOut << "Pos " << e.Pos.x << " " << e.Pos.y << " " << e.Pos.z << "\n";
//			fileOut << "Normal " << e.Normal.x << " " << e.Normal.y << " " << e.Normal.z << "\n";
//			fileOut << "TexC " << e.TexC.x << " " << e.TexC.y << "\n";
//		}
//
//		fileOut << "Indices " << "\n";
//		for (uint32_t i = 0; i < indexSize / 3; ++i)
//		{
//			fileOut << outIndexVector[3 * i] << " " << outIndexVector[3 * i + 1] << " " << outIndexVector[3 * i + 2] << "\n";
//		}
//	}
//}
//
//void FbxLoader::GetMaterials(fbxsdk::FbxNode* pNode, std::vector<Material>& outMaterial)
//{
//	int MaterialCount = pNode->GetMaterialCount();
//
//	for (int i = 0; i < MaterialCount; ++i)
//	{
//		Material tempMaterial;
//		FbxSurfaceMaterial* SurfaceMaterial = pNode->GetMaterial(i);
//		GetMaterialAttribute(SurfaceMaterial, tempMaterial);
//		GetMaterialTexture(SurfaceMaterial, tempMaterial);
//
//		if (tempMaterial.Name != "")
//		{
//			outMaterial.push_back(tempMaterial);
//		}
//	}
//}
//
//void FbxLoader::GetMaterialAttribute(fbxsdk::FbxSurfaceMaterial* pMaterial, Material& outMaterial)
//{
//	FbxDouble3 double3;
//	FbxDouble double1;
//	if (pMaterial->GetClassId().Is(FbxSurfacePhong::ClassId))
//	{
//		// Amibent Color
//		double3 = reinterpret_cast<FbxSurfacePhong*>(pMaterial)->Ambient;
//		outMaterial.Ambient.x = static_cast<float>(double3.mData[0]);
//		outMaterial.Ambient.y = static_cast<float>(double3.mData[1]);
//		outMaterial.Ambient.z = static_cast<float>(double3.mData[2]);
//
//		// Diffuse Color
//		double3 = reinterpret_cast<FbxSurfacePhong*>(pMaterial)->Diffuse;
//		outMaterial.DiffuseAlbedo.x = static_cast<float>(double3.mData[0]);
//		outMaterial.DiffuseAlbedo.y = static_cast<float>(double3.mData[1]);
//		outMaterial.DiffuseAlbedo.z = static_cast<float>(double3.mData[2]);
//
//		// Roughness 
//		double1 = reinterpret_cast<FbxSurfacePhong*>(pMaterial)->Shininess;
//		outMaterial.Roughness = 1 - double1;
//
//		// Reflection
//		double3 = reinterpret_cast<FbxSurfacePhong*>(pMaterial)->Reflection;
//		outMaterial.FresnelR0.x = static_cast<float>(double3.mData[0]);
//		outMaterial.FresnelR0.y = static_cast<float>(double3.mData[1]);
//		outMaterial.FresnelR0.z = static_cast<float>(double3.mData[2]);
//
//		// Specular Color
//		double3 = reinterpret_cast<FbxSurfacePhong*>(pMaterial)->Specular;
//		outMaterial.Specular.x = static_cast<float>(double3.mData[0]);
//		outMaterial.Specular.y = static_cast<float>(double3.mData[1]);
//		outMaterial.Specular.z = static_cast<float>(double3.mData[2]);
//
//		// Emissive Color
//		double3 = reinterpret_cast<FbxSurfacePhong*>(pMaterial)->Emissive;
//		outMaterial.Emissive.x = static_cast<float>(double3.mData[0]);
//		outMaterial.Emissive.y = static_cast<float>(double3.mData[1]);
//		outMaterial.Emissive.z = static_cast<float>(double3.mData[2]);
//
//		
//	}
//	else if (pMaterial->GetClassId().Is(FbxSurfaceLambert::ClassId))
//	{
//		// Amibent Color
//		double3 = reinterpret_cast<FbxSurfaceLambert*>(pMaterial)->Ambient;
//		outMaterial.Ambient.x = static_cast<float>(double3.mData[0]);
//		outMaterial.Ambient.y = static_cast<float>(double3.mData[1]);
//		outMaterial.Ambient.z = static_cast<float>(double3.mData[2]);
//
//		// Diffuse Color
//		double3 = reinterpret_cast<FbxSurfaceLambert*>(pMaterial)->Diffuse;
//		outMaterial.DiffuseAlbedo.x = static_cast<float>(double3.mData[0]);
//		outMaterial.DiffuseAlbedo.y = static_cast<float>(double3.mData[1]);
//		outMaterial.DiffuseAlbedo.z = static_cast<float>(double3.mData[2]);
//
//		// Emissive Color
//		double3 = reinterpret_cast<FbxSurfaceLambert*>(pMaterial)->Emissive;
//		outMaterial.Emissive.x = static_cast<float>(double3.mData[0]);
//		outMaterial.Emissive.y = static_cast<float>(double3.mData[1]);
//		outMaterial.Emissive.z = static_cast<float>(double3.mData[2]);
//	}
//}
//
//void FbxLoader::clear()
//{
//	mControlPoints.clear();
//}