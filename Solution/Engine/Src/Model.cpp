#include "pch.h"
#include "Model.h"

#include "AssetLibrary.h"

#include "Shlwapi.h"
#include <assimp/scene.h> 
#include <assimp/postprocess.h>
#include <assimp/cimport.h>        // Plain-C interface

namespace Rath
{
	HRESULT Model::Create(ID3D11Device* pd3dDevice, LPCSTR pFile, const void* data)
	{
		HRESULT hr = S_OK;
		CHAR buffer[_MAX_PATH];
		sprintf_s(buffer, _MAX_PATH, "%s%s", MODELFOLDER, pFile);

		LPCSTR pExtension = PathFindExtensionA(pFile);
		if (strcmp(pExtension, ".rath") == 0)
		{
			std::ifstream in(buffer, std::ios::binary);
			ExtractMeshs(pd3dDevice, in);
			in.close();
		}
		else
		{
			const aiScene* pScene = aiImportFile(buffer,
				//aiProcess_CalcTangentSpace |
				aiProcess_Triangulate |
				aiProcess_ConvertToLeftHanded);
			if (pScene)
			{
				ExtractMeshs(pd3dDevice, pScene);
			}
			else
			{
				hr = D3D11_ERROR_FILE_NOT_FOUND;
			}
		}
		return hr;
	}

	Model::~Model()
	{
		for (auto it : mppMaterialList)
			SAFE_RELEASE(it);
	}

	void Model::Render(ID3D11DeviceContext* pd3dImmediateContext)
	{
		int lastMaterialIndex = -1;
		for (auto it : mppMeshList)
		{
			if (it->getMaterialIndex() != lastMaterialIndex)
			{
				lastMaterialIndex = it->getMaterialIndex();
				if (lastMaterialIndex >= 0 && mppMaterialList.size() > 0)
					mppMaterialList[lastMaterialIndex]->Apply(pd3dImmediateContext);
			}
			it->Render(pd3dImmediateContext);
		}
	}

	void Model::ExtractMeshs(ID3D11Device* pd3dDevice, const aiScene* pScene)
	{
		if (pScene && pd3dDevice)
		{
			UINT meshcount = pScene->mNumMeshes;
			UINT materialcount = pScene->mNumMaterials;

			mppMeshList.resize(meshcount);
			for (size_t index = 0; index < meshcount; index++)
			{
				aiMesh* pMesh = pScene->mMeshes[index];
				UINT vertexcount = pMesh->mNumVertices;
				UINT facecount = pMesh->mNumFaces;
				UINT indexcount = facecount * 3;
				UINT material = pMesh->mMaterialIndex;
				UINT stride = 0;
				UINT vertexsize = 0;
				void* vertexbuffer = nullptr;
				UINT indexsize = 0;
				UINT* indexbuffer = nullptr;

				std::vector<TMesh> VertexBuffer(vertexcount);
				for (UINT i = 0; i < vertexcount; i++) {
					TMesh m;
					memcpy(&m.Position, &pMesh->mVertices[i], sizeof(XMFLOAT3));
					memcpy(&m.Normal, &pMesh->mNormals[i], sizeof(XMFLOAT3));
					memcpy(&m.Tex, &pMesh->mTextureCoords[0][i], sizeof(XMFLOAT2));
					VertexBuffer[i] = m;
				}

				XMVECTOR min = g_XMFltMax;
				XMVECTOR max = g_XMFltMin;
				for (auto it : VertexBuffer)
				{
					XMVECTOR pos = XMLoadFloat3(&it.Position);
					min = XMVectorMin(min, pos);
					max = XMVectorMax(max, pos);
				}
				XMVECTOR center = (min + max) / 2.0f;
				XMVECTOR extents = max - center;
				XMStoreFloat3(&mBoundingBox.Center, center);
				XMStoreFloat3(&mBoundingBox.Extents, extents);

				stride = sizeof(TMesh);
				vertexsize = vertexcount * stride;

				std::vector<UINT> IndexBuffer(indexcount);
				for (UINT i = 0, n = 0; i < facecount; i++) {
					for (UINT j = 0; j < 3; j++, n++)
						IndexBuffer[n] = pMesh->mFaces[i].mIndices[j];
				}
				indexsize = (UINT)IndexBuffer.size() * sizeof(UINT);

				mppMeshList[index] = new Mesh(pd3dDevice, stride, vertexsize, &VertexBuffer[0], indexsize, &IndexBuffer[0], material);
			}

			/* --- Create Materials --- */
			mppMaterialList.reserve(materialcount);
			for (size_t i = 0; i < materialcount; i++)
			{
				for (UINT j = 0; j < 1/*pScene->mMaterials[i]->GetTextureCount(aiTextureType_DIFFUSE)*/; j++)
				{
					aiString path;
					pScene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, j, &path);
					mppMaterialList.push_back(AssetLibrary::GetTexture(path.C_Str()));
				}
			}
			/* --- Create Materials --- */

			aiReleaseImport(pScene);
		}
	}

	void Model::ExtractMeshs(ID3D11Device* pd3dDevice, std::istream& pFile)
	{
		UINT meshcount = 0;
		UINT materialcount = 0;
		char animated = 0;

		pFile.read((char*)&meshcount, sizeof(meshcount));
		pFile.read((char*)&materialcount, sizeof(materialcount));
		pFile.read((char*)&mBoundingBox.Center, sizeof(mBoundingBox.Center));
		pFile.read((char*)&mBoundingBox.Extents, sizeof(mBoundingBox.Extents));
		pFile.read((char*)&animated, sizeof(animated));

		mppMeshList.resize(meshcount);
		for (size_t i = 0; i < meshcount; i++)
		{
			UINT vertexsize = 0;
			UINT indexsize = 0;
			UINT stride = 0;
			UINT material = 0;

			// Header
			pFile.read((char*)&vertexsize, sizeof(vertexsize));
			pFile.read((char*)&indexsize, sizeof(indexsize));
			pFile.read((char*)&stride, sizeof(stride));
			pFile.read((char*)&material, sizeof(material));

			void* vertexbuffer = malloc(vertexsize);
			Rath::read_compressed(pFile, (char*)vertexbuffer, vertexsize);

			UINT* indexbuffer = (UINT*)malloc(indexsize);
			Rath::read_compressed(pFile, (char*)indexbuffer, indexsize);

			mppMeshList[i] = new Mesh(pd3dDevice, stride, vertexsize, vertexbuffer, indexsize, indexbuffer, material);

			free(vertexbuffer);
			free(indexbuffer);
		}

		for (size_t i = 0; i < materialcount; i++)
		{
			CHAR buffer[_MAX_PATH];
			size_t length;
			pFile.read((char*)&length, sizeof(length));
			pFile.read((char*)buffer, length);
			buffer[length] = 0;
			mppMaterialList.push_back(AssetLibrary::GetTexture(buffer));
		}
	}

	void AnimatedModel::ExtractMeshs(ID3D11Device* pd3dDevice, const aiScene* pScene)
	{
		if (pScene)
		{
			/* --- Sort Bones --- */
			std::vector<aiBone*> BoneOrderList;
			for (size_t i = 0; i < pScene->mNumMeshes; i++)
			{
				aiMesh* pMesh = pScene->mMeshes[i];
				if (pMesh->HasBones()) for (size_t j = 0; j < pMesh->mNumBones; j++)
				{
					aiBone* pBone = pMesh->mBones[j];
					auto it = std::find(BoneOrderList.begin(), BoneOrderList.end(), pBone);
					if (it == BoneOrderList.end())
					{
						BoneOrderList.push_back(pBone);
					}
				}
			}
			/* --- Sort Bones --- */

			UINT meshcount = pScene->mNumMeshes;
			UINT materialcount = pScene->mNumMaterials;

			mppMeshList.resize(meshcount);
			for (size_t index = 0; index < meshcount; index++)
			{
				aiMesh* pMesh = pScene->mMeshes[index];
				UINT vertexcount = pMesh->mNumVertices;
				UINT facecount = pMesh->mNumFaces;
				UINT indexcount = facecount * 3;
				UINT material = pMesh->mMaterialIndex;
				UINT stride = 0;
				UINT vertexsize = 0;
				void* vertexbuffer = nullptr;
				UINT indexsize = 0;
				UINT* indexbuffer = nullptr;

				if (pMesh->HasBones())
				{
					std::vector<TAnimatedMesh> VertexBuffer(vertexcount);
					std::vector<std::vector<aiVertexWeight>> weightsPerVertex(vertexcount);
					for (UINT a = 0; a < pMesh->mNumBones; a++)	{
						const aiBone* bone = pMesh->mBones[a];
						for (unsigned int b = 0; b < bone->mNumWeights; b++)
						{
							auto it = find(BoneOrderList.begin(), BoneOrderList.end(), bone);
							weightsPerVertex[bone->mWeights[b].mVertexId].push_back(aiVertexWeight((UINT)(it - BoneOrderList.begin()), bone->mWeights[b].mWeight));
						}
					}

					for (UINT i = 0; i < vertexcount; i++) {
						TAnimatedMesh m;
						unsigned char boneIndices[4] = { 0, 0, 0, 0 };
						unsigned char boneWeights[4] = { 0, 0, 0, 0 };
						for (UINT a = 0; a < min(4u, (UINT)weightsPerVertex[i].size()); a++)
						{
							boneIndices[a] = (unsigned char)weightsPerVertex[i][a].mVertexId;
							boneWeights[a] = (unsigned char)(weightsPerVertex[i][a].mWeight * 255.0f);
						}
						memcpy(&m.Position, &pMesh->mVertices[i], sizeof(XMFLOAT3));
						memcpy(&m.Normal, &pMesh->mNormals[i], sizeof(XMFLOAT3));
						memcpy(&m.Tex, &pMesh->mTextureCoords[0][i], sizeof(XMFLOAT2));
						memcpy(&m.BoneIndices, &boneIndices[0], sizeof(UINT));
						memcpy(&m.BoneWeights, &boneWeights[0], sizeof(FLOAT));
						VertexBuffer[i] = m;
					}

					XMVECTOR min = g_XMFltMax;
					XMVECTOR max = g_XMFltMin;
					for (auto it : VertexBuffer)
					{
						XMVECTOR pos = XMLoadFloat3(&it.Position);
						min = XMVectorMin(min, pos);
						max = XMVectorMax(max, pos);
					}
					XMVECTOR center = (min + max) / 2.0f;
					XMVECTOR extents = max - center;
					XMStoreFloat3(&mBoundingBox.Center, center);
					XMStoreFloat3(&mBoundingBox.Extents, extents);

					stride = sizeof(TAnimatedMesh);
					vertexsize = vertexcount * stride;
					vertexbuffer = malloc(vertexsize);
					memcpy(vertexbuffer, &VertexBuffer[0], vertexsize);
				}
				else
				{
					std::vector<TMesh> VertexBuffer(vertexcount);
					for (UINT i = 0; i < vertexcount; i++) {
						TMesh m;
						memcpy(&m.Position, &pMesh->mVertices[i], sizeof(XMFLOAT3));
						memcpy(&m.Normal, &pMesh->mNormals[i], sizeof(XMFLOAT3));
						memcpy(&m.Tex, &pMesh->mTextureCoords[0][i], sizeof(XMFLOAT2));
						VertexBuffer[i] = m;
					}

					XMVECTOR min = g_XMFltMax;
					XMVECTOR max = g_XMFltMin;
					for (auto it : VertexBuffer)
					{
						XMVECTOR pos = XMLoadFloat3(&it.Position);
						min = XMVectorMin(min, pos);
						max = XMVectorMax(max, pos);
					}
					XMVECTOR center = (min + max) / 2.0f;
					XMVECTOR extents = max - center;
					XMStoreFloat3(&mBoundingBox.Center, center);
					XMStoreFloat3(&mBoundingBox.Extents, extents);

					stride = sizeof(Mesh);
					vertexsize = vertexcount * stride;
					vertexbuffer = malloc(vertexsize);
					memcpy(vertexbuffer, &VertexBuffer[0], vertexsize);
				}

				std::vector<UINT> IndexBuffer(indexcount);
				for (UINT i = 0, n = 0; i < facecount; i++) {
					for (UINT j = 0; j < 3; j++, n++)
						IndexBuffer[n] = pMesh->mFaces[i].mIndices[j];
				}
				indexsize = (UINT)IndexBuffer.size() * sizeof(UINT);

				mppMeshList[index] = new Mesh(pd3dDevice, stride, vertexsize, vertexbuffer, indexsize, &IndexBuffer[0], material);
				free(vertexbuffer);
			}

			/* --- Create Materials --- */
			mppMaterialList.reserve(materialcount);
			for (size_t i = 0; i < materialcount; i++)
			{
				for (UINT j = 0; j < 1/*pScene->mMaterials[i]->GetTextureCount(aiTextureType_DIFFUSE)*/; j++)
				{
					aiString path;
					pScene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, j, &path);
					mppMaterialList.push_back(AssetLibrary::GetTexture(path.C_Str()));
				}
			}
			/* --- Create Materials --- */

			AnimationController::Load(pScene, BoneOrderList);

			aiReleaseImport(pScene);
		}
	}

	void AnimatedModel::ExtractMeshs(ID3D11Device* pd3dDevice, std::istream& pFile)
	{
		UINT meshcount = 0;
		UINT materialcount = 0;
		char animated = 0;

		pFile.read((char*)&meshcount, sizeof(meshcount));
		pFile.read((char*)&materialcount, sizeof(materialcount));
		pFile.read((char*)&mBoundingBox.Center, sizeof(mBoundingBox.Center));
		pFile.read((char*)&mBoundingBox.Extents, sizeof(mBoundingBox.Extents));
		pFile.read((char*)&animated, sizeof(animated));

		mppMeshList.resize(meshcount);
		for (size_t i = 0; i < meshcount; i++)
		{
			UINT vertexsize = 0;
			UINT indexsize = 0;
			UINT stride = 0;
			UINT material = 0;

			// Header
			pFile.read((char*)&vertexsize, sizeof(vertexsize));
			pFile.read((char*)&indexsize, sizeof(indexsize));
			pFile.read((char*)&stride, sizeof(stride));
			pFile.read((char*)&material, sizeof(material));

			void* vertexbuffer = malloc(vertexsize);
			Rath::read_compressed(pFile, (char*)vertexbuffer, vertexsize);

			UINT* indexbuffer = (UINT*)malloc(indexsize);
			Rath::read_compressed(pFile, (char*)indexbuffer, indexsize);

			mppMeshList[i] = new Mesh(pd3dDevice, stride, vertexsize, vertexbuffer, indexsize, indexbuffer, material);

			free(vertexbuffer);
			free(indexbuffer);
		}

		for (size_t i = 0; i < materialcount; i++)
		{
			std::string buffer;
			pFile >> buffer;
			mppMaterialList.push_back(AssetLibrary::GetTexture(buffer.c_str()));
		}

		if (animated)
		{
			AnimationController::Load(pFile);
		}
	}

	void AnimatedModel::LoadAnimation(LPCSTR pFile)
	{
		HRESULT hr = S_OK;
		CHAR buffer[_MAX_PATH] = MODELFOLDER;
		strcat_s(buffer, pFile);
		const aiScene* pScene = aiImportFile(buffer,
			//aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_ConvertToLeftHanded);
		if (pScene)
		{
			AnimationController::Load(pScene);

			aiReleaseImport(pScene);
		}
	}
}