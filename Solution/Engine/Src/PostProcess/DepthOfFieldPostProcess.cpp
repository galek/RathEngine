#include "pch.h"
#include "BackBuffer.h"
#include "DepthOfFieldPostProcess.h"

namespace Rath
{
	HRESULT BackBuffer::DepthOfFieldPostProcess::CreateResources(ID3D11Device* device)
	{
		m_DepthBlurred.Initialize(device, m_parent.m_SurfaceDesc.Width, m_parent.m_SurfaceDesc.Height, DXGI_FORMAT_R16G16_FLOAT, 1, 1, 0, false, true, 1, false);
		m_DepthTemp.Initialize(device, m_parent.m_SurfaceDesc.Width, m_parent.m_SurfaceDesc.Height, DXGI_FORMAT_R16G16_FLOAT, 1, 1, 0, false, true, 1, false);

		return S_OK;
	}

	HRESULT BackBuffer::DepthOfFieldPostProcess::CreateDevice(ID3D11Device* device)
	{
		HRESULT hr = S_OK;

#define MakePath(string) SHADERFOLDER string ".cso"

		hr = Technique::LoadShader(MakePath("DepthBlurGeneration_ps"), device, &m_DepthBlurGenerationPShader);
		assert(hr == S_OK);
		hr = Technique::LoadShader(MakePath("CoCSpread_H_cs"), device, &m_cocSpreadHCShader);
		assert(hr == S_OK);
		hr = Technique::LoadShader(MakePath("CoCSpread_V_cs"), device, &m_cocSpreadVCShader);
		assert(hr == S_OK);
		hr = Technique::LoadShader(MakePath("DOF_H_cs"), device, &m_dofHCShader);
		assert(hr == S_OK);
		hr = Technique::LoadShader(MakePath("DOF_V_cs"), device, &m_dofVCShader);
		assert(hr == S_OK);

		m_DofConstants.Initialize(device);

		return hr;
	}

	void BackBuffer::DepthOfFieldPostProcess::DepthBlurGeneration(ID3D11DeviceContext* context, ID3D11ShaderResourceView* depth)
	{
		ID3D11ShaderResourceView* pSRView[] = { depth };
		m_parent.DrawFullScreen(context, 1, pSRView, m_DepthBlurred.RTView, m_DepthBlurGenerationPShader);

		ID3D11RenderTargetView* pnullView[] = { nullptr };
		context->OMSetRenderTargets(1, pnullView, nullptr);
	}

	void BackBuffer::DepthOfFieldPostProcess::CoCSpread(ID3D11DeviceContext* context)
	{
		// Horizontal pass
		UINT groupCountX = m_parent.m_SurfaceDesc.Width / GridSize;
		groupCountX += (m_parent.m_SurfaceDesc.Width % GridSize) > 0 ? 1 : 0;
		UINT groupCountY = m_parent.m_SurfaceDesc.Height;
		ID3D11ShaderResourceView* srViews[] = { m_DepthBlurred.SRView };
		ID3D11UnorderedAccessView* uaViews[] = { m_DepthTemp.UAView };
		context->CSSetShaderResources(0, 1, srViews);
		context->CSSetUnorderedAccessViews(0, 1, uaViews, nullptr);
		context->CSSetShader(m_cocSpreadHCShader, nullptr, 0);
		context->Dispatch(groupCountX, groupCountY, 1);

		srViews[0] = nullptr;
		uaViews[0] = nullptr;
		context->CSSetShaderResources(0, 1, srViews);
		context->CSSetUnorderedAccessViews(0, 1, uaViews, nullptr);


		// Vertical pass
		groupCountX = m_parent.m_SurfaceDesc.Width;
		groupCountY = m_parent.m_SurfaceDesc.Height / GridSize;
		groupCountY += (m_parent.m_SurfaceDesc.Height % GridSize) > 0 ? 1 : 0;
		uaViews[0] = m_DepthBlurred.UAView;
		srViews[0] = m_DepthTemp.SRView;
		context->CSSetUnorderedAccessViews(0, 1, uaViews, nullptr);
		context->CSSetShaderResources(0, 1, srViews);
		context->CSSetShader(m_cocSpreadVCShader, nullptr, 0);
		context->Dispatch(groupCountX, groupCountY, 1);

		srViews[0] = nullptr;
		uaViews[0] = nullptr;
		context->CSSetShaderResources(0, 1, srViews);
		context->CSSetUnorderedAccessViews(0, 1, uaViews, nullptr);
	}

	void BackBuffer::DepthOfFieldPostProcess::DOFComputeShader(ID3D11DeviceContext* context, 
		ID3D11ShaderResourceView* inputSRV, ID3D11ShaderResourceView* outputSRV,
		ID3D11UnorderedAccessView* inutUAV, ID3D11UnorderedAccessView* outputUAV)
	{
		// Horizontal pass
		UINT groupCountX = m_parent.m_SurfaceDesc.Width / GridSize;
		groupCountX += (m_parent.m_SurfaceDesc.Width % GridSize) > 0 ? 1 : 0;
		UINT groupCountY = m_parent.m_SurfaceDesc.Height;
		ID3D11ShaderResourceView* srViews[] = { inputSRV, m_DepthBlurred.SRView };
		ID3D11UnorderedAccessView* uaViews[] = { outputUAV };
		context->CSSetShaderResources(0, 2, srViews);
		context->CSSetUnorderedAccessViews(0, 1, uaViews, nullptr);
		context->CSSetShader(m_dofHCShader, nullptr, 0);
		context->Dispatch(groupCountX, groupCountY, 1);

		uaViews[0] = nullptr;
		context->CSSetUnorderedAccessViews(0, 1, uaViews, nullptr);

		// Vertical pass
		groupCountX = m_parent.m_SurfaceDesc.Width;
		groupCountY = m_parent.m_SurfaceDesc.Height / GridSize;
		groupCountY += (m_parent.m_SurfaceDesc.Height % GridSize) > 0 ? 1 : 0;
		srViews[0] = outputSRV;
		srViews[1] = m_DepthBlurred.SRView;
		uaViews[0] = inutUAV;
		context->CSSetShaderResources(0, 2, srViews);
		context->CSSetUnorderedAccessViews(0, 1, uaViews, nullptr);
		context->CSSetShader(m_dofVCShader, nullptr, 0);
		context->Dispatch(groupCountX, groupCountY, 1);

		srViews[0] = nullptr;
		srViews[1] = nullptr;
		uaViews[0] = nullptr;
		context->CSSetShaderResources(0, 2, srViews);
		context->CSSetUnorderedAccessViews(0, 1, uaViews, nullptr);
	}

	void BackBuffer::DepthOfFieldPostProcess::Render(ID3D11DeviceContext* context, 
		RenderTarget2D* input, RenderTarget2D* output, 
		float fNear, float fFar, 
		ID3D11ShaderResourceView* depthSRV)
	{
		m_DofConstants.Data.DOFDepths.x = 0.1f;
		m_DofConstants.Data.DOFDepths.y = 1.5f;
		m_DofConstants.Data.DOFDepths.z = 50.0f;
		m_DofConstants.Data.DOFDepths.w = 200.0f;
		m_DofConstants.Data.ProjectionAB.x = fFar / (fFar - fNear);
		m_DofConstants.Data.ProjectionAB.y = (-fFar * fNear) / (fFar - fNear);
		m_DofConstants.ApplyChanges(context);
		m_DofConstants.SetPS(context, 1);

		DepthBlurGeneration(context, depthSRV);
		CoCSpread(context);
		DOFComputeShader(context, input->SRView, output->SRView, input->UAView, output->UAView);
	}

}