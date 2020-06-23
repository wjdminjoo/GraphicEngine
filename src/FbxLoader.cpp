#include "FbxLoader.h"
#include <assert.h>
#include <cstdio>
#include <Textures.h>
static void CreateTexture(FbxScene* pScene, FbxMesh* pMesh);
static void GetFBXVertexData(FbxMesh* pMesh, VertexDataArray& outVertexData);
static void GetMatrialData(FbxSurfaceMaterial* mat);
static void GetMeshData(FbxNode* child, VertexDataArray& outVertexData);

bool LoadFBXConvertToVertexData(const char* filename, VertexDataArray& outVertexData)
{
	FbxManager* pFBXManager = FbxManager::Create();

	FbxScene* pScene = FbxScene::Create(pFBXManager, "");

	FbxIOSettings* pIO = FbxIOSettings::Create(pFBXManager, IOSROOT);
	pFBXManager->SetIOSettings(pIO);

	FbxImporter* pImporter = FbxImporter::Create(pFBXManager, "");

	if (pImporter->Initialize(filename, -1, pFBXManager->GetIOSettings()) == false)
	{
		return false;
	}

	if (pImporter->Import(pScene) == false)
	{
		return false;
	}

	pImporter->Destroy();

	FbxGeometryConverter geometryConverte(pFBXManager);
	geometryConverte.Triangulate(pScene, true);

	GetMeshData(pScene->GetRootNode(), outVertexData);

	pIO->Destroy();
	pScene->Destroy();
	pFBXManager->Destroy();

	getchar();

	return true;
}

void GetMeshData(FbxNode* parent, VertexDataArray& outVertexData)
{
	int numKids = parent->GetChildCount();
	for (int i = 0; i < numKids; i++)
	{
		FbxNode* child = parent->GetChild(i);
		if (child->GetMesh())
		{
			FbxMesh* pMesh = child->GetMesh();
			printf("Mesh\n");

			printf("Mesh Name:%s\n", pMesh->GetName());
			printf("Mesh Polygon Count:%d\n", pMesh->GetPolygonCount());
			printf("Mesh MaterialCount:%d\n", pMesh->GetElementMaterialCount());

			printf("Mesh COntrolPointsCount:%d\n", pMesh->GetControlPointsCount());
			printf("Mesh Texture UVCount:%d\n", pMesh->GetTextureUVCount());

			FbxArray<FbxVector4> normals;
			pMesh->GetPolygonVertexNormals(normals);
			printf("Mesh Normals Count:%d\n", normals.GetCount());

			GetFBXVertexData(pMesh, outVertexData);

			//auto eyeTex = FbxSurfaceMaterial::Create(pMesh, "eye.jpg");
			//child->AddMaterial(eyeTex);


		}



		int numMat = child->GetMaterialCount();

		for (int j = 0; j < numMat; ++j)
		{
			FbxSurfaceMaterial* mat = child->GetMaterial(j);
			if (mat)
			{
				GetMatrialData(mat);
			}
		}

		if (numMat == 0)
		{
			printf("Material None\n");
		}

		child->GetChild(0);

		GetMeshData(child, outVertexData);
	}
}

void GetFBXVertexData(FbxMesh* pMesh, VertexDataArray& outVertexData)
{
	std::vector<FbxVector4> positions, normals;
	FbxVector4 normal;

	for (int i = 0; i < pMesh->GetPolygonCount(); i++)
	{
		for (int j = 0; j < pMesh->GetPolygonSize(i); j++)
		{
			positions.push_back(pMesh->GetControlPointAt(pMesh->GetPolygonVertex(i, j)));

			pMesh->GetPolygonVertexNormal(i, j, normal);
			normals.push_back(normal);
		}
	}

	outVertexData.resize(positions.size());

	for (size_t i = 0; i < outVertexData.size(); ++i)
	{
		fbxsdk::FbxVector4& pos = positions[i];
		fbxsdk::FbxVector4& normal = normals[i];

		outVertexData[i].pos = DirectX::XMFLOAT3(pos.mData[0], pos.mData[1], pos.mData[2]);
		outVertexData[i].normal = DirectX::XMFLOAT4(normal.mData[0], normal.mData[1], normal.mData[2], normal.mData[3]);
	}

	FbxStringList uvSetNames;
	pMesh->GetUVSetNames(uvSetNames);

	bool unmapped = false;
	int UVCount = 0;

	for (int i = 0; i < uvSetNames.GetCount(); ++i)
	{
		for (int j = 0; j < pMesh->GetPolygonCount(); ++j)
		{
			for (int k = 0; k < pMesh->GetPolygonSize(j); ++k)
			{
				FbxVector2 UV;
				pMesh->GetPolygonVertexUV(j, k, uvSetNames.GetStringAt(i), UV, unmapped);

				if (outVertexData.size() > UVCount)
				{
					outVertexData[UVCount].uv = DirectX::XMFLOAT2(UV.mData[0], UV.mData[1]);
				}
				UVCount++;
			}
		}
	}
}

FbxDouble3 GetMaterialProperty(
	const FbxSurfaceMaterial* pMaterial,
	const char* pPropertyName,
	const char* pFactorPropertyName)
{
	FbxDouble3 lResult(0, 0, 0);
	const FbxProperty lProperty = pMaterial->FindProperty(pPropertyName);
	const FbxProperty lFactorProperty = pMaterial->FindProperty(pFactorPropertyName);
	if (lProperty.IsValid() && lFactorProperty.IsValid())
	{
		lResult = lProperty.Get<FbxDouble3>();
		double lFactor = lFactorProperty.Get<FbxDouble>();
		if (lFactor != 1)
		{
			lResult[0] *= lFactor;
			lResult[1] *= lFactor;
			lResult[2] *= lFactor;
		}
	}

	if (lProperty.IsValid())
	{
		printf("Texture\n");
		const int lTextureCount = lProperty.GetSrcObjectCount<FbxFileTexture>();
		for (int i = 0; i < lTextureCount; i++)
		{
			FbxFileTexture* lFileTexture = lProperty.GetSrcObject<FbxFileTexture>(i);
			if (lFileTexture)
			{
				FbxString uvsetName = lFileTexture->UVSet.Get();
				std::string uvSetString = uvsetName.Buffer();
				std::string filepath = lFileTexture->GetFileName();

				printf("UVSet Name=%s\n", uvSetString.c_str());
				printf("Texture Name=%s\n", filepath.c_str());
			}
		}
		puts("");

		printf("Layered Texture\n");
		const int lLayeredTextureCount = lProperty.GetSrcObjectCount<FbxLayeredTexture>();
		for (int i = 0; i < lLayeredTextureCount; i++)
		{
			FbxLayeredTexture* lLayeredTexture = lProperty.GetSrcObject<FbxLayeredTexture>(i);

			const int lTextureFileCount = lLayeredTexture->GetSrcObjectCount<FbxFileTexture>();

			for (int j = 0; j < lTextureFileCount; j++)
			{
				FbxFileTexture* lFileTexture = lLayeredTexture->GetSrcObject<FbxFileTexture>(j);
				if (lFileTexture)
				{
					FbxString uvsetName = lFileTexture->UVSet.Get();
					std::string uvSetString = uvsetName.Buffer();
					std::string filepath = lFileTexture->GetFileName();

					printf("UVSet Name=%s\n", uvSetString.c_str());
					printf("Texture Name=%s\n", filepath.c_str());
				}
			}
		}
		puts("");
	}

	return lResult;
}

void GetMatrialData(FbxSurfaceMaterial* mat)
{
	if (mat == nullptr)
	{
		return;
	}

	puts("");

	if (mat->GetClassId().Is(FbxSurfaceLambert::ClassId))
	{
		printf("Lambert ClassId \n");
	}
	else if (mat->GetClassId().Is(FbxSurfacePhong::ClassId))
	{
		printf("Phong ClassId\n");
	}

	const FbxDouble3 lEmissive = GetMaterialProperty(mat, FbxSurfaceMaterial::sEmissive, FbxSurfaceMaterial::sEmissiveFactor);
	printf("lEmissive:r = %f, g = %f, b = %f\n", lEmissive.mData[0], lEmissive.mData[1], lEmissive.mData[2]);

	const FbxDouble3 lAmbient = GetMaterialProperty(mat, FbxSurfaceMaterial::sAmbient, FbxSurfaceMaterial::sAmbientFactor);
	printf("lAmbient:r = %f, g = %f, b = %f\n", lAmbient.mData[0], lAmbient.mData[1], lAmbient.mData[2]);

	const FbxDouble3 lDiffuse = GetMaterialProperty(mat, FbxSurfaceMaterial::sDiffuse, FbxSurfaceMaterial::sDiffuseFactor);
	printf("lDiffuse:r = %f, g = %f, b = %f\n", lDiffuse.mData[0], lDiffuse.mData[1], lDiffuse.mData[2]);

	const FbxDouble3 lSpecular = GetMaterialProperty(mat, FbxSurfaceMaterial::sSpecular, FbxSurfaceMaterial::sSpecularFactor);
	printf("lSpecular:r = %f, g = %f, b = %f\n", lSpecular.mData[0], lSpecular.mData[1], lSpecular.mData[2]);

	FbxProperty lTransparencyFactorProperty = mat->FindProperty(FbxSurfaceMaterial::sTransparencyFactor);
	if (lTransparencyFactorProperty.IsValid())
	{
		double lTransparencyFactor = lTransparencyFactorProperty.Get<FbxDouble>();
		printf("Transparency Factor = %lf\n", lTransparencyFactor);
	}

	FbxProperty lShininessProperty = mat->FindProperty(FbxSurfaceMaterial::sShininess);
	if (lShininessProperty.IsValid())
	{
		double lShininess = lShininessProperty.Get<FbxDouble>();
		printf("Shininess = %lf\n", lShininess);
	}
}


void CreateTexture(FbxScene* pScene, FbxMesh* pMesh)
{
	FbxSurfacePhong* lMaterial = NULL;

	FbxNode* lNode = pMesh->GetNode();
	if (lNode)
	{
		lMaterial = lNode->GetSrcObject<FbxSurfacePhong>(0);
		if (lMaterial == NULL)
		{
			FbxString lMaterialName = "EyeMaterial";
			FbxString lShadingName = "EyePhong";
			FbxDouble3 lBlack(0.0, 0.0, 0.0);
			FbxDouble3 lRed(1.0, 0.0, 0.0);
			FbxDouble3 lDiffuseColor(0.75, 0.75, 0.0);
			lMaterial = FbxSurfacePhong::Create(pScene, lMaterialName.Buffer());

			lMaterial->Emissive.Set(lBlack);
			lMaterial->Ambient.Set(lRed);
			lMaterial->AmbientFactor.Set(1.);
			lMaterial->Diffuse.Set(lDiffuseColor);
			lMaterial->DiffuseFactor.Set(1.);
			lMaterial->TransparencyFactor.Set(0.4);
			lMaterial->ShadingModel.Set(lShadingName);
			lMaterial->Shininess.Set(0.5);
			lMaterial->Specular.Set(lBlack);
			lMaterial->SpecularFactor.Set(0.3);

			lNode->AddMaterial(lMaterial);
		}
	}

	FbxFileTexture* lTexture = FbxFileTexture::Create(pScene, "Diffuse Texture");

	lTexture->SetFileName("eye.jpg");
	lTexture->SetTextureUse(FbxTexture::eStandard);
	lTexture->SetMappingType(FbxTexture::eUV);
	lTexture->SetMaterialUse(FbxFileTexture::eModelMaterial);
	lTexture->SetSwapUV(false);
	lTexture->SetTranslation(0.0, 0.0);
	lTexture->SetScale(1.0, 1.0);
	lTexture->SetRotation(0.0, 0.0);

	if (lMaterial)
		lMaterial->Diffuse.ConnectSrcObject(lTexture);

	lTexture = FbxFileTexture::Create(pScene, "Ambient Texture");

	lTexture->SetFileName("eye.jpg");
	lTexture->SetTextureUse(FbxTexture::eStandard);
	lTexture->SetMappingType(FbxTexture::eUV);
	lTexture->SetMaterialUse(FbxFileTexture::eModelMaterial);
	lTexture->SetSwapUV(false);
	lTexture->SetTranslation(0.0, 0.0);
	lTexture->SetScale(1.0, 1.0);
	lTexture->SetRotation(0.0, 0.0);

	if (lMaterial)
		lMaterial->Ambient.ConnectSrcObject(lTexture);

	lTexture = FbxFileTexture::Create(pScene, "Emissive Texture");

	lTexture->SetFileName("eye.jpg");
	lTexture->SetTextureUse(FbxTexture::eStandard);
	lTexture->SetMappingType(FbxTexture::eUV);
	lTexture->SetMaterialUse(FbxFileTexture::eModelMaterial);
	lTexture->SetSwapUV(false);
	lTexture->SetTranslation(0.0, 0.0);
	lTexture->SetScale(1.0, 1.0);
	lTexture->SetRotation(0.0, 0.0);

	if (lMaterial)
		lMaterial->Emissive.ConnectSrcObject(lTexture);
}
