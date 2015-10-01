#pragma once
#include "Loadable.h"

namespace Rath
{
	class __declspec(uuid("{2C1BB63F-CC82-435F-AC3B-C0C847647C16}")) Texture : public Loadable
	{
	protected:
		ID3D11Resource*				m_texture;
		ID3D11ShaderResourceView*	m_textureSRV;

		HRESULT						Create(ID3D11Device* pd3dDevice, LPCSTR filename, const void* data = nullptr);
	public:
		Texture();
		~Texture();

		void GetTextureSize(UINT* width, UINT* height);
		void Apply(ID3D11DeviceContext* pd3dImmediateContext);

		void BindToVS(ID3D11DeviceContext* deviceContext, uint32 slot) const;
		void BindToPS(ID3D11DeviceContext* deviceContext, uint32 slot) const;
		void BindToGS(ID3D11DeviceContext* deviceContext, uint32 slot) const;
		void BindToHS(ID3D11DeviceContext* deviceContext, uint32 slot) const;
		void BindToDS(ID3D11DeviceContext* deviceContext, uint32 slot) const;
		void BindToCS(ID3D11DeviceContext* deviceContext, uint32 slot) const;
	};

	_COM_SMARTPTR_TYPEDEF(Texture, __uuidof(Texture));
}