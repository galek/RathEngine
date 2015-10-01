#include "pch.h"
#include "BackBuffer.h"

#include "Debug\TimestampQueries.h"

#include "PostProcess\DepthOfFieldPostProcess.h"
#include "PostProcess\BloomPostProcess.h"
#include "PostProcess\FXAAPostProcess.h"
#include "PostProcess\CMAAPostProcess.h"

namespace Rath
{
	using namespace DirectX;

	using Microsoft::WRL::ComPtr;

	void BackBuffer::DrawFullScreen(
		_In_ ID3D11DeviceContext* pContext,
		_In_range_(0, 4)  UINT NumInputs,
		_In_reads_opt_(NumSamplers)  ID3D11ShaderResourceView *const *ppInputs,
		_In_ ID3D11RenderTargetView* pOutput,
		_In_ ID3D11PixelShader* pPixelShader)
	{

		for (size_t i = 0; i < NumInputs; ++i)
		{
			if (ppInputs[i] == nullptr)
			{
				m_PixelConstants.Data.InputSize[i].x = 0.0f;
				m_PixelConstants.Data.InputSize[i].y = 0.0f;
				continue;
			}

			ID3D11Resource* resource;
			ID3D11Texture2DPtr texture;
			D3D11_TEXTURE2D_DESC desc;
			D3D11_SHADER_RESOURCE_VIEW_DESC srDesc;
			ppInputs[i]->GetDesc(&srDesc);
			ppInputs[i]->GetResource(&resource);
			uint32 mipLevel = srDesc.Texture2D.MostDetailedMip;
			texture.Attach(reinterpret_cast<ID3D11Texture2D*>(resource));
			texture->GetDesc(&desc);
			m_PixelConstants.Data.InputSize[i].x = static_cast<float>(std::max<uint32>(desc.Width / (1 << mipLevel), 1));
			m_PixelConstants.Data.InputSize[i].y = static_cast<float>(std::max<uint32>(desc.Height / (1 << mipLevel), 1));
		}

		ID3D11Resource* resource;
		ID3D11Texture2DPtr texture;
		D3D11_TEXTURE2D_DESC desc;
		D3D11_RENDER_TARGET_VIEW_DESC rtDesc;
		pOutput->GetResource(&resource);
		pOutput->GetDesc(&rtDesc);
		uint32 mipLevel = rtDesc.Texture2D.MipSlice;
		texture.Attach(reinterpret_cast<ID3D11Texture2D*>(resource));
		texture->GetDesc(&desc);
		m_PixelConstants.Data.OutputSize.x = static_cast<float>(std::max<uint32>(desc.Width / (1 << mipLevel), 1));
		m_PixelConstants.Data.OutputSize.y = static_cast<float>(std::max<uint32>(desc.Height / (1 << mipLevel), 1));

		m_PixelConstants.ApplyChanges(pContext);

		// Set the viewports
		D3D11_VIEWPORT viewport;
		viewport.Width = static_cast<float>(std::max<uint32>(desc.Width / (1 << mipLevel), 1));
		viewport.Height = static_cast<float>(std::max<uint32>(desc.Height / (1 << mipLevel), 1));
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		pContext->RSSetViewports(1, &viewport);

		ID3D11RenderTargetView* pRTView[] = { pOutput };
		pContext->OMSetRenderTargets(1, pRTView, nullptr);
		pContext->PSSetShader(pPixelShader, nullptr, 0);
		pContext->PSSetShaderResources(0, NumInputs, ppInputs);
		pContext->Draw(3, 0);
	}

	BackBuffer::BackBuffer(EngineState* state) :
		m_Bloom(state->m_Bloom),
		m_Fxaa(state->m_Fxaa),
		m_DepthOfField(state->m_DepthOfField),
		m_LightShafts(state->m_LightShafts),
		m_pCamera(&state->m_Camera),
		m_SurfaceDesc(state->m_BackBufferDesc)
	{
		Assert_(m_pCamera != nullptr);

		m_BloomPostProcess.reset(new BloomPostProcess(*this));
		m_DepthOfFieldPostProcess.reset(new DepthOfFieldPostProcess(*this));
		m_FXAAPostProcess.reset(new FXAAPostProcess(*this));
	}

	BackBuffer::~BackBuffer()
	{
	}

	DirectX::SimpleMath::Vector2 BackBuffer::GetResolvedDepth()
	{
		return m_reductionDepth;
	}

	void BackBuffer::CreateShader(ID3D11Device* device)
	{
		HRESULT hr = S_OK;

#define MakePath(string) SHADERFOLDER string ".cso"

		hr = Technique::LoadShader(MakePath("ScreenTriangle_vs"), device, &m_ScreenTriangleVShader);
		assert(hr == S_OK);
		hr = Technique::LoadShader(MakePath("CopyBackBuffer_ps"), device, &m_CopyPShader);
		assert(hr == S_OK);
		hr = Technique::LoadShader(MakePath("MSAA_Detect_ps"), device, &m_MSAADetectPShader);
		assert(hr == S_OK);
		hr = Technique::LoadShader(MakePath("Resolve_Depth_ps"), device, &m_ResolveDepthPShader);
		assert(hr == S_OK);
		hr = Technique::LoadShader(MakePath("LightShafts_ps"), device, &m_LightShaftsPShader);
		assert(hr == S_OK);
		hr = Technique::LoadShader(MakePath("LightShafts_MS_ps"), device, &m_LightShaftsMSPShader);
		assert(hr == S_OK);
		hr = Technique::LoadShader(MakePath("CopyLightTexture_PS"), device, &m_CopyLightTexturePShader);
		assert(hr == S_OK);
		hr = Technique::LoadShader(MakePath("DepthReduction_cs"), device, &m_DepthReductionCShader);
		assert(hr == S_OK);
		hr = Technique::LoadShader(MakePath("DepthReductionInitial_cs"), device, &m_DdepthReductionInitialCShader);
		assert(hr == S_OK);
	}

	void BackBuffer::CreateReductionTargets(ID3D11Device* device)
	{
		// Create the staging textures for reading back the reduced depth buffer
		for (uint32 i = 0; i < MaxReadbackLatency; ++i)
			m_ReductionStagingTextures[i].Initialize(device, 1, 1, DXGI_FORMAT_R16G16_UNORM);

		m_DepthReductionTargets.clear();

		uint32 w = m_SurfaceDesc.Width;
		uint32 h = m_SurfaceDesc.Height;

		auto DispatchSize = [](uint32 tgSize, uint32 numElements)
		{
			uint32 dispatchSize = numElements / tgSize;
			dispatchSize += numElements % tgSize > 0 ? 1 : 0;
			return dispatchSize;
		};

		while (w > 1 || h > 1)
		{
			w = DispatchSize(ReductionTGSize, w);
			h = DispatchSize(ReductionTGSize, h);

			RenderTarget2D rt;
			rt.Initialize(device, w, h, DXGI_FORMAT_R16G16_UNORM, 1, 1, 0, FALSE, TRUE);
			m_DepthReductionTargets.push_back(rt);
		}
	}

	void BackBuffer::CreateBackBuffers(ID3D11Device* device)
	{
		m_BackBuffer.Initialize(device,
			m_SurfaceDesc.Width,
			m_SurfaceDesc.Height,
			m_SurfaceDesc.Format,
			1,
			m_SurfaceDesc.SampleDesc.Count,
			m_SurfaceDesc.SampleDesc.Quality,
			false,
			false,
			1,
			false);

		for (size_t i = 0; i < ARRAYSIZE(m_NonMSAABackBuffer); i++)
			m_NonMSAABackBuffer[i].Initialize(device,
			m_SurfaceDesc.Width,
			m_SurfaceDesc.Height,
			m_SurfaceDesc.Format,
			1,
			1,
			0,
			false,
			true,
			1,
			false);

		m_DepthStencil.Initialize(device,
			m_SurfaceDesc.Width,
			m_SurfaceDesc.Height,
			DXGI_FORMAT_D32_FLOAT,
			true,
			m_SurfaceDesc.SampleDesc.Count,
			m_SurfaceDesc.SampleDesc.Quality,
			1);
	}

	void BackBuffer::CreateEffects(ID3D11Device* device)
	{
		if (m_SurfaceDesc.SampleDesc.Count > 1)
		{
			m_ResolvedDepth.Initialize(device,
				m_SurfaceDesc.Width,
				m_SurfaceDesc.Height,
				DXGI_FORMAT_R32_FLOAT);
		}

		if (m_LightShafts)
		{
			m_LightShaftsBuffer.Initialize(device,
				(m_SurfaceDesc.Width + 1) / 2,
				(m_SurfaceDesc.Height + 1) / 2,
				DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
		}

		if (m_Bloom)
		{
			m_BloomPostProcess->CreateResources(device);
		}

		if (m_DepthOfField)
		{
			m_DepthOfFieldPostProcess->CreateResources(device);
		}
	}

	void BackBuffer::CreateDevice(ID3D11Device* device)
	{
		CreateShader(device);

		m_PixelConstants.Initialize(device);
		m_MSAAConstants.Initialize(device);
		m_ReductionConstants.Initialize(device);

		if (m_Bloom)
		{
			m_BloomPostProcess->CreateDevice(device);
		}
		if (m_DepthOfField)
		{
			m_DepthOfFieldPostProcess->CreateDevice(device);
		}
		if (m_Fxaa)
		{
			m_FXAAPostProcess->CreateDevice(device);
		}
	}

	void BackBuffer::CreateResources(ID3D11Device* device, const DXGI_SURFACE_DESC& BackBufferDesc)
	{
		m_SurfaceDesc = BackBufferDesc;

		CreateBackBuffers(device);
		CreateReductionTargets(device);
		CreateEffects(device);
	}

	void BackBuffer::ResolveDepth(ID3D11DeviceContext* context)
	{
		if (m_SurfaceDesc.SampleDesc.Count > 1)
		{
			ID3D11ShaderResourceView*	SRV_Array[] = { m_DepthStencil.SRView };
			ID3D11RenderTargetView*		RTV_Array[] = { m_ResolvedDepth.RTView };

			context->OMSetRenderTargets(ARRAYSIZE(RTV_Array), RTV_Array, nullptr);
			context->VSSetShader(m_ScreenTriangleVShader, nullptr, 0);
			context->PSSetShader(m_ResolveDepthPShader, nullptr, 0);
			context->PSSetShaderResources(0, ARRAYSIZE(SRV_Array), SRV_Array);
			context->Draw(3, 0);
		}
	}

	void BackBuffer::ReduceDepth(ID3D11DeviceContext* context)
	{
		m_ReductionConstants.Data.Projection = XMMatrixTranspose(m_pCamera->GetCameraProjection());
		m_ReductionConstants.Data.NearClip = m_pCamera->GetNearClip();
		m_ReductionConstants.Data.FarClip = m_pCamera->GetFarClip();
		m_ReductionConstants.ApplyChanges(context);
		m_ReductionConstants.SetCS(context, 1);

		ID3D11RenderTargetView* rtvs[1] = { nullptr };
		context->OMSetRenderTargets(1, rtvs, nullptr);

		ID3D11UnorderedAccessView* uavs[1] = { m_DepthReductionTargets[0].UAView };
		context->CSSetUnorderedAccessViews(0, 1, uavs, nullptr);

		ID3D11ShaderResourceView* srvs[1] = { m_DepthStencil.SRView };
		if (m_SurfaceDesc.SampleDesc.Count > 1)
			srvs[0] = m_ResolvedDepth.SRView;
		context->CSSetShaderResources(0, 1, srvs);

		context->CSSetShader(m_DdepthReductionInitialCShader, nullptr, 0);

		uint32 dispatchX = m_DepthReductionTargets[0].Width;
		uint32 dispatchY = m_DepthReductionTargets[0].Height;
		context->Dispatch(dispatchX, dispatchY, 1);

		uavs[0] = nullptr;
		context->CSSetUnorderedAccessViews(0, 1, uavs, nullptr);

		srvs[0] = nullptr;
		context->CSSetShaderResources(0, 1, srvs);

		context->CSSetShader(m_DepthReductionCShader, nullptr, 0);

		for (uint32 i = 1; i < m_DepthReductionTargets.size(); ++i)
		{
			uavs[0] = m_DepthReductionTargets[i].UAView;
			context->CSSetUnorderedAccessViews(0, 1, uavs, nullptr);

			srvs[0] = m_DepthReductionTargets[i - 1].SRView;
			context->CSSetShaderResources(0, 1, srvs);

			dispatchX = m_DepthReductionTargets[i].Width;
			dispatchY = m_DepthReductionTargets[i].Height;
			context->Dispatch(dispatchX, dispatchY, 1);

			uavs[0] = nullptr;
			context->CSSetUnorderedAccessViews(0, 1, uavs, nullptr);

			srvs[0] = nullptr;
			context->CSSetShaderResources(0, 1, srvs);
		}

		// Copy to a staging texture
		const uint32 latency = 1;
		ID3D11Texture2D* lastTarget = m_DepthReductionTargets.back().Texture;
		context->CopyResource(m_ReductionStagingTextures[m_currFrame % latency].Texture, lastTarget);

		++m_currFrame;

		if (m_currFrame >= latency)
		{
			StagingTexture2D& stagingTexture = m_ReductionStagingTextures[m_currFrame % latency];

			uint32 pitch;
			const uint16* texData = reinterpret_cast<uint16*>(stagingTexture.Map(context, 0, pitch));
			m_reductionDepth.x = texData[0] / static_cast<float>(0xffff);
			m_reductionDepth.y = texData[1] / static_cast<float>(0xffff);

			stagingTexture.Unmap(context, 0);
		}
		else
		{
			m_reductionDepth = DirectX::SimpleMath::Vector2(0.0f, 1.0f);
		}
	}

	void BackBuffer::DetectComplexPixels(ID3D11DeviceContext* context)
	{
		if (m_SurfaceDesc.SampleDesc.Count > 1)
		{
			m_MSAAConstants.Data.iMSAA_Detect_fSampleCount = m_SurfaceDesc.SampleDesc.Count;
			m_MSAAConstants.Data.fMSAA_Detect_fDepthEpsilonPercent = 0.0001f;
			m_MSAAConstants.ApplyChanges(context);
			m_MSAAConstants.SetPS(context, 1);

			// Resources for this render
			ID3D11ShaderResourceView*	SRV_Array[1] = { m_DepthStencil.SRView };
			ID3D11RenderTargetView*		RTV_Array[1] = { nullptr };

			context->OMSetRenderTargets(ARRAYSIZE(RTV_Array), RTV_Array, m_DepthStencil.ReadOnlyDSView);
			context->VSSetShader(m_ScreenTriangleVShader, nullptr, 0);
			context->PSSetShader(m_ResolveDepthPShader, nullptr, 0);
			context->PSSetShaderResources(0, ARRAYSIZE(SRV_Array), SRV_Array);
			context->Draw(3, 0);
		}
	}

	void BackBuffer::CopyLightShaftBuffer(ID3D11DeviceContext* context)
	{
		//if (m_SurfaceDesc.SampleDesc.Count > 1)
		//	context->ResolveSubresource(m_LightShaftsBuffer.Texture, 0, m_BackBuffer.Texture, 0, m_BackBufferFormat);
		//else
		//	context->CopyResource(m_LightShaftsBuffer.Texture, m_BackBuffer.Texture);

		// Resources for this render
		ID3D11ShaderResourceView*	SRV_Array[1] = { m_BackBuffer.SRView };
		ID3D11RenderTargetView*		RTV_Array[1] = { m_LightShaftsBuffer.RTView };

		// Set the viewport
		D3D11_VIEWPORT viewport;
		viewport.Width = static_cast<float>((m_SurfaceDesc.Width + 1) / 2);
		viewport.Height = static_cast<float>((m_SurfaceDesc.Height + 1) / 2);
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		context->RSSetViewports(1, &viewport);

		context->IASetInputLayout(nullptr);
		context->PSSetShaderResources(0, ARRAYSIZE(SRV_Array), SRV_Array);
		context->OMSetRenderTargets(ARRAYSIZE(RTV_Array), RTV_Array, nullptr);
		context->VSSetShader(m_ScreenTriangleVShader, nullptr, 0);
		context->PSSetShader(m_CopyLightTexturePShader, nullptr, 0);
		context->Draw(3, 0);

		ID3D11RenderTargetView* rtv = m_BackBuffer.RTView;
		ID3D11ShaderResourceView* srv = nullptr;
		viewport.Width = static_cast<float>(m_SurfaceDesc.Width);
		viewport.Height = static_cast<float>(m_SurfaceDesc.Height);
		context->RSSetViewports(1, &viewport);
		context->PSSetShaderResources(0, 1, &srv);
		context->OMSetRenderTargets(1, &rtv, m_DepthStencil.DSView);
	}

	void BackBuffer::Update(float fElapsedTime)
	{
		if (m_Bloom)
		{
			m_BloomPostProcess->FrameMove(fElapsedTime);
		}
	}

	void BackBuffer::Clear(ID3D11DeviceContext* context)
	{
		context->ClearRenderTargetView(m_BackBuffer.RTView, DirectX::Colors::Black);
		context->ClearDepthStencilView(m_DepthStencil.DSView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0);
		ID3D11RenderTargetView* rtv = m_BackBuffer.RTView;
		context->OMSetRenderTargets(1, &rtv, m_DepthStencil.DSView);

		// Set the viewports
		D3D11_VIEWPORT viewport;
		viewport.Width = static_cast<float>(m_SurfaceDesc.Width);
		viewport.Height = static_cast<float>(m_SurfaceDesc.Height);
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		context->RSSetViewports(1, &viewport);
	}

	void BackBuffer::Begin(ID3D11DeviceContext* context)
	{
		if (m_LightShafts)
		{
			CopyLightShaftBuffer(context);
		}

		//pImmediateContext->ClearRenderTargetView(m_BackBuffer.RTView, DirectX::Colors::Black);
		//pImmediateContext->ClearDepthStencilView(m_DepthStencil.DSView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0);
		ID3D11RenderTargetView* rtv = m_BackBuffer.RTView;
		context->OMSetRenderTargets(1, &rtv, m_DepthStencil.DSView);

		// Set the viewports
		D3D11_VIEWPORT viewport;
		viewport.Width = static_cast<float>(m_SurfaceDesc.Width);
		viewport.Height = static_cast<float>(m_SurfaceDesc.Height);
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		context->RSSetViewports(1, &viewport);
	}

	void BackBuffer::End(ID3D11DeviceContext* context, ID3D11RenderTargetView* rtv)
	{
		context->VSSetShader(nullptr, nullptr, 0);
		context->GSSetShader(nullptr, nullptr, 0);
		context->HSSetShader(nullptr, nullptr, 0);
		context->DSSetShader(nullptr, nullptr, 0);
		context->PSSetShader(nullptr, nullptr, 0);
		context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		ID3D11ShaderResourceView* ppSRVNULL[] = { nullptr, nullptr, nullptr, nullptr, nullptr };
		context->PSSetShaderResources(0, ARRAYSIZE(ppSRVNULL), ppSRVNULL);

		context->VSSetShader(m_ScreenTriangleVShader, nullptr, 0);
		m_PixelConstants.SetPS(context, 0);
		context->IASetInputLayout(nullptr);

		uint32 outIndex = 0;
		if (m_LightShafts)
		{
			DEBUGTIMER(GPU_TIME_LIGHTSHAFTS_PASS);

			ID3D11ShaderResourceView* pSRViews[] = { m_BackBuffer.SRView, m_LightShaftsBuffer.SRView };
			DrawFullScreen(context, 2, pSRViews, 
				m_NonMSAABackBuffer[outIndex].RTView, 
				(m_SurfaceDesc.SampleDesc.Count > 1) ? m_LightShaftsMSPShader : m_LightShaftsPShader);
		}
		else
		{
			// Draw to Multisample BackBuffer
			if (m_SurfaceDesc.SampleDesc.Count > 1)
				context->ResolveSubresource(m_NonMSAABackBuffer[outIndex].Texture, 0, m_BackBuffer.Texture, 0, m_SurfaceDesc.Format);
			else
				context->CopyResource(m_NonMSAABackBuffer[outIndex].Texture, m_BackBuffer.Texture);
		}

		if (m_DepthOfField)
		{
			DEBUGTIMER(GPU_TIME_DOF_PASS);

			m_DepthOfFieldPostProcess->Render(context,
				&m_NonMSAABackBuffer[outIndex], &m_NonMSAABackBuffer[!outIndex],
				m_pCamera->GetNearClip(), m_pCamera->GetFarClip(),
				(m_SurfaceDesc.SampleDesc.Count > 1) ? m_ResolvedDepth.SRView : m_DepthStencil.SRView);
		}

		if (m_Bloom)
		{
			DEBUGTIMER(GPU_TIME_BLOOM_PASS);

			m_BloomPostProcess->Render(context, 
				&m_NonMSAABackBuffer[outIndex], 
				&m_NonMSAABackBuffer[!outIndex]);

			outIndex = !outIndex;
		}

		if (m_Fxaa)
		{
			DEBUGTIMER(GPU_TIME_AA_PASS);

			m_FXAAPostProcess->Render(context,
				m_NonMSAABackBuffer[outIndex].SRView, rtv);
		}
		else
		{
			// Resources for this render
			ID3D11ShaderResourceView*	SRV_Array[1] = { m_NonMSAABackBuffer[outIndex].SRView };
			ID3D11RenderTargetView*		RTV_Array[1] = { rtv };

			// Set the viewport
			D3D11_VIEWPORT viewport;
			viewport.Width = static_cast<float>(m_SurfaceDesc.Width);
			viewport.Height = static_cast<float>(m_SurfaceDesc.Height);
			viewport.TopLeftX = 0;
			viewport.TopLeftY = 0;
			viewport.MinDepth = 0.0f;
			viewport.MaxDepth = 1.0f;
			context->RSSetViewports(1, &viewport);

			context->OMSetRenderTargets(ARRAYSIZE(RTV_Array), RTV_Array, nullptr);
			context->VSSetShader(m_ScreenTriangleVShader, nullptr, 0);
			context->PSSetShader(m_CopyPShader, nullptr, 0);
			context->PSSetShaderResources(0, ARRAYSIZE(SRV_Array), SRV_Array);
			context->Draw(3, 0);
		}
	}
}