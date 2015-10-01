#include "pch.h"
#include "UIManager.h"
#include "AssetLibrary.h"

namespace Rath
{
	UIManager::UIManager() :
		m_uiVertexSize(0),
		m_uiIndexSize(0)
	{
		m_pUITechnique = AssetLibrary::GetTechnique("UIShader");

		m_DefaultFont = new FontElement("TimesNewRoman.fnt");
	}

	UIManager::~UIManager()
	{
		for (auto it : m_Elements)
		{
			delete it;
		}
		SAFE_DELETE(m_DefaultFont);
	}

	void UIManager::CreateDevice(ID3D11Device* device)
	{
		m_pd3dDevice = device;

		D3D11_BUFFER_DESC bd;
		if (m_uiVertexSize)
		{
			bd.Usage = D3D11_USAGE_DYNAMIC;
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bd.ByteWidth = m_uiVertexSize;
			bd.MiscFlags = 0;
			bd.StructureByteStride = 0;
			DX::ThrowIfFailed(device->CreateBuffer(&bd, nullptr, m_pVertexBuffer.ReleaseAndGetAddressOf()));
		}

		if (m_uiIndexSize)
		{
			bd.Usage = D3D11_USAGE_DYNAMIC;
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
			bd.ByteWidth = m_uiIndexSize;
			bd.MiscFlags = 0;
			bd.StructureByteStride = 0;
			DX::ThrowIfFailed(device->CreateBuffer(&bd, nullptr, m_pIndexBuffer.ReleaseAndGetAddressOf()));
		}
	}

	void UIManager::WindowSizeChanged(uint32 width, uint32 height)
	{
		m_DefaultFont->WindowSizeChanged(width, height);
	}

	void UIManager::CopyBuffer(UIElement* pElement, ID3D11DeviceContext* context)
	{
		UINT VertexSize = UINT(pElement->m_pMesh.size() * sizeof(UI_MESH));
		UINT IndexSize = UINT(pElement->m_pIndex.size() * sizeof(UINT));

		D3D11_BUFFER_DESC bd;
		D3D11_MAPPED_SUBRESOURCE MappedResource;
		if (VertexSize > m_uiVertexSize)
		{
			D3D11_SUBRESOURCE_DATA InitData;
			ZeroMemory(&InitData, sizeof(InitData));
			InitData.pSysMem = &pElement->m_pMesh[0];

			m_uiVertexSize = VertexSize;
			bd.Usage = D3D11_USAGE_DYNAMIC;
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bd.ByteWidth = m_uiVertexSize;
			bd.MiscFlags = 0;
			bd.StructureByteStride = 0;
			DX::ThrowIfFailed(m_pd3dDevice->CreateBuffer(&bd, &InitData, m_pVertexBuffer.ReleaseAndGetAddressOf()));
		}
		else if (SUCCEEDED(context->Map(m_pVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource)))
		{
			memcpy(MappedResource.pData, &pElement->m_pMesh[0], VertexSize);
			context->Unmap(m_pVertexBuffer.Get(), 0);
		}

		if (IndexSize > m_uiIndexSize)
		{
			D3D11_SUBRESOURCE_DATA InitData;
			ZeroMemory(&InitData, sizeof(InitData));
			InitData.pSysMem = &pElement->m_pIndex[0];

			m_uiIndexSize = IndexSize;
			bd.Usage = D3D11_USAGE_DYNAMIC;
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
			bd.ByteWidth = m_uiIndexSize;
			bd.MiscFlags = 0;
			bd.StructureByteStride = 0;
			DX::ThrowIfFailed(m_pd3dDevice->CreateBuffer(&bd, &InitData, m_pIndexBuffer.ReleaseAndGetAddressOf()));
		}
		else if (SUCCEEDED(context->Map(m_pIndexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource)))
		{
			memcpy(MappedResource.pData, &pElement->m_pIndex[0], IndexSize);
			context->Unmap(m_pIndexBuffer.Get(), 0);
		}
	}

	void UIManager::AddElement(UIElement* element)
	{
		if (element != nullptr)
			m_Elements.push_back(element);
	}

	void UIManager::RemoveElement(UIElement* element)
	{
		auto it = std::find(m_Elements.begin(), m_Elements.end(), element);
		if (it != m_Elements.end())
		{
			m_Elements.erase(it);
		}
	}

	void UIManager::PrintText(LPCWCHAR text, const XMFLOAT2& position, FLOAT size, const FLOAT ColorRGBA[4], FLOAT depth)
	{
		m_DefaultFont->PrintText(text, position, size, ColorRGBA, depth);
	}

	void UIManager::PrintText(LPCWCHAR text, const XMINT2& position, UINT size, const FLOAT ColorRGBA[4], FLOAT depth)
	{
		m_DefaultFont->PrintText(text, position, size, ColorRGBA, depth);
	}

	void UIManager::Render(ID3D11DeviceContext* context)
	{
		UINT offset = 0;
		UINT stride = sizeof(UI_MESH);

		m_pUITechnique->Apply(context);

		for (auto it : m_Elements) if (it->m_uiIndexSize)
		{
			CopyBuffer(it, context);
			it->m_pTexture->Apply(context);
			context->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);
			context->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
			context->DrawIndexed(it->m_uiIndexSize, 0, 0);
		}

		if (m_DefaultFont->m_uiIndexSize)
		{
			CopyBuffer(m_DefaultFont, context);
			m_DefaultFont->m_pTexture->Apply(context);
			context->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);
			context->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
			context->DrawIndexed(m_DefaultFont->m_uiIndexSize, 0, 0);
			m_DefaultFont->Clear();
		}
	}
}