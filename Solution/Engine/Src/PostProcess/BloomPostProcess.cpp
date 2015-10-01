#include "pch.h"
#include "BackBuffer.h"
#include "BloomPostProcess.h"

namespace Rath
{
	HRESULT BackBuffer::BloomPostProcess::CreateDevice(ID3D11Device* device)
	{
		HRESULT hr = S_OK;

#define MakePath(string) SHADERFOLDER string ".cso"

		hr = Technique::LoadShader(MakePath("LuminanceMap_ps"), device, &m_LuminanceMapPShader);
		assert(hr == S_OK);

		hr = Technique::LoadShader(MakePath("AdaptLuminance_ps"), device, &m_AdaptLuminancePShader);
		assert(hr == S_OK);

		hr = Technique::LoadShader(MakePath("Threshold_ps"), device, &m_ThresholdPShader);
		assert(hr == S_OK);

		hr = Technique::LoadShader(MakePath("Scale_ps"), device, &m_ScalePShader);
		assert(hr == S_OK);

		hr = Technique::LoadShader(MakePath("BloomBlurH_ps"), device, &m_BloomBlurHPShader);
		assert(hr == S_OK);

		hr = Technique::LoadShader(MakePath("BloomBlurV_ps"), device, &m_BloomBlurVPShader);
		assert(hr == S_OK);

		hr = Technique::LoadShader(MakePath("Composite_ps"), device, &m_CompositePShader);
		assert(hr == S_OK);

		m_LumenConstants.Initialize(device);

		m_LuminanceMap.Initialize(device, LumMapSize, LumMapSize, DXGI_FORMAT_R32_FLOAT, 10, 1, 0, TRUE, TRUE);

		m_AdaptLuminance[0].Initialize(device, 1, 1, DXGI_FORMAT_R32_FLOAT);
		m_AdaptLuminance[1].Initialize(device, 1, 1, DXGI_FORMAT_R32_FLOAT);

		m_CurrLumTarget = 0;

		return hr;
	}

	HRESULT BackBuffer::BloomPostProcess::CreateResources(ID3D11Device* device)
	{
		m_BloomSource.Initialize(device, m_parent.m_SurfaceDesc.Width, m_parent.m_SurfaceDesc.Height, DXGI_FORMAT_R11G11B10_FLOAT);

		m_DownScale[0].Initialize(device, m_parent.m_SurfaceDesc.Width / 2, m_parent.m_SurfaceDesc.Height / 2, DXGI_FORMAT_R11G11B10_FLOAT);
		m_DownScale[1].Initialize(device, m_parent.m_SurfaceDesc.Width / 4, m_parent.m_SurfaceDesc.Height / 4, DXGI_FORMAT_R11G11B10_FLOAT);
		m_DownScale[2].Initialize(device, m_parent.m_SurfaceDesc.Width / 8, m_parent.m_SurfaceDesc.Height / 8, DXGI_FORMAT_R11G11B10_FLOAT);
		m_DownScale[3].Initialize(device, m_parent.m_SurfaceDesc.Width / 8, m_parent.m_SurfaceDesc.Height / 8, DXGI_FORMAT_R11G11B10_FLOAT);

		return S_OK;
	}

	void BackBuffer::BloomPostProcess::CalcAvgLuminance(ID3D11DeviceContext* context, ID3D11ShaderResourceView* input)
	{
		// Luminance mapping
		ID3D11ShaderResourceView* pSRView[] = { input };
		m_parent.DrawFullScreen(context, 1, pSRView, m_LuminanceMap.RTView, m_LuminanceMapPShader);

		// Generate the mip chain
		context->GenerateMips(m_LuminanceMap.SRView);

		// Adaptation
		ID3D11ShaderResourceView* pSRViews[] = { m_AdaptLuminance[!m_CurrLumTarget].SRView, m_LuminanceMap.SRView };
		m_parent.DrawFullScreen(context, 2, pSRViews, m_AdaptLuminance[m_CurrLumTarget].RTView, m_AdaptLuminancePShader);
	}

	void BackBuffer::BloomPostProcess::Bloom(ID3D11DeviceContext* context, ID3D11ShaderResourceView* input)
	{
		// Downscale
		ID3D11ShaderResourceView* pSRViews[] = { input, m_AdaptLuminance[m_CurrLumTarget].SRView };
		m_parent.DrawFullScreen(context, 2, pSRViews, m_BloomSource.RTView, m_ThresholdPShader);


		ID3D11ShaderResourceView* pSRView[] = { m_BloomSource.SRView };
		m_parent.DrawFullScreen(context, 1, pSRView, m_DownScale[0].RTView, m_ScalePShader);

		pSRView[0] = m_DownScale[0].SRView;
		m_parent.DrawFullScreen(context, 1, pSRView, m_DownScale[1].RTView, m_ScalePShader);

		pSRView[0] = m_DownScale[1].SRView;
		m_parent.DrawFullScreen(context, 1, pSRView, m_DownScale[2].RTView, m_ScalePShader);

		// Blur it
		for (uint64 i = 0; i < 4; ++i)
		{
			pSRView[0] = m_DownScale[2].SRView;
			m_parent.DrawFullScreen(context, 1, pSRView, m_DownScale[3].RTView, m_BloomBlurHPShader);

			ID3D11ShaderResourceView* ppSRVNULL[] = { nullptr, nullptr };
			context->PSSetShaderResources(0, ARRAYSIZE(ppSRVNULL), ppSRVNULL);

			pSRView[0] = m_DownScale[3].SRView;
			m_parent.DrawFullScreen(context, 1, pSRView, m_DownScale[2].RTView, m_BloomBlurVPShader);

			context->PSSetShaderResources(0, ARRAYSIZE(ppSRVNULL), ppSRVNULL);
		}

		pSRView[0] = m_DownScale[2].SRView;
		m_parent.DrawFullScreen(context, 1, pSRView, m_DownScale[1].RTView, m_ScalePShader);

		pSRView[0] = m_DownScale[1].SRView;
		m_parent.DrawFullScreen(context, 1, pSRView, m_DownScale[0].RTView, m_ScalePShader);
	}

	void BackBuffer::BloomPostProcess::ToneMap(ID3D11DeviceContext* context, ID3D11ShaderResourceView* input, ID3D11RenderTargetView* output)
	{
		// Composite the bloom with the original image, and apply tone-mapping
		ID3D11ShaderResourceView* pSRViews[] = { input, m_AdaptLuminance[m_CurrLumTarget].SRView, m_DownScale[0].SRView };
		m_parent.DrawFullScreen(context, 3, pSRViews, output, m_CompositePShader);
	}

	void BackBuffer::BloomPostProcess::FrameMove(float fElapsedTime)
	{
		m_LumenConstants.Data = { 3.0f, 1.0f, 0.8f, 0.5f, fElapsedTime, 0.115f, 0.005f, 0.1f };
	}

	void BackBuffer::BloomPostProcess::Render(ID3D11DeviceContext* context, RenderTarget2D* input, RenderTarget2D* output)
	{
		m_LumenConstants.ApplyChanges(context);
		m_LumenConstants.SetPS(context, 1);

		// Calculate the average luminance first
		CalcAvgLuminance(context, input->SRView);

		// Now do the bloom
		Bloom(context, input->SRView);

		// Apply tone mapping
		ToneMap(context, input->SRView, output->RTView);

		m_CurrLumTarget = !m_CurrLumTarget;
	}
}