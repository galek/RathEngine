#pragma once

#include "SimpleMath.h"
#include "Technique.h"
#include "GraphicTypes.h"
#include "EngineState.h"

namespace Rath
{
	class BackBuffer
	{
	private:
		static const uint32 MaxReadbackLatency = 4;
		static const uint32 ReductionTGSize = 16;
		static const uint32 CullTGSize = 128;
		static const uint32 BatchTGSize = 256;
		static const uint32 MaxInputs = 4;

		struct PS_CB_STRUCT
		{
			DirectX::SimpleMath::Vector2 InputSize[MaxInputs];
			DirectX::SimpleMath::Vector2 OutputSize;
		};

		struct MSAA_CB_STRUCT
		{
			float	fMSAA_Detect_fDepthEpsilonPercent;
			int		iMSAA_Detect_fSampleCount;
		};

		struct REDUCTION_CB_STRUCT
		{
			DirectX::SimpleMath::Matrix	Projection;
			float						NearClip;
			float						FarClip;
		};
	protected:
		ID3D11VertexShaderPtr			m_ScreenTriangleVShader;
		ID3D11PixelShaderPtr			m_CopyPShader;
		ID3D11PixelShaderPtr			m_MSAADetectPShader;
		ID3D11PixelShaderPtr			m_ResolveDepthPShader;
		ID3D11PixelShaderPtr			m_LightShaftsPShader;
		ID3D11PixelShaderPtr			m_LightShaftsMSPShader;
		ID3D11PixelShaderPtr			m_CopyLightTexturePShader;
		ID3D11ComputeShaderPtr			m_DdepthReductionInitialCShader;
		ID3D11ComputeShaderPtr			m_DepthReductionCShader;

		ConstantBuffer<PS_CB_STRUCT>						m_PixelConstants;
		ConstantBuffer<MSAA_CB_STRUCT>						m_MSAAConstants;
		ConstantBuffer<REDUCTION_CB_STRUCT>					m_ReductionConstants;

		RenderTarget2D										m_BackBuffer;
		RenderTarget2D										m_LightShaftsBuffer;
		RenderTarget2D										m_NonMSAABackBuffer[2];

		DepthStencilBuffer									m_DepthStencil;
		RenderTarget2D										m_ResolvedDepth;
		std::vector<RenderTarget2D>							m_DepthReductionTargets;
		StagingTexture2D									m_ReductionStagingTextures[MaxReadbackLatency];
		uint32												m_currFrame;
		DirectX::SimpleMath::Vector2						m_reductionDepth;
		DXGI_SURFACE_DESC									m_SurfaceDesc;

		bool												m_Bloom;
		bool												m_Fxaa;
		bool												m_DepthOfField;
		bool												m_LightShafts;

		void CreateShader(ID3D11Device* device);
		void CreateBackBuffers(ID3D11Device* device);
		void CreateReductionTargets(ID3D11Device* device);
		void CreateEffects(ID3D11Device* device);

		void CopyLightShaftBuffer(ID3D11DeviceContext* context);

		void DrawFullScreen(
			_In_ ID3D11DeviceContext* pContext,
			_In_range_(0, 4)  UINT NumInputs,
			_In_reads_opt_(NumSamplers)  ID3D11ShaderResourceView *const *ppInputs,
			_In_ ID3D11RenderTargetView* pOutput,
			_In_ ID3D11PixelShader* pPixelShader);

		class BloomPostProcess;
		class DepthOfFieldPostProcess;
		class FXAAPostProcess;
		class CMAAPostProcess;

		std::unique_ptr<BloomPostProcess>			m_BloomPostProcess;
		std::unique_ptr<DepthOfFieldPostProcess>	m_DepthOfFieldPostProcess;
		std::unique_ptr<FXAAPostProcess>			m_FXAAPostProcess;
		std::unique_ptr<CMAAPostProcess>			m_CMAAPostProcess;

		const Camera*	m_pCamera;
	public:
		BackBuffer(EngineState* state);
		~BackBuffer();

		void Update(float fElapsedTime);

		void CreateDevice(ID3D11Device* device);
		void CreateResources(ID3D11Device* device, const DXGI_SURFACE_DESC& backBufferDesc);

		void ResolveDepth(ID3D11DeviceContext* context);
		void ReduceDepth(ID3D11DeviceContext* context);
		void DetectComplexPixels(ID3D11DeviceContext* context);

		void Clear(ID3D11DeviceContext* context);
		void Begin(ID3D11DeviceContext* context);
		void End(ID3D11DeviceContext* context, ID3D11RenderTargetView* rtv);

		ID3D11ShaderResourceView* GetDepthStencilView() { return m_DepthStencil.SRView; };
		ID3D11ShaderResourceView* GetResolvedDepthStencilView() { return m_ResolvedDepth.SRView; };
		ID3D11DepthStencilView*	  GetDepthStencilSRViewRO() { return m_DepthStencil.ReadOnlyDSView; };

		DirectX::SimpleMath::Vector2 GetResolvedDepth();
	};
};
