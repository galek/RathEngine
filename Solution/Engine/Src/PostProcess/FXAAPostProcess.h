#pragma once

namespace Rath
{
	class BackBuffer::FXAAPostProcess
	{
	private:
		struct FXAA_CB_STRUCT
		{
			DirectX::SimpleMath::Vector4 m_fxaa;
		};
		BackBuffer& m_parent;
	protected:
		ConstantBuffer<FXAA_CB_STRUCT>	m_FxaaConstants;
		ID3D11PixelShaderPtr			m_FXAAPShader;
	public:
		FXAAPostProcess(BackBuffer& x) : m_parent(x) {};

		HRESULT CreateDevice(ID3D11Device* device);
		void	Render(ID3D11DeviceContext* context, ID3D11ShaderResourceView* input, ID3D11RenderTargetView* output);
	};
}
