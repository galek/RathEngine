#include "pch.h"
#include "Texture.h"

#include "Shlwapi.h"

#include "WICTextureLoader.h"
#include "DDSTextureLoader.h"
namespace Rath
{
	void Texture::GetTextureSize(UINT* width, UINT* height)
	{
		// This is the most generic solution. you can make it a lot
		// simpler if you know it will always be a 2D texture file
		D3D11_RESOURCE_DIMENSION dim;
		m_texture->GetType(&dim);
		switch (dim)
		{
		case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
		{
			auto txt = reinterpret_cast<ID3D11Texture1D*>(m_texture);
			D3D11_TEXTURE1D_DESC desc;
			txt->GetDesc(&desc);
			if (width) *width = desc.Width;
			if (height) *height = 1;
		}
		break;
		case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
		{
			auto txt = reinterpret_cast<ID3D11Texture2D*>(m_texture);
			D3D11_TEXTURE2D_DESC desc;
			txt->GetDesc(&desc);
			if (width) *width = desc.Width;
			if (height) *height = desc.Height;
		}
		break;
		case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
		{
			auto txt = reinterpret_cast<ID3D11Texture3D*>(m_texture);
			D3D11_TEXTURE3D_DESC desc;
			txt->GetDesc(&desc);
			if (width) *width = desc.Width;
			if (height) *height = desc.Height;
		}
		break;
		default:
			if (width) *width = 0;
			if (height) *height = 0;
			break;
		}
	};

	HRESULT Texture::Create(ID3D11Device* pd3dDevice, LPCSTR filename, const void* data)
	{
		HRESULT hr = S_OK;

		SAFE_RELEASE(m_texture);
		SAFE_RELEASE(m_textureSRV);

		WCHAR buffer[_MAX_PATH];
		swprintf_s(buffer, _MAX_PATH, L"%S%S", TEXTUREFOLDER, filename);

		LPWSTR pExtension = PathFindExtension(buffer);
		if (_wcsicmp(pExtension, L".dds") == 0)
		{
			V(DirectX::CreateDDSTextureFromFile(pd3dDevice, buffer, 
				&m_texture, &m_textureSRV));
		}
		else
		{
			V(DirectX::CreateWICTextureFromFile(pd3dDevice, nullptr, buffer, 
				&m_texture, &m_textureSRV));
		}

		return hr;
	}

	Texture::Texture() :
		m_texture(nullptr),
		m_textureSRV(nullptr)
	{
	}

	Texture::~Texture()
	{
		SAFE_RELEASE(m_texture);
		SAFE_RELEASE(m_textureSRV);
	}

	void Texture::Apply(ID3D11DeviceContext* pd3dImmediateContext)
	{
		ID3D11ShaderResourceView* SRV[] = { m_textureSRV };
		pd3dImmediateContext->PSSetShaderResources(0, 1, SRV);
	}

	void Texture::BindToVS(ID3D11DeviceContext* deviceContext, uint32 slot) const
	{
		Assert_(m_textureSRV != nullptr);
		ID3D11ShaderResourceView* SRV[] = { m_textureSRV };
		deviceContext->VSSetShaderResources(slot, 1, SRV);
	}

	void Texture::BindToPS(ID3D11DeviceContext* deviceContext, uint32 slot) const
	{
		Assert_(m_textureSRV != nullptr);
		ID3D11ShaderResourceView* SRV[] = { m_textureSRV };
		deviceContext->PSSetShaderResources(slot, 1, SRV);
	}

	void Texture::BindToGS(ID3D11DeviceContext* deviceContext, uint32 slot) const
	{
		Assert_(m_textureSRV != nullptr);
		ID3D11ShaderResourceView* SRV[] = { m_textureSRV };
		deviceContext->GSSetShaderResources(slot, 1, SRV);
	}

	void Texture::BindToHS(ID3D11DeviceContext* deviceContext, uint32 slot) const
	{
		Assert_(m_textureSRV != nullptr);
		ID3D11ShaderResourceView* SRV[] = { m_textureSRV };
		deviceContext->HSSetShaderResources(slot, 1, SRV);
	}

	void Texture::BindToDS(ID3D11DeviceContext* deviceContext, uint32 slot) const
	{
		Assert_(m_textureSRV != nullptr);
		ID3D11ShaderResourceView* SRV[] = { m_textureSRV };
		deviceContext->DSSetShaderResources(slot, 1, SRV);
	}

	void Texture::BindToCS(ID3D11DeviceContext* deviceContext, uint32 slot) const
	{
		Assert_(m_textureSRV != nullptr);
		ID3D11ShaderResourceView* SRV[] = { m_textureSRV };
		deviceContext->CSSetShaderResources(slot, 1, SRV);
	}
}