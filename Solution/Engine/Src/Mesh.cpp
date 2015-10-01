#include "pch.h"
#include "Mesh.h"
#include "FileIO.h"

namespace Rath
{
	Mesh::Mesh(ID3D11Device* device, INT Stride, UINT VertexSize, void* VertexBuffer, UINT IndexSize, UINT* IndexBuffer, UINT Material) :
		m_stride(Stride),
		m_material(Material),
		m_offset(0),
		m_pVertexBuffer(nullptr),
		m_pIndexBuffer(nullptr)
	{
		D3D11_BUFFER_DESC bd;
		D3D11_SUBRESOURCE_DATA InitData;
		ZeroMemory(&InitData, sizeof(InitData));

		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.CPUAccessFlags = 0;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.ByteWidth = VertexSize;
		bd.MiscFlags = 0;
		bd.StructureByteStride = 0;
		InitData.pSysMem = VertexBuffer;
		DX::ThrowIfFailed(device->CreateBuffer(&bd, &InitData, &m_pVertexBuffer));
		m_offset = 0;

		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.CPUAccessFlags = 0;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.ByteWidth = IndexSize;
		bd.MiscFlags = 0;
		bd.StructureByteStride = 0;
		InitData.pSysMem = IndexBuffer;
		DX::ThrowIfFailed(device->CreateBuffer(&bd, &InitData, &m_pIndexBuffer));
		m_indexSize = IndexSize / sizeof(UINT);
	}

	Mesh::~Mesh()
	{
		SAFE_RELEASE(m_pVertexBuffer);
		SAFE_RELEASE(m_pIndexBuffer);
	}

	void Mesh::Render(ID3D11DeviceContext* context)
	{
		ID3D11Buffer * VS[] = { m_pVertexBuffer };
		context->IASetVertexBuffers(0, 1, VS, &m_stride, &m_offset);
		context->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		context->DrawIndexed(m_indexSize, 0, 0);
	}

	ID3D11Mesh::~ID3D11Mesh()
	{
		SAFE_RELEASE(m_pVertexBuffer);
		SAFE_RELEASE(m_pIndexBuffer);
	}

	void ID3D11Mesh::Create(ID3D11Device* device, UINT VertexSize, void* VertexBuffer, UINT IndexSize, UINT* IndexBuffer)
	{
		SAFE_RELEASE(m_pVertexBuffer);
		SAFE_RELEASE(m_pIndexBuffer);

		D3D11_BUFFER_DESC bd;
		D3D11_SUBRESOURCE_DATA InitData;
		ZeroMemory(&InitData, sizeof(InitData));

		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.CPUAccessFlags = 0;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.ByteWidth = VertexSize;
		bd.MiscFlags = 0;
		bd.StructureByteStride = 0;
		InitData.pSysMem = VertexBuffer;
		DX::ThrowIfFailed(device->CreateBuffer(&bd, &InitData, &m_pVertexBuffer));

		if (IndexBuffer)
		{
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.CPUAccessFlags = 0;
			bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
			bd.ByteWidth = IndexSize;
			bd.MiscFlags = 0;
			bd.StructureByteStride = 0;
			InitData.pSysMem = IndexBuffer;
			DX::ThrowIfFailed(device->CreateBuffer(&bd, &InitData, &m_pIndexBuffer));
		}
	}

	void ID3D11Mesh::Render(ID3D11DeviceContext* context)
	{
		ID3D11Buffer * VS[] = { m_pVertexBuffer };
		context->IASetVertexBuffers(0, 1, VS, &m_stride, &m_offset);
		if (m_pIndexBuffer)
		{
			context->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
			context->DrawIndexed(m_indexSize, 0, 0);
		}
		else
		{
			context->Draw(m_indexSize, 0);
		}

	}

	ID3D11InstancedMesh::~ID3D11InstancedMesh()
	{
		SAFE_RELEASE(m_pInstanceBuffer);
	}

	void ID3D11InstancedMesh::CreateInstancing(ID3D11Device* device, ID3D11DeviceContext* context, UINT InstanceSize, void* InstanceBuffer)
	{
		D3D11_MAPPED_SUBRESOURCE MappedResource;
		if (m_instanceRealSize < InstanceSize)
		{
			SAFE_RELEASE(m_pInstanceBuffer);

			D3D11_BUFFER_DESC bd;
			D3D11_SUBRESOURCE_DATA InitData;
			ZeroMemory(&InitData, sizeof(InitData));

			bd.Usage = D3D11_USAGE_DYNAMIC;
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bd.ByteWidth = InstanceSize;
			bd.MiscFlags = 0;
			bd.StructureByteStride = 0;
			InitData.pSysMem = InstanceBuffer;
			DX::ThrowIfFailed(device->CreateBuffer(&bd, &InitData, &m_pInstanceBuffer));

			m_instanceRealSize = InstanceSize;
		}
		else if (SUCCEEDED(context->Map(m_pInstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource)))
		{
			memcpy(MappedResource.pData, InstanceBuffer, InstanceSize);
			context->Unmap(m_pInstanceBuffer, 0);
		}
	}

	void ID3D11InstancedMesh::Render(ID3D11DeviceContext* context)
	{
		if (m_instanceSize > 0)
		{
			UINT STRIDE[] = { m_stride, m_instanceStride };
			UINT OFFSET[] = { m_offset, m_instanceOffset };
			ID3D11Buffer * VB[] = { m_pVertexBuffer, m_pInstanceBuffer };
			context->IASetVertexBuffers(0, 2, VB, STRIDE, OFFSET);

			if (m_pIndexBuffer)
			{
				context->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
				context->DrawIndexedInstanced(m_indexSize, m_instanceSize, 0, 0, 0);
			}
			else
			{
				context->DrawInstanced(m_indexSize, m_instanceSize, 0, 0);
			}
		}
	}
}