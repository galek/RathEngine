#include "stdafx.h"
#include "ChunkMesh.h"
#include "Chunk.h"

UINT ChunkMesh::uiStride = sizeof(BLOCK_MESH);
ChunkMesh::ChunkMesh() :
	uiOffset(0),
	uiSize(0),
	uiRealSize(0),
	pVertexBuffer(nullptr),
	pLightTexture(nullptr),
	pLightTextureSRV(nullptr),
	pCollisionShape(nullptr)
{
}

ChunkMesh::~ChunkMesh()
{
	SAFE_RELEASE(pVertexBuffer);
	SAFE_RELEASE(pLightTexture);
	SAFE_RELEASE(pLightTextureSRV);

	if (pCollisionShape != nullptr)
	{
		Rath::PhysicsManager::instance().RemoveActor(pCollisionShape);
		pCollisionShape->release();
	}
}

void ChunkMesh::Create(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pImmediateContext)
{
	if (pd3dDevice)
	{
		std::vector<BLOCK_MESH> Mesh;
		Mesh.reserve(CHUNK_SQRWIDTH * CHUNK_HEIGHT);
		std::vector<uint32>  Light((CHUNK_WIDTH + 2) * (CHUNK_HEIGHT + 2) * (CHUNK_WIDTH + 2));

		GenerateMesh(Mesh, Light);

		HRESULT hr = S_OK;

		uiSize = (UINT)Mesh.size();
		if (uiRealSize < uiSize)
		{
			uiRealSize = uiSize;
			ID3D11Buffer*			VertexBuffer = nullptr;
			D3D11_BUFFER_DESC		Desc;
			D3D11_SUBRESOURCE_DATA	InitData = { &Mesh[0] , 0, 0 };
			Desc.Usage = D3D11_USAGE_DEFAULT;
			Desc.CPUAccessFlags = 0;
			Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			Desc.ByteWidth = uiRealSize * sizeof(BLOCK_MESH);
			Desc.MiscFlags = 0;
			Desc.StructureByteStride = 0;
			hr = pd3dDevice->CreateBuffer(&Desc, &InitData, &VertexBuffer);
			VertexBuffer = (ID3D11Buffer*)InterlockedExchangePointer((void * volatile *)&pVertexBuffer, VertexBuffer);
			SAFE_RELEASE(VertexBuffer);
		}
		else
		{
			D3D11_BOX destRegion = { 0, 0, 0, uiSize * sizeof(BLOCK_MESH), 1, 1 };
			pImmediateContext->UpdateSubresource(pVertexBuffer, 0,
				&destRegion, &Mesh[0], 0, 0);
		}

		if (pLightTexture == nullptr)
		{
			ID3D11Texture3D*			LightTexture = nullptr;
			ID3D11ShaderResourceView*	LightTextureSRV = nullptr;

			D3D11_TEXTURE3D_DESC Desc = { CHUNK_WIDTH + 2, CHUNK_HEIGHT, CHUNK_WIDTH + 2, 1, DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0 };
			D3D11_SUBRESOURCE_DATA InitialData = { &Light[0], (CHUNK_WIDTH + 2) * sizeof(uint32), (CHUNK_WIDTH + 2) * (CHUNK_HEIGHT) * sizeof(uint32) };
			hr = pd3dDevice->CreateTexture3D(&Desc, &InitialData, &LightTexture);

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = { DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_SRV_DIMENSION_TEXTURE3D };
			srvDesc.Texture3D.MipLevels = 1;
			srvDesc.Texture3D.MostDetailedMip = 0;
			hr = pd3dDevice->CreateShaderResourceView(LightTexture, &srvDesc, &LightTextureSRV);

			LightTexture = (ID3D11Texture3D*)InterlockedExchangePointer((void * volatile *)&pLightTexture, LightTexture);
			LightTextureSRV = (ID3D11ShaderResourceView*)InterlockedExchangePointer((void * volatile *)&pLightTextureSRV, LightTextureSRV);

			SAFE_RELEASE(LightTexture);
			SAFE_RELEASE(LightTextureSRV);
		}
		else
		{
			D3D11_BOX destRegion = { 0, 0, 0, CHUNK_WIDTH + 2, CHUNK_HEIGHT, CHUNK_WIDTH + 2 };
			pImmediateContext->UpdateSubresource(pLightTexture, 0, 
				&destRegion, &Light[0], 
				(CHUNK_WIDTH + 2) * sizeof(uint32), 
				(CHUNK_WIDTH + 2) * sizeof(uint32) * (CHUNK_HEIGHT));
		}
	}

	GenerateShape();
}

void ChunkMesh::Render(ID3D11DeviceContext* pImmediateContext)
{
	if (pVertexBuffer)
	{
		ID3D11Buffer * VS[] = { pVertexBuffer };
		ID3D11ShaderResourceView* SRV[] = { pLightTextureSRV };
		pImmediateContext->IASetVertexBuffers(0, 1, VS, &uiStride, &uiOffset);
		//pImmediateContext->VSSetShaderResources(0, 1, SRV);
		pImmediateContext->PSSetShaderResources(2, 1, SRV);
		//pImmediateContext->DrawInstanced(uiSize, 3, 0, 0);
		pImmediateContext->Draw(uiSize, 0);
	}
}

void ChunkMesh::RenderDepthOnly(ID3D11DeviceContext* pImmediateContext, uint32 flags)
{
	if (pVertexBuffer)
	{
		ID3D11Buffer * VS[] = { pVertexBuffer };
		pImmediateContext->IASetVertexBuffers(0, 1, VS, &uiStride, &uiOffset);
		pImmediateContext->Draw(flags ? uiSize : uiShadowSize, 0);
	}
}

void ChunkMesh::BindLightTexture(ID3D11DeviceContext* pImmediateContext)
{
	ID3D11ShaderResourceView* SRV[] = { pLightTextureSRV };
	//pImmediateContext->VSSetShaderResources(0, 1, SRV);
	pImmediateContext->PSSetShaderResources(2, 1, SRV);
}