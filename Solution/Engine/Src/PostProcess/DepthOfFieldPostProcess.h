#pragma once

namespace Rath
{
	class BackBuffer::DepthOfFieldPostProcess
	{
	private:
		static const UINT GridSize = 450;

		BackBuffer& m_parent;
	protected:
		struct DOF_CB_STRUCT
		{
			DirectX::SimpleMath::Vector4 ProjectionAB;
			DirectX::SimpleMath::Vector4 DOFDepths;
		};
		ConstantBuffer<DOF_CB_STRUCT>	m_DofConstants;

		RenderTarget2D					m_DepthBlurred;
		RenderTarget2D					m_DepthTemp;

		ID3D11PixelShaderPtr		m_DepthBlurGenerationPShader;
		ID3D11ComputeShaderPtr		m_cocSpreadHCShader;
		ID3D11ComputeShaderPtr		m_cocSpreadVCShader;
		ID3D11ComputeShaderPtr		m_dofHCShader;
		ID3D11ComputeShaderPtr		m_dofVCShader;

		void DepthBlurGeneration(ID3D11DeviceContext* context, ID3D11ShaderResourceView* depth);
		void CoCSpread(ID3D11DeviceContext* context);
		void DOFComputeShader(ID3D11DeviceContext* context, ID3D11ShaderResourceView* inputSRV, ID3D11ShaderResourceView* outputSRV,
			ID3D11UnorderedAccessView* inutUAV, ID3D11UnorderedAccessView* outputUAV);
	public:
		DepthOfFieldPostProcess(BackBuffer& x) : m_parent(x) {};

		HRESULT CreateDevice(ID3D11Device* device);
		HRESULT CreateResources(ID3D11Device* device);
		void	Render(ID3D11DeviceContext* context, RenderTarget2D* input, RenderTarget2D* output, float fNear, float fFar, ID3D11ShaderResourceView* depthSRV);
	};
}
