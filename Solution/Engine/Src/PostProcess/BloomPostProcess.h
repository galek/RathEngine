#pragma once

namespace Rath
{
	class BackBuffer::BloomPostProcess
	{
	private:
		static const uint32 LumMapSize = 1024;
		struct LUM_CB_STRUCT
		{
			float BloomThreshold;
			float BloomMagnitude;
			float BloomBlurSigma;
			float Tau;
			float TimeDelta;
			float KeyValue;
			float MaxAdapt;
			float MinAdapt;
		};
		BackBuffer& m_parent;
	protected:
		ConstantBuffer<LUM_CB_STRUCT>	m_LumenConstants;

		RenderTarget2D			m_LuminanceMap;
		RenderTarget2D			m_AdaptLuminance[2];
		RenderTarget2D			m_BloomSource;
		RenderTarget2D			m_DownScale[4];

		ID3D11PixelShaderPtr	m_LuminanceMapPShader;
		ID3D11PixelShaderPtr	m_AdaptLuminancePShader;

		ID3D11PixelShaderPtr	m_ThresholdPShader;
		ID3D11PixelShaderPtr	m_ScalePShader;

		ID3D11PixelShaderPtr	m_BloomBlurHPShader;
		ID3D11PixelShaderPtr	m_BloomBlurVPShader;

		ID3D11PixelShaderPtr	m_CompositePShader;

		uint64					m_CurrLumTarget;

		void CalcAvgLuminance(ID3D11DeviceContext* context, ID3D11ShaderResourceView* input);
		void Bloom(ID3D11DeviceContext* context, ID3D11ShaderResourceView* input);
		void ToneMap(ID3D11DeviceContext* context, ID3D11ShaderResourceView* input, ID3D11RenderTargetView* output);
	public:
		BloomPostProcess(BackBuffer& x) : m_parent(x) {};

		HRESULT CreateDevice(ID3D11Device* device);
		HRESULT CreateResources(ID3D11Device* device);
		void	FrameMove(float fElapsedTime);
		void	Render(ID3D11DeviceContext* context, RenderTarget2D* input, RenderTarget2D* output);
	};
}