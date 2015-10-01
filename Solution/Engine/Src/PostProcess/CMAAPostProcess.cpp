#include "pch.h"
#include "BackBuffer.h"
#include "CMAAPostProcess.h"

namespace Rath
{
	HRESULT BackBuffer::CMAAPostProcess::CreateDevice(ID3D11Device* device)
	{
		HRESULT hr = S_OK;

#define MakePath(string) SHADERFOLDER string ".cso"

		hr = Technique::LoadShader(MakePath("FXAA_ps"), device, &m_edges0PS);
		assert(hr == S_OK);
		hr = Technique::LoadShader(MakePath("FXAA_ps"), device, &m_edges1PS);
		assert(hr == S_OK);
		hr = Technique::LoadShader(MakePath("FXAA_ps"), device, &m_edgesCombinePS);
		assert(hr == S_OK);
		hr = Technique::LoadShader(MakePath("FXAA_ps"), device, &m_processAndApplyPS);
		assert(hr == S_OK);

		m_CmaaConstants.Initialize(device);

		return hr;
	}

	HRESULT BackBuffer::CMAAPostProcess::CreateResources(ID3D11Device* device)
	{
		m_depthStencilTex.Initialize(device,
			(m_parent.m_SurfaceDesc.Width + 1) / 2,
			(m_parent.m_SurfaceDesc.Height + 1) / 2,
			DXGI_FORMAT_R16_TYPELESS,
			true,
			1,
			0,
			1);

		for (uint64 i = 0; i < 2; i++)
			m_edgesTexture[i].Initialize(device,
			m_parent.m_SurfaceDesc.Width,
			m_parent.m_SurfaceDesc.Height,
			DXGI_FORMAT_R8_UNORM,
			1,
			1,
			0,
			false,
			true);

		m_mini4edgeTexture.Initialize(device,
			(m_parent.m_SurfaceDesc.Width + 1) / 2,
			(m_parent.m_SurfaceDesc.Height + 1) / 2,
			DXGI_FORMAT_R8G8B8A8_TYPELESS,
			1,
			1,
			0,
			false,
			true);

		m_workingColorTexture.Initialize(device,
			m_parent.m_SurfaceDesc.Width,
			m_parent.m_SurfaceDesc.Height,
			DXGI_FORMAT_R8G8B8A8_TYPELESS,
			1,
			1,
			0,
			false,
			true);

		return S_OK;
	}

	void BackBuffer::CMAAPostProcess::Render(ID3D11DeviceContext* context, ID3D11ShaderResourceView* input, ID3D11RenderTargetView* output)
	{
		D3D11_VIEWPORT vpOld;
		// backup original viewport
		UINT numViewports = 1;
		context->RSGetViewports(&numViewports, &vpOld);

		// Backup current render/stencil
		ID3D11RenderTargetView * oldRenderTargetViews[4] = { NULL };
		ID3D11DepthStencilView * oldDepthStencilView = NULL;
		context->OMGetRenderTargets(_countof(oldRenderTargetViews), oldRenderTargetViews, &oldDepthStencilView);

		ID3D11RenderTargetView *pView[4] = { NULL, NULL, NULL, NULL };
		context->OMSetRenderTargets(4, pView, NULL);
		// Clear the shader resources to avoid a hazard warning
		ID3D11ShaderResourceView *pNullResources[16] = { 0 };
		context->PSSetShaderResources(0, 16, pNullResources);
		context->VSSetShaderResources(0, 16, pNullResources);

		m_frameID++;

		ID3D11ShaderResourceView *  edgesTextureA_SRV;
		ID3D11UnorderedAccessView * edgesTextureA_UAV;
		ID3D11Texture2D *           edgesTextureA;
		ID3D11ShaderResourceView *  edgesTextureB_SRV;
		ID3D11UnorderedAccessView * edgesTextureB_UAV;
		ID3D11Texture2D *           edgesTextureB;
		// flip flop - one pass clears the texture that needs clearing for the other one (actually it's only important that it clears the highest bit)
		if ((m_frameID % 2) == 0)
		{
			edgesTextureA = m_edgesTexture[0].Texture;
			edgesTextureA_SRV = m_edgesTexture[0].SRView;
			edgesTextureA_UAV = m_edgesTexture[0].UAView;
			edgesTextureB = m_edgesTexture[1].Texture;
			edgesTextureB_SRV = m_edgesTexture[1].SRView;
			edgesTextureB_UAV = m_edgesTexture[1].UAView;
		}
		else
		{
			edgesTextureA = m_edgesTexture[1].Texture;
			edgesTextureA_SRV = m_edgesTexture[1].SRView;
			edgesTextureA_UAV = m_edgesTexture[1].UAView;
			edgesTextureB = m_edgesTexture[0].Texture;
			edgesTextureB_SRV = m_edgesTexture[0].SRView;
			edgesTextureB_UAV = m_edgesTexture[0].UAView;
		}

		D3D11_VIEWPORT viewport, viewportHalfHalf;

		{
			// Setup the viewport to match the backbuffer
			viewport.TopLeftX = 0.0f;
			viewport.TopLeftY = 0.0f;
			viewport.Width = (float)m_parent.m_SurfaceDesc.Width;
			viewport.Height = (float)m_parent.m_SurfaceDesc.Height;
			viewport.MinDepth = 0;
			viewport.MaxDepth = 1;

			viewportHalfHalf = viewport;
			assert(((((int)viewport.TopLeftX) % 2) == 0) && ((((int)viewport.TopLeftY) % 2) == 0));
			viewportHalfHalf.TopLeftX = viewport.TopLeftX / 2;
			viewportHalfHalf.TopLeftY = viewport.TopLeftY / 2;
			viewportHalfHalf.Width = (float)((int(viewport.Width) + 1) / 2);
			viewportHalfHalf.Height = (float)((int(viewport.Height) + 1) / 2);
		}

		// Setup constants
		//{
		//	m_CmaaConstants.Data.LumWeights[0] = 0.2126f;
		//	m_CmaaConstants.Data.LumWeights[1] = 0.7152f;
		//	m_CmaaConstants.Data.LumWeights[2] = 0.0722f;
		//	m_CmaaConstants.Data.LumWeights[3] = 0.0000f;

		//	m_CmaaConstants.Data.ColorThreshold = PPAADEMO_gEdgeDetectionThreshold;             // 1.0/13.0
		//	m_CmaaConstants.Data.DepthThreshold = 0.07f;
		//	m_CmaaConstants.Data.NonDominantEdgeRemovalAmount = clamp(PPAADEMO_gNonDominantEdgeRemovalAmount, 0.05f, 0.95f);       // 0.35
		//	m_CmaaConstants.Data.Dummy0 = 0.0;

		//	m_CmaaConstants.Data.OneOverScreenSize[0] = 1.0f / (float)m_parent.m_SurfaceDesc.Width;
		//	m_CmaaConstants.Data.OneOverScreenSize[1] = 1.0f / (float)m_parent.m_SurfaceDesc.Height;
		//	m_CmaaConstants.Data.ScreenWidth = (int)m_parent.m_SurfaceDesc.Width;
		//	m_CmaaConstants.Data.ScreenHeight = (int)m_parent.m_SurfaceDesc.Height;

		//	m_CmaaConstants.Data.DebugZoomTool[0] = 0.0f;
		//	m_CmaaConstants.Data.DebugZoomTool[1] = 0.0f;
		//	m_CmaaConstants.Data.DebugZoomTool[2] = 0.0f;
		//	m_CmaaConstants.Data.DebugZoomTool[3] = 0.0f;

		//	m_CmaaConstants.ApplyChanges(context);
		//	m_CmaaConstants.SetPS(context, 4);
		//	m_CmaaConstants.SetCS(context, 4);
		//}

		//// set adequate viewport
		//context->RSSetViewports(1, &viewportHalfHalf);

		//UINT UAVInitialCounts[] = { (UINT)-1, (UINT)-1, (UINT)-1, (UINT)-1 };

		//// no need to clear DepthStencil; cleared by the next fullscreen pass - not sure if this is correct for Hi-Z but seems to work well
		//// on all tested hardware (AMD 5xxx, 7xxx; NVidia 4XX, 6XX; Intel IvyBridge/Haswell) 
		////context->ClearDepthStencilView( m_depthStencilTexDSV, D3D11_CLEAR_DEPTH, 1.0f, 0 );

		//// Detect edges Pass 0
		////   - for every pixel detect edges to the right and down and output depth mask where edges detected (1 - far, for detected, 0-near for empty pixels)
		//{
		//	//context->OMSetRenderTargets( 1, &m_mini4edgeTextureUintRTV, m_depthStencilTexDSV );
		//	ID3D11UnorderedAccessView * UAVs[] = { m_workingColorTexture.UAView };
		//	context->OMSetRenderTargetsAndUnorderedAccessViews(1, &m_mini4edgeTexture.RTView, m_depthStencilTex.DSView, 1, _countof(UAVs), UAVs, UAVInitialCounts);

		//	context->PSSetShaderResources(0, 1, &sourceColorSRV_UNORM);
		//	m_parent.DrawFullScreen(context, m_edges0PS, NULL, MySample::GetBS_Opaque(), m_renderCreateMaskDSS, 0, 1.0f);
		//	context->PSSetShaderResources(0, 1, nullSRVs);
		//}

		//// Detect edges Pass 1 (finish the previous pass edge processing).
		//// Do the culling of non-dominant local edges (leave mainly locally dominant edges) and merge Right and Bottom edges into TopRightBottomLeft
		//{
		//	ID3D11UnorderedAccessView * UAVs[] = { edgesTextureB_UAV };
		//	context->OMSetRenderTargetsAndUnorderedAccessViews(0, NULL, m_depthStencilTex.ReadOnlyDSView, 0, _countof(UAVs), UAVs, UAVInitialCounts);
		//	context->PSSetShaderResources(3, 1, &m_mini4edgeTexture.SRView);

		//	m_parent.DrawFullScreen(context, m_edges1PS, NULL, MySample::GetBS_Opaque(), m_renderUseMaskDSS, 0, 0.0f);
		//	context->PSSetShaderResources(3, 1, nullSRVs);
		//	context->PSSetShaderResources(0, 1, nullSRVs);
		//}

		////  - Combine RightBottom (.xy) edges from previous pass into RightBottomLeftTop (.xyzw) edges and output it into the mask (have to fill in the whole buffer
		////    including empty ones for the line length detection to work correctly). 
		////  - On all pixels with any edge, input buffer into a temporary color buffer needed for correct blending in the next pass (other pixels not needed
		////    so not copied to avoid bandwidth use)
		////  - On all pixels with 2 or more edges output positive depth mask for the next pass 
		//{
		//	// Combine edges: each pixel will now contain info on all (top, right, bottom, left) edges; also create depth mask as above depth and mark potential Zs
		//	// AND also copy source color data but only on edge pixels
		//	ID3D11UnorderedAccessView * UAVs[] = { destColorUAV, edgesTextureA_UAV };
		//	context->OMSetRenderTargetsAndUnorderedAccessViews(0, NULL, m_depthStencilTex.DSView, 1, _countof(UAVs), UAVs, UAVInitialCounts);

		//	context->PSSetShaderResources(0, 1, &m_workingColorTexture.SRView);
		//	context->PSSetShaderResources(3, 1, &edgesTextureB_SRV);
		//	m_parent.DrawFullScreen(context, m_edgesCombinePS, NULL, MySample::GetBS_Opaque(), m_renderCreateMaskDSS, 0, 1.0f);
		//	context->PSSetShaderResources(0, 1, nullSRVs);
		//	context->PSSetShaderResources(3, 1, nullSRVs);
		//}

		// set full res viewport
		context->RSSetViewports(1, &viewport);

		// Restore original render/depthstencil
		context->OMSetRenderTargets(_countof(oldRenderTargetViews), oldRenderTargetViews, oldDepthStencilView);
		for (int i = 0; i < _countof(oldRenderTargetViews); i++)
			SAFE_RELEASE(oldRenderTargetViews[i]);
		SAFE_RELEASE(oldDepthStencilView);

		// Restore original viewport
		context->RSSetViewports(1, &vpOld);
	}
}