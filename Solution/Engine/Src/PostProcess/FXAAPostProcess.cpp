#include "pch.h"
#include "BackBuffer.h"
#include "FXAAPostProcess.h"

namespace Rath
{

	HRESULT BackBuffer::FXAAPostProcess::CreateDevice(ID3D11Device* device)
	{
		HRESULT hr = S_OK;

#define MakePath(string) SHADERFOLDER string ".cso"

		hr = Technique::LoadShader(MakePath("FXAA_ps"), device, &m_FXAAPShader);
		assert(hr == S_OK);

		m_FxaaConstants.Initialize(device);

		return hr;
	}

	void BackBuffer::FXAAPostProcess::Render(ID3D11DeviceContext* context, ID3D11ShaderResourceView* input, ID3D11RenderTargetView* output)
	{
		m_FxaaConstants.Data.m_fxaa = DirectX::SimpleMath::Vector4(1.0f / (float)m_parent.m_SurfaceDesc.Width, 1.0f / (float)m_parent.m_SurfaceDesc.Height, 0.0f, 0.0f);
		m_FxaaConstants.ApplyChanges(context);
		m_FxaaConstants.SetPS(context, 3);

		ID3D11ShaderResourceView* pSRViews[] = { input };
		m_parent.DrawFullScreen(context, 1, pSRViews, output, m_FXAAPShader);
	}
}