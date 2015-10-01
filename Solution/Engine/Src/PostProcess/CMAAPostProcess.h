#pragma once

namespace Rath
{
	class BackBuffer::CMAAPostProcess
	{
	private:
		struct CMAA_CB_STRUCT
		{
			float           LumWeights[4];          // .rgb - luminance weight for each colour channel; .w unused for now (maybe will be used for gamma correction before edge detect)

			float           ColorThreshold;                     // for simple edge detection
			float           DepthThreshold;                     // for depth (unused at the moment)
			float           NonDominantEdgeRemovalAmount;       // how much non-dominant edges to remove
			float           Dummy0;

			float           OneOverScreenSize[2];
			int             ScreenWidth;
			int             ScreenHeight;

			float           DebugZoomTool[4];
		};
		BackBuffer& m_parent;
	protected:
		ConstantBuffer<CMAA_CB_STRUCT>		m_CmaaConstants;
		DepthStencilBuffer					m_depthStencilTex;
		RWTexture							m_edgesTexture[2];
		RenderTarget2D						m_mini4edgeTexture;
		RWTexture							m_workingColorTexture;

		ID3D11PixelShaderPtr                m_edges0PS;
		ID3D11PixelShaderPtr                m_edges1PS;
		ID3D11PixelShaderPtr                m_edgesCombinePS;
		ID3D11PixelShaderPtr                m_processAndApplyPS;

		ID3D11DepthStencilStatePtr          m_renderAlwaysDSS;
		ID3D11DepthStencilStatePtr          m_renderCreateMaskDSS;
		ID3D11DepthStencilStatePtr          m_renderUseMaskDSS;

		int                                 m_frameID;
	public:
		CMAAPostProcess(BackBuffer& x) : m_parent(x) {};

		HRESULT CreateDevice(ID3D11Device* device);
		HRESULT CreateResources(ID3D11Device* device);
		void	Render(ID3D11DeviceContext* context, ID3D11ShaderResourceView* input, ID3D11RenderTargetView* output);
	};
}
