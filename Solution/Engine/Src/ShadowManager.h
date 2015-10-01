#pragma once

#define TEST_EXTERNAL_MAPS
#define __GFSDK_DX11__
#include "gfsdk_shadowlib.h"
#include "GFSDK_SSAO.h"

#include "GraphicTypes.h"
#include "EngineState.h"

namespace Rath
{
	class ShadowManager
	{
	protected:
		typedef std::function<void(ID3D11DeviceContext*, const DirectX::XMMATRIX&)> BoundDepthOnlyRenderFunction;

		struct GLOBAL_CB_STRUCT
		{
			DirectX::SimpleMath::Matrix  mView;                          // View matrix
			DirectX::SimpleMath::Matrix  mProjection;                    // Projection matrix
			DirectX::SimpleMath::Matrix  mViewProjection;                // VP matrix
			DirectX::SimpleMath::Matrix  mInvView;                       // Inverse of view matrix
			DirectX::SimpleMath::Matrix  mCenterViewProjection;          // Centered VP matrix

			DirectX::SimpleMath::Vector4 vEye;							// Camera's location
			DirectX::SimpleMath::Vector4 vDirection;
			DirectX::SimpleMath::Vector4 vLightDiffuse;					// Light's diffuse color
			DirectX::SimpleMath::Vector4 vLightAmbient;					// Light's ambient color

			DirectX::SimpleMath::Vector4 vViewport;
			DirectX::SimpleMath::Vector4 vViewFrustumPlanes[4];			// View frustum planes ( x=left, y=right, z=top, w=bottom )
			DirectX::SimpleMath::Vector4 vCenterViewFrustumPlanes[4];	// View frustum planes ( x=left, y=right, z=top, w=bottom )
			DirectX::SimpleMath::Vector4 vTessellationFactor;

			void BindViewAndProj(const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj);
		};
	protected:
#define MAX_CASCADES 4
		DirectX::SimpleMath::Vector3		m_vSceneAABBMin;
		DirectX::SimpleMath::Vector3		m_vSceneAABBMax;
		DirectX::SimpleMath::Matrix			m_matShadowProj[MAX_CASCADES];
		DirectX::SimpleMath::Matrix			m_matShadowView;

		INT                                 m_iCascadePartitionsMax;
		FLOAT                               m_fCascadePartitionsFrustum[MAX_CASCADES]; // Values are  between near and far
		INT                                 m_iCascadePartitionsZeroToOne[MAX_CASCADES]; // Values are 0 to 100 and represent a percent of the frstum

		D3D11_VIEWPORT                      m_RenderVP[MAX_CASCADES];
		DepthStencilBuffer                  m_CascadedShadowMap;
		//---------------------------------
		typedef GFSDK_ShadowLib_Status(__cdecl * GetDLLVersion)(
			GFSDK_ShadowLib_Version* __GFSDK_RESTRICT__ const pVersion);

		typedef GFSDK_ShadowLib_Status(__cdecl * Create)(
			const GFSDK_ShadowLib_Version* __GFSDK_RESTRICT__ const			pVersion,
			GFSDK_ShadowLib_Context** __GFSDK_RESTRICT__ const				ppContext,
			const GFSDK_ShadowLib_DeviceContext* __GFSDK_RESTRICT__ const	pPlatformDevice,
			gfsdk_new_delete_t*												customAllocator);

		GFSDK_ShadowLib_Version				m_ShadowLibVersion;
		GFSDK_ShadowLib_Context*			m_pShadowLibCtx;
		GFSDK_ShadowLib_DeviceContext		m_DeviceAndContext;

		GFSDK_ShadowLib_BufferRenderParams	m_SBRenderParams;
		GFSDK_ShadowLib_BufferDesc			m_SBDesc;
		GFSDK_ShadowLib_Buffer*				m_pShadowBufferHandle;
		GFSDK_ShadowLib_ShaderResourceView	m_ShadowBufferSRV;
		ID3D11RenderTargetViewPtr			m_ShadowBufferRTV;

		GFSDK_ShadowLib_ExternalMapDesc		m_ESMDesc;

		GFSDK_ShadowLib_MapRenderParams		m_SMRenderParams;
		GFSDK_ShadowLib_MapDesc				m_SMDesc;
		GFSDK_ShadowLib_Map*				m_pShadowMapHandle;

		GFSDK_ShadowLib_ShaderResourceView	m_ShadowMapSRV;

		GFSDK_ShadowLib_TempResources		m_TempResources;
		GFSDK_ShadowLib_Texture2D			m_DownsampledShadowMap;

		unsigned int						m_ShadowMapScale;
		unsigned int						m_ShadowMapWidth;
		unsigned int						m_ShadowMapHeight;
		GFSDK_ShadowLib_ViewLocationDesc	m_ViewLocation[GFSDK_ShadowLib_ViewType_Cascades_4];

		GFSDK_SSAO_Parameters_D3D11			m_AOParams;
		GFSDK_SSAO_Context_D3D11*			m_AOContext;
		bool								m_AmbientOcclusion;

		ConstantBuffer<GLOBAL_CB_STRUCT>	m_ConstantBufferFrame;
		BoundDepthOnlyRenderFunction		m_DepthOnlyRenderFunction;

		void* m_GetDLLVersion_Proc;
		void* m_Create_Proc;

		static void ShadowMapRenderFunction(void* pParams, gfsdk_float4x4* pViewProj);

		bool	FindDLL(const char *filePath, std::string &fullPath);
		void	LoadDLL();

		const Camera*	m_pCamera;
		const Camera*	m_pLight;
	public:
		ShadowManager(EngineState* state, BoundDepthOnlyRenderFunction renderFunction);
		~ShadowManager();

		void CreateDevice(ID3D11Device* device, ID3D11DeviceContext* context);
		void CreateResources(ID3D11Device* device, const DXGI_SURFACE_DESC& BackBufferDesc, ID3D11DepthStencilView* pReadOnlyDSV);

		void ModulateShadowBuffer(ID3D11RenderTargetView* pOutputRTV);

		void DisplayShadowBuffer(ID3D11RenderTargetView* pOutputRTV);
		void DisplayShadowMaps(ID3D11RenderTargetView* pOutputRTV, UINT Width, UINT Height);
		void DisplayMapFrustums(ID3D11RenderTargetView* pOutputRTV, ID3D11DepthStencilView* pDSV);


		void RenderShadowMaps(ID3D11DeviceContext* context);
		void RenderShadowBuffer(ID3D11ShaderResourceView* pDSSRV, ID3D11ShaderResourceView* pDSResolvedSRV);

		ID3D11ShaderResourceView*	GetShadowBuffer() { return m_ShadowBufferSRV.pSRV; };
		ID3D11RenderTargetView*		GetShadowBufferRTV() { return m_ShadowBufferRTV; };
	};
}
