#include "pch.h"
#include "ShadowManager.h"

#include "Debug\TimestampQueries.h"

#ifdef _WIN64
#pragma comment( lib, "..\\GFSDK_ShadowLib\\lib\\GFSDK_ShadowLib_DX11.win64.lib" )
#pragma comment( lib, "..\\GFSDK_HBAO\\lib\\GFSDK_SSAO.win64.lib" )
#else
#pragma comment( lib, "..\\GFSDK_ShadowLib\\lib\\GFSDK_ShadowLib_DX11.win32.lib" )
#pragma comment( lib, "..\\GFSDK_HBAO\\lib\\GFSDK_SSAO.win32.lib" )
#endif

namespace Rath
{
	void ShadowManager::GLOBAL_CB_STRUCT::BindViewAndProj(const XMMATRIX& view, const XMMATRIX& proj)
	{
		XMMATRIX viewproj = view * proj;
		mView = XMMatrixTranspose(view);
		mProjection = XMMatrixTranspose(proj);
		mViewProjection = XMMatrixTranspose(viewproj);
		mInvView = XMMatrixInverse(nullptr, view);
		memcpy(&vEye, &mInvView.Translation(), sizeof(DirectX::SimpleMath::Vector3));
		vDirection = XMVector3Normalize(mInvView.Backward());
		mInvView = XMMatrixTranspose(mInvView);
		XMFRUSTUM frustum(viewproj);
		memcpy(&vViewFrustumPlanes[0], &frustum.mPlanes[0], sizeof(XMVECTOR) * 4);
		viewproj = view;
		viewproj.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		mCenterViewProjection = XMMatrixTranspose(viewproj * proj);
		XMFRUSTUM centerfrustum(viewproj);
		memcpy(&vCenterViewFrustumPlanes[0], &centerfrustum.mPlanes[0], sizeof(XMVECTOR) * 4);
	}

	void InitAOParams(GFSDK_SSAO_Parameters_D3D11 &AOParams)
	{
		AOParams = GFSDK_SSAO_Parameters_D3D11();
		AOParams.Radius = 1.0f;
		AOParams.Bias = 0.3f;
		AOParams.DetailAO = 0.f;
		AOParams.CoarseAO = 1.f;
		AOParams.PowerExponent = 5.f;
		AOParams.Blur.Sharpness = 4.f;
		AOParams.Blur.Enable = true;
#if ENABLE_BLUR_RADIUS
		AOParams.Blur.Radius = GFSDK_SSAO_BLUR_RADIUS_4;
#endif
#if ENABLE_BLUR_SHARPNESS_PROFILE
		AOParams.Blur.SharpnessProfile.Enable = TRUE;
		AOParams.Blur.SharpnessProfile.ForegroundSharpnessScale = 16.f;
		AOParams.Blur.SharpnessProfile.ForegroundViewDepth = 4.f;
		AOParams.Blur.SharpnessProfile.BackgroundViewDepth = 5.f;
#endif
#if ENABLE_VIEW_DEPTH_THRESHOLD
		AOParams.DepthThreshold.Enable = TRUE;
		AOParams.DepthThreshold.MaxViewDepth = MAX_DEPTH_THRESHOLD;
#endif
#if ENABLE_MSAA_MODES
		AOParams.Output.MSAAMode = GFSDK_SSAO_PER_SAMPLE_AO;
#endif
		static GFSDK_SSAO_FLOAT blendfactor[4] = { 0.5f, 0.5f, 0.5f, 0.5f };
		AOParams.Output.CustomBlendState.pBlendFactor = blendfactor;

		AOParams.Output.BlendMode = GFSDK_SSAO_MULTIPLY_RGB;
	}

	ShadowManager::ShadowManager(EngineState* state, BoundDepthOnlyRenderFunction renderFunction) :
		m_pCamera(&state->m_Camera),
		m_pLight(&state->m_Light),
		m_AmbientOcclusion(state->m_AmbientOcclusion),
		m_DepthOnlyRenderFunction(renderFunction),
		m_iCascadePartitionsMax(100),
		m_AOContext(nullptr)
	{
		Assert_(m_pCamera != nullptr);
		Assert_(m_pLight != nullptr);
		Assert_(m_DepthOnlyRenderFunction != nullptr);
#ifdef TEST_EXTERNAL_MAPS
		const uint32 SHADOWSETTINGS[][4] =
		{
			{ 100, 100, 100, 100 },
			{ 15, 100, 100, 100 },
			{ 7, 25, 100, 100 },
			{ 5, 15, 60, 100 },
		};
		for (uint32 i = 0; i < 8; i++)
			m_iCascadePartitionsZeroToOne[i] = SHADOWSETTINGS[2][i];
#endif

		unsigned int						m_ShadowMapScale = state->m_ShadowMapSize / 512;
		unsigned int						m_ShadowMapWidth = 512;
		unsigned int						m_ShadowMapHeight = 512;
		GFSDK_ShadowLib_ViewLocationDesc	m_ViewLocation[GFSDK_ShadowLib_ViewType_Cascades_4];

		// Light Desc
		m_ESMDesc.LightDesc.eLightType = GFSDK_ShadowLib_LightType_Directional;
		m_ESMDesc.LightDesc.fLightSize = 0.3f;

		// SM Desc
		m_ESMDesc.Desc.eViewType = (GFSDK_ShadowLib_ViewType)state->m_ShadowMapCount;
		m_ESMDesc.Desc.eMapType = GFSDK_ShadowLib_MapType_TextureArray;
		m_ESMDesc.Desc.uArraySize = state->m_ShadowMapCount;
		m_ESMDesc.Desc.uResolutionWidth = m_ShadowMapWidth * m_ShadowMapScale;
		m_ESMDesc.Desc.uResolutionHeight = m_ShadowMapWidth * m_ShadowMapScale;
		m_ESMDesc.fShadowIntensity = 0.2f;

		// SM Desc
		m_SMDesc.eViewType = (GFSDK_ShadowLib_ViewType)state->m_ShadowMapCount;
		m_SMDesc.eMapType = GFSDK_ShadowLib_MapType_TextureArray;
		m_SMDesc.uArraySize = state->m_ShadowMapCount;
		m_SMDesc.uResolutionWidth = m_ShadowMapWidth * m_ShadowMapScale;
		m_SMDesc.uResolutionHeight = m_ShadowMapHeight * m_ShadowMapScale;

		//m_SMRenderParams.iDepthBias = 1;
		//m_SMRenderParams.fSlopeScaledDepthBias = 1.0f;

		// SB Render Params
		m_SBRenderParams.eTechniqueType = (GFSDK_ShadowLib_TechniqueType)state->m_ShadowMapType;
		m_SBRenderParams.eQualityType = (GFSDK_ShadowLib_QualityType)state->m_ShadowMapQuality;
		// Penumbra params
		m_SBRenderParams.PCSSPenumbraParams.fMaxThreshold = 160.0f;
		m_SBRenderParams.PCSSPenumbraParams.fMinSizePercent = 5.0f;
		m_SBRenderParams.PCSSPenumbraParams.fMinWeightExponent = 4.0f;
		m_SBRenderParams.PCSSPenumbraParams.fMinWeightThresholdPercent = 10.0f;

		m_SBRenderParams.fConvergenceTestTolerance = 0.01f;
		m_SBRenderParams.fSoftShadowTestScale = 0.0005f;

		for (int j = 0; j < GFSDK_ShadowLib_ViewType_Cascades_4; j++)
		{
			m_ViewLocation[j].v2Origin.x = 0;
			m_ViewLocation[j].v2Origin.y = 0;
			m_ViewLocation[j].v2Dimension.x = (float)m_ShadowMapWidth;
			m_ViewLocation[j].v2Dimension.y = (float)m_ShadowMapHeight;
			m_ESMDesc.Desc.ViewLocation[j].uMapID = j;
			m_ESMDesc.Desc.ViewLocation[j].v2Origin.x = m_ViewLocation[j].v2Origin.x * m_ShadowMapScale;
			m_ESMDesc.Desc.ViewLocation[j].v2Origin.y = m_ViewLocation[j].v2Origin.y * m_ShadowMapScale;
			m_ESMDesc.Desc.ViewLocation[j].v2Dimension.x = m_ViewLocation[j].v2Dimension.x * m_ShadowMapScale;
			m_ESMDesc.Desc.ViewLocation[j].v2Dimension.y = m_ViewLocation[j].v2Dimension.y * m_ShadowMapScale;
			m_SMDesc.ViewLocation[j].uMapID = j;
			m_SMDesc.ViewLocation[j].v2Origin.x = m_ViewLocation[j].v2Origin.x * m_ShadowMapScale;
			m_SMDesc.ViewLocation[j].v2Origin.y = m_ViewLocation[j].v2Origin.y * m_ShadowMapScale;
			m_SMDesc.ViewLocation[j].v2Dimension.x = m_ViewLocation[j].v2Dimension.x * m_ShadowMapScale;
			m_SMDesc.ViewLocation[j].v2Dimension.y = m_ViewLocation[j].v2Dimension.y * m_ShadowMapScale;
		}

		m_pShadowLibCtx = NULL;
		memset(&m_DeviceAndContext, 0, sizeof(m_DeviceAndContext));
		memset(&m_ShadowBufferSRV, 0, sizeof(m_ShadowBufferSRV));
		memset(&m_DownsampledShadowMap, 0, sizeof(m_DownsampledShadowMap));

		InitAOParams(m_AOParams);

		LoadDLL();
	}


	ShadowManager::~ShadowManager()
	{
		SAFE_RELEASE(m_DownsampledShadowMap.pTexture);
		SAFE_RELEASE(m_DownsampledShadowMap.pSRV);
		SAFE_RELEASE(m_DownsampledShadowMap.pRTV);

		SAFE_RELEASE(m_AOContext);

		if (m_pShadowLibCtx != NULL)
		{
			m_pShadowLibCtx->Destroy();
			m_pShadowLibCtx = NULL;
		}
	}

	//--------------------------------------------------------------------------------------
	//
	//--------------------------------------------------------------------------------------
	void ShadowManager::DisplayMapFrustums(ID3D11RenderTargetView* pOutputRTV, ID3D11DepthStencilView* pDSV)
	{
		gfsdk_float3 v3Color;
		v3Color.x = 1.0f;
		v3Color.y = 0.0f;
		v3Color.z = 0.0f;

		GFSDK_ShadowLib_RenderTargetView ColorRTV;
		ColorRTV.pRTV = pOutputRTV;

		GFSDK_ShadowLib_DepthStencilView DSV;
		DSV.pDSV = pDSV;

		unsigned int NumViews;

#ifdef TEST_EXTERNAL_MAPS
		NumViews = m_ESMDesc.Desc.eViewType;
#else
		NumViews = m_SMDesc.eViewType;
#endif

		for (unsigned int j = 0; j < NumViews; j++)
		{
			switch (j)
			{
			case 0:
				v3Color.x = 1.0f;
				v3Color.y = 0.0f;
				v3Color.z = 0.0f;
				break;
			case 1:
				v3Color.x = 0.0f;
				v3Color.y = 1.0f;
				v3Color.z = 0.0f;
				break;
			case 2:
				v3Color.x = 0.0f;
				v3Color.y = 0.0f;
				v3Color.z = 1.0f;
				break;
			case 3:
				v3Color.x = 1.0f;
				v3Color.y = 1.0f;
				v3Color.z = 0.0f;
				break;
			}

#ifdef TEST_EXTERNAL_MAPS
			m_pShadowLibCtx->DevModeDisplayExternalMapFrustum(m_pShadowBufferHandle,
				&ColorRTV,
				&DSV,
				&m_ESMDesc,
				j,
				v3Color);

#else
			m_pShadowLibCtx->DevModeDisplayMapFrustum(m_pShadowBufferHandle,
				&ColorRTV,
				NULL,
				m_pShadowMapHandle,
				j,
				v3Color);
#endif
		}
	}

	//--------------------------------------------------------------------------------------
	//
	//--------------------------------------------------------------------------------------
	void ShadowManager::DisplayShadowBuffer(ID3D11RenderTargetView* pOutputRTV)
	{
		gfsdk_float2 v2Scale;
		v2Scale.x = 1.0f;
		v2Scale.y = 1.0f;

		GFSDK_ShadowLib_RenderTargetView ColorRTV;
		ColorRTV.pRTV = pOutputRTV;

		m_pShadowLibCtx->DevModeDisplayBuffer(m_pShadowBufferHandle,
			&ColorRTV,
			v2Scale,
			NULL);
	}

	//--------------------------------------------------------------------------------------
	//
	//--------------------------------------------------------------------------------------
	void ShadowManager::DisplayShadowMaps(ID3D11RenderTargetView* pOutputRTV, UINT Width, UINT Height)
	{
		GFSDK_ShadowLib_RenderTargetView ColorRTV;
		ColorRTV.pRTV = pOutputRTV;
#ifdef TEST_EXTERNAL_MAPS

		float fMapResW = (float)m_ESMDesc.Desc.uResolutionWidth;
		float fMapResH = (float)m_ESMDesc.Desc.uResolutionHeight;

		float fWidthScale = Width / ((float)m_ESMDesc.Desc.uArraySize * fMapResW);
		fWidthScale = (fWidthScale > 1.0f) ? (1.0f) : (fWidthScale);

		float fOneFifth = (float)Height / (5.0f);
		float fHeightScale = fOneFifth / fMapResH;
		fHeightScale = (fHeightScale > 1.0f) ? (1.0f) : (fHeightScale);

		float fScale = (fHeightScale < fWidthScale) ? (fHeightScale) : (fWidthScale);

		fMapResW = floorf(fMapResW * fScale);
		fMapResH = floorf(fMapResH * fScale);

		for (unsigned int j = 0; j < (unsigned int)m_ESMDesc.Desc.uArraySize; j++)
		{
			GFSDK_ShadowLib_RenderTargetView ColorRTV;
			ColorRTV.pRTV = pOutputRTV;

			m_pShadowLibCtx->DevModeDisplayExternalMap(m_pShadowBufferHandle,
				&ColorRTV,
				&m_ESMDesc,
				&m_ShadowMapSRV,
				j,
				j * (unsigned int)fMapResW + j,
				Height - (unsigned int)fMapResH,
				fScale);
		}

#else

		float fMapResW = (float)m_SMDesc.uResolutionWidth;
		float fMapResH = (float)m_SMDesc.uResolutionHeight;

		float fWidthScale = Width / ((float)m_SMDesc.uArraySize * fMapResW);
		fWidthScale = (fWidthScale > 1.0f) ? (1.0f) : (fWidthScale);

		float fOneFifth = (float)Height / (5.0f);
		float fHeightScale = fOneFifth / fMapResH;
		fHeightScale = (fHeightScale > 1.0f) ? (1.0f) : (fHeightScale);

		float fScale = (fHeightScale < fWidthScale) ? (fHeightScale) : (fWidthScale);

		fMapResW = floorf(fMapResW * fScale);
		fMapResH = floorf(fMapResH * fScale);

		for (unsigned int j = 0; j < (unsigned int)m_SMDesc.uArraySize; j++)
		{
			m_pShadowLibCtx->DevModeDisplayMap(m_pShadowBufferHandle,
				&ColorRTV,
				m_pShadowMapHandle,
				j,
				j * (unsigned int)fMapResW + j,
				Height - (unsigned int)fMapResH,
				fScale);
		}

#endif
	}

	//--------------------------------------------------------------------------------------
	//
	//--------------------------------------------------------------------------------------
	bool ShadowManager::FindDLL(const char *filePath, std::string &fullPath)
	{
		FILE *fp = NULL;

		// loop N times up the hierarchy, testing at each level
		std::string upPath;
		for (int i = 0; i < 10; i++)
		{
			fullPath.assign(upPath);  // reset to current upPath.
			//	fullPath.append("bin/");
			fullPath.append(filePath);

			fopen_s(&fp, fullPath.c_str(), "rb");
			if (fp)
				break;

			upPath.append("../");
		}

		if (!fp)
		{
			fprintf(stderr, "Error opening file '%s'\n", filePath);
			return false;
		}

		fclose(fp);

		return true;
	}

	//--------------------------------------------------------------------------------------
	//
	//--------------------------------------------------------------------------------------
	void ShadowManager::LoadDLL()
	{

#ifdef _WIN64
		std::string dllPath;
		if (!FindDLL("GFSDK_ShadowLib_DX11.win64.dll", dllPath))
		{
			MessageBoxA(NULL, "GFSDK_ShadowLib_DX11.win64.dll not found", "Error", MB_OK);
			exit(1);
		}
#else
		std::string dllPath;
		if (!FindDLL("GFSDK_ShadowLib_DX11.win32.dll", dllPath))
		{
			MessageBoxA(NULL, "GFSDK_ShadowLib_DX11.win32.dll not found", "Error", MB_OK);
			exit(1);
		}
#endif

		HMODULE module = LoadLibraryA(dllPath.c_str());
		if (!module)
		{
			MessageBoxA(NULL, "LoadLibraryA failed", "Error", MB_OK);
			exit(1);
		}

		m_GetDLLVersion_Proc = GetProcAddress(module, "GFSDK_ShadowLib_GetDLLVersion");
		m_Create_Proc = GetProcAddress(module, "GFSDK_ShadowLib_Create");
	}

	//--------------------------------------------------------------------------------------
	//
	//--------------------------------------------------------------------------------------
	void ShadowManager::CreateResources(ID3D11Device* device, const DXGI_SURFACE_DESC& BackBufferDesc, ID3D11DepthStencilView* pReadOnlyDSV)
	{
		m_SBDesc.uResolutionWidth = BackBufferDesc.Width;
		m_SBDesc.uResolutionHeight = BackBufferDesc.Height;
		m_SBDesc.uSampleCount = BackBufferDesc.SampleDesc.Count;
		m_SBDesc.ReadOnlyDSV.pDSV = pReadOnlyDSV;


#ifdef TEST_EXTERNAL_MAPS
		if (m_ESMDesc.Desc.eMapType == GFSDK_ShadowLib_MapType_Texture &&
			m_ESMDesc.Desc.eViewType == GFSDK_ShadowLib_ViewType_Single)
		{
			m_ESMDesc.Desc.bDownsample = true;
		}

		if (m_ESMDesc.Desc.eMapType == GFSDK_ShadowLib_MapType_Texture && m_ESMDesc.Desc.eViewType == GFSDK_ShadowLib_ViewType_Single)
		{
			m_DownsampledShadowMap.uWidth = m_ESMDesc.Desc.uResolutionWidth >> 1;
			m_DownsampledShadowMap.uHeight = m_ESMDesc.Desc.uResolutionHeight >> 1;
			m_DownsampledShadowMap.uSampleCount = 1;
			m_DownsampledShadowMap.Format = DXGI_FORMAT_R32_FLOAT;
			SAFE_RELEASE(m_DownsampledShadowMap.pTexture);
			SAFE_RELEASE(m_DownsampledShadowMap.pSRV);
			SAFE_RELEASE(m_DownsampledShadowMap.pRTV);
			m_pShadowLibCtx->DevModeCreateTexture2D(&m_DownsampledShadowMap);
		}
#else
		if (m_pShadowMapHandle)
			m_pShadowLibCtx->RemoveMap(&m_pShadowMapHandle);

		if (m_SMDesc.eMapType == GFSDK_ShadowLib_MapType_Texture &&
			m_SMDesc.eViewType == GFSDK_ShadowLib_ViewType_Single)
		{
			m_SMDesc.bDownsample = true;
		}

		m_pShadowLibCtx->AddMap(&m_SMDesc, &m_pShadowMapHandle);

		if (m_SMDesc.eMapType == GFSDK_ShadowLib_MapType_Texture && m_SMDesc.eViewType == GFSDK_ShadowLib_ViewType_Single)
		{
			m_DownsampledShadowMap.uWidth = m_SMDesc.uResolutionWidth >> 1;
			m_DownsampledShadowMap.uHeight = m_SMDesc.uResolutionHeight >> 1;
			m_DownsampledShadowMap.uSampleCount = 1;
			m_DownsampledShadowMap.Format = DXGI_FORMAT_R32_FLOAT;
			SAFE_RELEASE(m_DownsampledShadowMap.pTexture);
			SAFE_RELEASE(m_DownsampledShadowMap.pSRV);
			SAFE_RELEASE(m_DownsampledShadowMap.pRTV);
			m_pShadowLibCtx->DevModeCreateTexture2D(&m_DownsampledShadowMap);
		}
#endif

		if (m_pShadowBufferHandle)
			m_pShadowLibCtx->RemoveBuffer(&m_pShadowBufferHandle);
		m_pShadowLibCtx->AddBuffer(&m_SBDesc, &m_pShadowBufferHandle);

		if (m_AmbientOcclusion)
		{
			GFSDK_SSAO_Status status;
			status = m_AOContext->PreCreateRTs(&m_AOParams, (GFSDK_SSAO_UINT)m_SBDesc.uResolutionWidth, (GFSDK_SSAO_UINT)m_SBDesc.uResolutionHeight);
			assert(status == GFSDK_SSAO_OK);
		}
	}

	void ShadowManager::CreateDevice(ID3D11Device* device, ID3D11DeviceContext* context)
	{
#ifdef TEST_EXTERNAL_MAPS
		m_vSceneAABBMin = XMVECTOR(g_XMFltMax);
		m_vSceneAABBMax = XMVECTOR(g_XMFltMin);

		for (uint32 index = 0; index < GFSDK_ShadowLib_ViewType_Cascades_4; ++index)
		{
			m_RenderVP[index].Height = (FLOAT)m_ESMDesc.Desc.uResolutionWidth;
			m_RenderVP[index].Width = (FLOAT)m_ESMDesc.Desc.uResolutionHeight;
			m_RenderVP[index].MaxDepth = 1.0f;
			m_RenderVP[index].MinDepth = 0.0f;
			m_RenderVP[index].TopLeftX = 0.0f;
			m_RenderVP[index].TopLeftY = 0.0f;
		}

		m_CascadedShadowMap.Initialize(device,
			m_ESMDesc.Desc.uResolutionWidth,
			m_ESMDesc.Desc.uResolutionHeight,
			DXGI_FORMAT_D32_FLOAT,
			true,
			1,
			0,
			m_ESMDesc.Desc.uArraySize);

		m_ShadowMapSRV.pSRV = m_CascadedShadowMap.SRView;
#endif
		// DLL version
		GFSDK_ShadowLib_Version DLLVersion;
		GFSDK_ShadowLib_Status retCode = ((GetDLLVersion)m_GetDLLVersion_Proc)(&DLLVersion);

		// Header version
		m_ShadowLibVersion.uMajor = GFSDK_SHADOWLIB_MAJOR_VERSION;
		m_ShadowLibVersion.uMinor = GFSDK_SHADOWLIB_MINOR_VERSION;

		// Do they match?
		if (DLLVersion.uMajor == m_ShadowLibVersion.uMajor && DLLVersion.uMinor == m_ShadowLibVersion.uMinor)
		{
			m_DeviceAndContext.pD3DDevice = device;
			m_DeviceAndContext.pDeviceContext = context;

			retCode = ((Create)m_Create_Proc)(&m_ShadowLibVersion, &m_pShadowLibCtx, &m_DeviceAndContext, NULL);

			if (retCode != GFSDK_ShadowLib_Status_Ok) assert(false);
		}
		else
		{
			assert(false);
		}

		m_ConstantBufferFrame.Initialize(device);

		SAFE_RELEASE(m_AOContext);
		if (m_AmbientOcclusion)
		{
			GFSDK_SSAO_Status status;
			status = GFSDK_SSAO_CreateContext_D3D11(device, &m_AOContext);
			assert(status == GFSDK_SSAO_OK);

			SAFE_RELEASE(m_ShadowBufferRTV);
		}
	}

	//--------------------------------------------------------------------------------------
	// Render the cascades into a texture atlas.
	//--------------------------------------------------------------------------------------
	static const XMVECTORF32 g_vHalfVector = { 0.5f, 0.5f, 0.5f, 0.5f };
	static const XMVECTORF32 g_vMultiplySetzwToZero = { 1.0f, 1.0f, 0.0f, 0.0f };
	//--------------------------------------------------------------------------------------
	// This function takes the camera's projection matrix and returns the 8
	// points that make up a view frustum.
	// The frustum is scaled to fit within the Begin and End interval paramaters.
	//--------------------------------------------------------------------------------------
	void CreateFrustumPointsFromCascadeInterval(float fCascadeIntervalBegin, FLOAT fCascadeIntervalEnd, CXMMATRIX vProjection, XMVECTOR* pvCornerPointsWorld)
	{

		BoundingFrustum vViewFrust(vProjection);
		vViewFrust.Near = fCascadeIntervalBegin;
		vViewFrust.Far = fCascadeIntervalEnd;

		static const XMVECTORU32 vGrabY = { 0x00000000, 0xFFFFFFFF, 0x00000000, 0x00000000 };
		static const XMVECTORU32 vGrabX = { 0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000 };

		XMVECTOR vRightTop = { vViewFrust.RightSlope, vViewFrust.TopSlope, 1.0f, 1.0f };
		XMVECTOR vLeftBottom = { vViewFrust.LeftSlope, vViewFrust.BottomSlope, 1.0f, 1.0f };
		XMVECTOR vNear = { vViewFrust.Near, vViewFrust.Near, vViewFrust.Near, 1.0f };
		XMVECTOR vFar = { vViewFrust.Far, vViewFrust.Far, vViewFrust.Far, 1.0f };
		XMVECTOR vRightTopNear = XMVectorMultiply(vRightTop, vNear);
		XMVECTOR vRightTopFar = XMVectorMultiply(vRightTop, vFar);
		XMVECTOR vLeftBottomNear = XMVectorMultiply(vLeftBottom, vNear);
		XMVECTOR vLeftBottomFar = XMVectorMultiply(vLeftBottom, vFar);

		pvCornerPointsWorld[0] = vRightTopNear;
		pvCornerPointsWorld[1] = XMVectorSelect(vRightTopNear, vLeftBottomNear, vGrabX);
		pvCornerPointsWorld[2] = vLeftBottomNear;
		pvCornerPointsWorld[3] = XMVectorSelect(vRightTopNear, vLeftBottomNear, vGrabY);

		pvCornerPointsWorld[4] = vRightTopFar;
		pvCornerPointsWorld[5] = XMVectorSelect(vRightTopFar, vLeftBottomFar, vGrabX);
		pvCornerPointsWorld[6] = vLeftBottomFar;
		pvCornerPointsWorld[7] = XMVectorSelect(vRightTopFar, vLeftBottomFar, vGrabY);

	}

	//--------------------------------------------------------------------------------------
	// Used to compute an intersection of the orthographic projection and the Scene AABB
	//--------------------------------------------------------------------------------------
	struct Triangle
	{
		XMVECTOR pt[3];
		bool culled;
	};


	//--------------------------------------------------------------------------------------
	// Computing an accurate near and flar plane will decrease surface acne and Peter-panning.
	// Surface acne is the term for erroneous self shadowing.  Peter-panning is the effect where
	// shadows disappear near the base of an object.
	// As offsets are generally used with PCF filtering due self shadowing issues, computing the
	// correct near and far planes becomes even more important.
	// This concept is not complicated, but the intersection code is.
	//--------------------------------------------------------------------------------------
	void ComputeNearAndFar(FLOAT& fNearPlane, FLOAT& fFarPlane, FXMVECTOR vLightCameraOrthographicMin, FXMVECTOR vLightCameraOrthographicMax, XMVECTOR* pvPointsInCameraView)
	{

		// Initialize the near and far planes
		fNearPlane = FLT_MAX;
		fFarPlane = -FLT_MAX;

		Triangle triangleList[16];
		INT iTriangleCnt = 1;

		triangleList[0].pt[0] = pvPointsInCameraView[0];
		triangleList[0].pt[1] = pvPointsInCameraView[1];
		triangleList[0].pt[2] = pvPointsInCameraView[2];
		triangleList[0].culled = false;

		// These are the indices used to tesselate an AABB into a list of triangles.
		static const INT iAABBTriIndexes[] =
		{
			0, 1, 2, 1, 2, 3,
			4, 5, 6, 5, 6, 7,
			0, 2, 4, 2, 4, 6,
			1, 3, 5, 3, 5, 7,
			0, 1, 4, 1, 4, 5,
			2, 3, 6, 3, 6, 7
		};

		INT iPointPassesCollision[3];

		// At a high level: 
		// 1. Iterate over all 12 triangles of the AABB.  
		// 2. Clip the triangles against each plane. Create new triangles as needed.
		// 3. Find the min and max z values as the near and far plane.

		//This is easier because the triangles are in camera spacing making the collisions tests simple comparisions.

		float fLightCameraOrthographicMinX = XMVectorGetX(vLightCameraOrthographicMin);
		float fLightCameraOrthographicMaxX = XMVectorGetX(vLightCameraOrthographicMax);
		float fLightCameraOrthographicMinY = XMVectorGetY(vLightCameraOrthographicMin);
		float fLightCameraOrthographicMaxY = XMVectorGetY(vLightCameraOrthographicMax);

		for (INT AABBTriIter = 0; AABBTriIter < 12; ++AABBTriIter)
		{

			triangleList[0].pt[0] = pvPointsInCameraView[iAABBTriIndexes[AABBTriIter * 3 + 0]];
			triangleList[0].pt[1] = pvPointsInCameraView[iAABBTriIndexes[AABBTriIter * 3 + 1]];
			triangleList[0].pt[2] = pvPointsInCameraView[iAABBTriIndexes[AABBTriIter * 3 + 2]];
			iTriangleCnt = 1;
			triangleList[0].culled = FALSE;

			// Clip each invidual triangle against the 4 frustums.  When ever a triangle is clipped into new triangles, 
			//add them to the list.
			for (INT frustumPlaneIter = 0; frustumPlaneIter < 4; ++frustumPlaneIter)
			{

				FLOAT fEdge;
				INT iComponent;

				if (frustumPlaneIter == 0)
				{
					fEdge = fLightCameraOrthographicMinX; // todo make float temp
					iComponent = 0;
				}
				else if (frustumPlaneIter == 1)
				{
					fEdge = fLightCameraOrthographicMaxX;
					iComponent = 0;
				}
				else if (frustumPlaneIter == 2)
				{
					fEdge = fLightCameraOrthographicMinY;
					iComponent = 1;
				}
				else
				{
					fEdge = fLightCameraOrthographicMaxY;
					iComponent = 1;
				}

				for (INT triIter = 0; triIter < iTriangleCnt; ++triIter)
				{
					// We don't delete triangles, so we skip those that have been culled.
					if (!triangleList[triIter].culled)
					{
						INT iInsideVertCount = 0;
						XMVECTOR tempOrder;
						// Test against the correct frustum plane.
						// This could be written more compactly, but it would be harder to understand.

						if (frustumPlaneIter == 0)
						{
							for (INT triPtIter = 0; triPtIter < 3; ++triPtIter)
							{
								if (XMVectorGetX(triangleList[triIter].pt[triPtIter]) >
									XMVectorGetX(vLightCameraOrthographicMin))
								{
									iPointPassesCollision[triPtIter] = 1;
								}
								else
								{
									iPointPassesCollision[triPtIter] = 0;
								}
								iInsideVertCount += iPointPassesCollision[triPtIter];
							}
						}
						else if (frustumPlaneIter == 1)
						{
							for (INT triPtIter = 0; triPtIter < 3; ++triPtIter)
							{
								if (XMVectorGetX(triangleList[triIter].pt[triPtIter]) <
									XMVectorGetX(vLightCameraOrthographicMax))
								{
									iPointPassesCollision[triPtIter] = 1;
								}
								else
								{
									iPointPassesCollision[triPtIter] = 0;
								}
								iInsideVertCount += iPointPassesCollision[triPtIter];
							}
						}
						else if (frustumPlaneIter == 2)
						{
							for (INT triPtIter = 0; triPtIter < 3; ++triPtIter)
							{
								if (XMVectorGetY(triangleList[triIter].pt[triPtIter]) >
									XMVectorGetY(vLightCameraOrthographicMin))
								{
									iPointPassesCollision[triPtIter] = 1;
								}
								else
								{
									iPointPassesCollision[triPtIter] = 0;
								}
								iInsideVertCount += iPointPassesCollision[triPtIter];
							}
						}
						else
						{
							for (INT triPtIter = 0; triPtIter < 3; ++triPtIter)
							{
								if (XMVectorGetY(triangleList[triIter].pt[triPtIter]) <
									XMVectorGetY(vLightCameraOrthographicMax))
								{
									iPointPassesCollision[triPtIter] = 1;
								}
								else
								{
									iPointPassesCollision[triPtIter] = 0;
								}
								iInsideVertCount += iPointPassesCollision[triPtIter];
							}
						}

						// Move the points that pass the frustum test to the begining of the array.
						if (iPointPassesCollision[1] && !iPointPassesCollision[0])
						{
							tempOrder = triangleList[triIter].pt[0];
							triangleList[triIter].pt[0] = triangleList[triIter].pt[1];
							triangleList[triIter].pt[1] = tempOrder;
							iPointPassesCollision[0] = TRUE;
							iPointPassesCollision[1] = FALSE;
						}
						if (iPointPassesCollision[2] && !iPointPassesCollision[1])
						{
							tempOrder = triangleList[triIter].pt[1];
							triangleList[triIter].pt[1] = triangleList[triIter].pt[2];
							triangleList[triIter].pt[2] = tempOrder;
							iPointPassesCollision[1] = TRUE;
							iPointPassesCollision[2] = FALSE;
						}
						if (iPointPassesCollision[1] && !iPointPassesCollision[0])
						{
							tempOrder = triangleList[triIter].pt[0];
							triangleList[triIter].pt[0] = triangleList[triIter].pt[1];
							triangleList[triIter].pt[1] = tempOrder;
							iPointPassesCollision[0] = TRUE;
							iPointPassesCollision[1] = FALSE;
						}

						if (iInsideVertCount == 0)
						{ // All points failed. We're done,  
							triangleList[triIter].culled = true;
						}
						else if (iInsideVertCount == 1)
						{// One point passed. Clip the triangle against the Frustum plane
							triangleList[triIter].culled = false;

							// 
							XMVECTOR vVert0ToVert1 = triangleList[triIter].pt[1] - triangleList[triIter].pt[0];
							XMVECTOR vVert0ToVert2 = triangleList[triIter].pt[2] - triangleList[triIter].pt[0];

							// Find the collision ratio.
							FLOAT fHitPointTimeRatio = fEdge - XMVectorGetByIndex(triangleList[triIter].pt[0], iComponent);
							// Calculate the distance along the vector as ratio of the hit ratio to the component.
							FLOAT fDistanceAlongVector01 = fHitPointTimeRatio / XMVectorGetByIndex(vVert0ToVert1, iComponent);
							FLOAT fDistanceAlongVector02 = fHitPointTimeRatio / XMVectorGetByIndex(vVert0ToVert2, iComponent);
							// Add the point plus a percentage of the vector.
							vVert0ToVert1 *= fDistanceAlongVector01;
							vVert0ToVert1 += triangleList[triIter].pt[0];
							vVert0ToVert2 *= fDistanceAlongVector02;
							vVert0ToVert2 += triangleList[triIter].pt[0];

							triangleList[triIter].pt[1] = vVert0ToVert2;
							triangleList[triIter].pt[2] = vVert0ToVert1;

						}
						else if (iInsideVertCount == 2)
						{ // 2 in  // tesselate into 2 triangles


							// Copy the triangle\(if it exists) after the current triangle out of
							// the way so we can override it with the new triangle we're inserting.
							triangleList[iTriangleCnt] = triangleList[triIter + 1];

							triangleList[triIter].culled = false;
							triangleList[triIter + 1].culled = false;

							// Get the vector from the outside point into the 2 inside points.
							XMVECTOR vVert2ToVert0 = triangleList[triIter].pt[0] - triangleList[triIter].pt[2];
							XMVECTOR vVert2ToVert1 = triangleList[triIter].pt[1] - triangleList[triIter].pt[2];

							// Get the hit point ratio.
							FLOAT fHitPointTime_2_0 = fEdge - XMVectorGetByIndex(triangleList[triIter].pt[2], iComponent);
							FLOAT fDistanceAlongVector_2_0 = fHitPointTime_2_0 / XMVectorGetByIndex(vVert2ToVert0, iComponent);
							// Calcaulte the new vert by adding the percentage of the vector plus point 2.
							vVert2ToVert0 *= fDistanceAlongVector_2_0;
							vVert2ToVert0 += triangleList[triIter].pt[2];

							// Add a new triangle.
							triangleList[triIter + 1].pt[0] = triangleList[triIter].pt[0];
							triangleList[triIter + 1].pt[1] = triangleList[triIter].pt[1];
							triangleList[triIter + 1].pt[2] = vVert2ToVert0;

							//Get the hit point ratio.
							FLOAT fHitPointTime_2_1 = fEdge - XMVectorGetByIndex(triangleList[triIter].pt[2], iComponent);
							FLOAT fDistanceAlongVector_2_1 = fHitPointTime_2_1 / XMVectorGetByIndex(vVert2ToVert1, iComponent);
							vVert2ToVert1 *= fDistanceAlongVector_2_1;
							vVert2ToVert1 += triangleList[triIter].pt[2];
							triangleList[triIter].pt[0] = triangleList[triIter + 1].pt[1];
							triangleList[triIter].pt[1] = triangleList[triIter + 1].pt[2];
							triangleList[triIter].pt[2] = vVert2ToVert1;
							// Cncrement triangle count and skip the triangle we just inserted.
							++iTriangleCnt;
							++triIter;


						}
						else
						{ // all in
							triangleList[triIter].culled = false;

						}
					}// end if !culled loop            
				}
			}
			for (INT index = 0; index < iTriangleCnt; ++index)
			{
				if (!triangleList[index].culled)
				{
					// Set the near and far plan and the min and max z values respectivly.
					for (int vertind = 0; vertind < 3; ++vertind)
					{
						float fTriangleCoordZ = XMVectorGetZ(triangleList[index].pt[vertind]);
						if (fNearPlane > fTriangleCoordZ)
						{
							fNearPlane = fTriangleCoordZ;
						}
						if (fFarPlane < fTriangleCoordZ)
						{
							fFarPlane = fTriangleCoordZ;
						}
					}
				}
			}
		}
	}

	void ShadowManager::RenderShadowMaps(ID3D11DeviceContext* context)
	{
#ifdef TEST_EXTERNAL_MAPS
		XMVECTOR eyePt = m_pCamera->GetPosition();
		XMVECTOR direction = XMVectorSetY(m_pCamera->GetDirection(), 0.0f) * m_pCamera->GetOptFarClip() * 0.1f;
		XMVECTOR vMeshMin = eyePt - XMVectorSet(100.0f, 0.0f, 100.0f, 0.0f) + direction;
		XMVECTOR vMeshMax = eyePt + XMVectorSet(100.0f, 50.0f, 100.0f, 0.0f) + direction;

		m_vSceneAABBMin = vMeshMin;
		m_vSceneAABBMax = vMeshMax;

		XMMATRIX matViewCameraProjection = m_pCamera->GetCameraProjection();
		XMMATRIX matViewCameraView = m_pCamera->GetCameraView();
		XMMATRIX matLightCameraView = m_pLight->GetCameraView();
		XMMATRIX matInverseViewCamera = XMMatrixInverse(nullptr, matViewCameraView);

		XMStoreFloat4x4((XMFLOAT4X4*)&m_ESMDesc.m4x4EyeProjectionMatrix, matViewCameraProjection);
		XMStoreFloat4x4((XMFLOAT4X4*)&m_ESMDesc.m4x4EyeViewMatrix, matViewCameraView);

		XMStoreFloat3((XMFLOAT3*)&m_ESMDesc.LightDesc.v3LightPos, m_pCamera->GetPosition());
		XMStoreFloat3((XMFLOAT3*)&m_ESMDesc.LightDesc.v3LightLookAt, m_pCamera->GetLookAtPoint());

		// Convert from min max representation to center extents represnetation.
		// This will make it easier to pull the points out of the transformation.
		BoundingBox bb;
		BoundingBox::CreateFromPoints(bb, m_vSceneAABBMin, m_vSceneAABBMax);

		DirectX::SimpleMath::Vector3 tmp[8];
		bb.GetCorners(tmp);

		// Transform the scene AABB to Light space.
		XMVECTOR vSceneAABBPointsLightSpace[8];
		for (int index = 0; index < 8; ++index)
		{
			XMVECTOR v = XMLoadFloat3(&tmp[index]);
			vSceneAABBPointsLightSpace[index] = XMVector3Transform(v, matLightCameraView);
		}

		FLOAT fFrustumIntervalBegin, fFrustumIntervalEnd;
		XMVECTOR vLightCameraOrthographicMin;  // light space frustrum aabb 
		XMVECTOR vLightCameraOrthographicMax;
		FLOAT fCameraNearFarRange = m_pCamera->GetFarClip() - m_pCamera->GetNearClip();
		XMVECTOR vWorldUnitsPerTexel = g_XMZero;

		// We loop over the cascades to calculate the orthographic projection for each cascade.
		for (uint32 iCascadeIndex = 0; iCascadeIndex < m_ESMDesc.Desc.uArraySize; ++iCascadeIndex)
		{
			// In the FIT_TO_SCENE technique the Cascades overlap eachother.  In other words, interval 1 is coverd by
			// cascades 1 to 8, interval 2 is covered by cascades 2 to 8 and so forth.
			fFrustumIntervalBegin = 0.0f;

			// Scale the intervals between 0 and 1. They are now percentages that we can scale with.
			fFrustumIntervalEnd = (FLOAT)m_iCascadePartitionsZeroToOne[iCascadeIndex];
			fFrustumIntervalBegin /= (FLOAT)m_iCascadePartitionsMax;
			fFrustumIntervalEnd /= (FLOAT)m_iCascadePartitionsMax;
			fFrustumIntervalBegin = fFrustumIntervalBegin * fCameraNearFarRange;
			fFrustumIntervalEnd = fFrustumIntervalEnd * fCameraNearFarRange;
			XMVECTOR vFrustumPoints[8];

			// This function takes the began and end intervals along with the projection matrix and returns the 8
			// points that repreresent the cascade Interval
			CreateFrustumPointsFromCascadeInterval(fFrustumIntervalBegin, fFrustumIntervalEnd, matViewCameraProjection, vFrustumPoints);

			vLightCameraOrthographicMin = g_FltMax;
			vLightCameraOrthographicMax = -g_FltMax;

			XMVECTOR vTempTranslatedCornerPoint;
			// This next section of code calculates the min and max values for the orthographic projection.
			for (int icpIndex = 0; icpIndex < 8; ++icpIndex)
			{
				// Transform the frustum from camera view space to world space.
				vFrustumPoints[icpIndex] = XMVector4Transform(vFrustumPoints[icpIndex], matInverseViewCamera);
				// Transform the point from world space to Light Camera Space.
				vTempTranslatedCornerPoint = XMVector4Transform(vFrustumPoints[icpIndex], matLightCameraView);
				// Find the closest point.
				vLightCameraOrthographicMin = XMVectorMin(vTempTranslatedCornerPoint, vLightCameraOrthographicMin);
				vLightCameraOrthographicMax = XMVectorMax(vTempTranslatedCornerPoint, vLightCameraOrthographicMax);
			}

			// Fit the ortho projection to the cascades far plane and a near plane of zero. 
			// Pad the projection to be the size of the diagonal of the Frustum partition. 
			// 
			// To do this, we pad the ortho transform so that it is always big enough to cover 
			// the entire camera view frustum.
			XMVECTOR vDiagonal = vFrustumPoints[0] - vFrustumPoints[6];
			vDiagonal = XMVector3Length(vDiagonal);

			// The bound is the length of the diagonal of the frustum interval.
			FLOAT fCascadeBound = XMVectorGetX(vDiagonal);

			// The offset calculated will pad the ortho projection so that it is always the same size 
			// and big enough to cover the entire cascade interval.
			XMVECTOR vBoarderOffset = (vDiagonal - (vLightCameraOrthographicMax - vLightCameraOrthographicMin)) * g_vHalfVector;
			// Set the Z and W components to zero.
			vBoarderOffset *= g_vMultiplySetzwToZero;

			// Add the offsets to the projection.
			vLightCameraOrthographicMax += vBoarderOffset;
			vLightCameraOrthographicMin -= vBoarderOffset;

			// The world units per texel are used to snap the shadow the orthographic projection
			// to texel sized increments.  This keeps the edges of the shadows from shimmering.
			FLOAT fWorldUnitsPerTexel = fCascadeBound / (float)m_ESMDesc.Desc.uResolutionWidth;
			vWorldUnitsPerTexel = XMVectorSet(fWorldUnitsPerTexel, fWorldUnitsPerTexel, 0.0f, 0.0f);

			float fLightCameraOrthographicMinZ = XMVectorGetZ(vLightCameraOrthographicMin);


			//if (m_bMoveLightTexelSize)
			{
				// We snape the camera to 1 pixel increments so that moving the camera does not cause the shadows to jitter.
				// This is a matter of integer dividing by the world space size of a texel
				vLightCameraOrthographicMin /= vWorldUnitsPerTexel;
				vLightCameraOrthographicMin = XMVectorFloor(vLightCameraOrthographicMin);
				vLightCameraOrthographicMin *= vWorldUnitsPerTexel;

				vLightCameraOrthographicMax /= vWorldUnitsPerTexel;
				vLightCameraOrthographicMax = XMVectorFloor(vLightCameraOrthographicMax);
				vLightCameraOrthographicMax *= vWorldUnitsPerTexel;

			}

			//These are the unconfigured near and far plane values.  They are purposly awful to show 
			// how important calculating accurate near and far planes is.
			FLOAT fNearPlane = 0.1f;
			FLOAT fFarPlane = 10.0f;


			XMVECTOR vLightSpaceSceneAABBminValue = g_FltMax;  // world space scene aabb 
			XMVECTOR vLightSpaceSceneAABBmaxValue = -g_FltMax;
			// We calculate the min and max vectors of the scene in light space. The min and max "Z" values of the  
			// light space AABB can be used for the near and far plane. This is easier than intersecting the scene with the AABB
			// and in some cases provides similar results.
			for (int index = 0; index < 8; ++index)
			{
				vLightSpaceSceneAABBminValue = XMVectorMin(vSceneAABBPointsLightSpace[index], vLightSpaceSceneAABBminValue);
				vLightSpaceSceneAABBmaxValue = XMVectorMax(vSceneAABBPointsLightSpace[index], vLightSpaceSceneAABBmaxValue);
			}

			// The min and max z values are the near and far planes.
			fNearPlane = XMVectorGetZ(vLightSpaceSceneAABBminValue);
			fFarPlane = XMVectorGetZ(vLightSpaceSceneAABBmaxValue);

			// Create the orthographic projection for this cascade.
			m_matShadowProj[iCascadeIndex] = XMMatrixOrthographicOffCenterLH(XMVectorGetX(vLightCameraOrthographicMin), XMVectorGetX(vLightCameraOrthographicMax),
				XMVectorGetY(vLightCameraOrthographicMin), XMVectorGetY(vLightCameraOrthographicMax),
				fNearPlane, fFarPlane);
			m_fCascadePartitionsFrustum[iCascadeIndex] = fFrustumIntervalEnd;
		}
		m_matShadowView = m_pLight->GetCameraView();

		//if (m_eSelectedNearFarFit == FIT_NEARFAR_PANCAKING)
		//{
		//	pd3dDeviceContext->RSSetState(m_prsShadowPancake);
		//}
		//else
		//{
		//	pd3dDeviceContext->RSSetState(m_prsShadow);
		//}

		m_ConstantBufferFrame.SetVS(context, 0);
		m_ConstantBufferFrame.SetHS(context, 0);
		m_ConstantBufferFrame.SetDS(context, 0);
		m_ConstantBufferFrame.SetPS(context, 0);

		// Iterate over cascades and render shadows.
		for (uint32 currentCascade = 0; currentCascade < m_ESMDesc.Desc.uArraySize; ++currentCascade)
		{
			// Set the shadow map as the depth target
			ID3D11DepthStencilView* dsv = m_CascadedShadowMap.ArraySlices[currentCascade];
			ID3D11RenderTargetView* nullRenderTargets[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = { nullptr };
			context->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, nullRenderTargets, dsv);
			context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

			// Each cascade has its own viewport because we're storing all the cascades in one large texture.
			context->RSSetViewports(1, &m_RenderVP[currentCascade]);

			// We calculate the matrices in the Init function.
			XMMATRIX matWorldViewProjection = m_matShadowView * m_matShadowProj[currentCascade];

			m_ConstantBufferFrame.Data.BindViewAndProj(m_matShadowView, m_matShadowProj[currentCascade]);
			m_ConstantBufferFrame.Data.vViewport = DirectX::SimpleMath::Vector4(m_RenderVP[currentCascade].Width, m_RenderVP[currentCascade].Height, 1.f, 1.f);
			m_ConstantBufferFrame.ApplyChanges(context);

			m_DepthOnlyRenderFunction(context, matWorldViewProjection);
		}
#else
		XMStoreFloat4x4((XMFLOAT4X4*)&m_SMRenderParams.m4x4EyeProjectionMatrix, ProjMatrix);
		XMStoreFloat4x4((XMFLOAT4X4*)&m_SMRenderParams.m4x4EyeViewMatrix, ViewMatrix);

		XMStoreFloat3((XMFLOAT3*)&m_SMRenderParams.LightDesc.v3LightPos, eyePt);
		XMStoreFloat3((XMFLOAT3*)&m_SMRenderParams.LightDesc.v3LightLookAt, lootAt);

		//eyePt = XMVectorSetY(eyePt, 0.0f);
		XMVECTOR direction = XMVectorSetY(mState.mCamera.GetDirection(), 0.0f) * 32.0f;
		XMVECTOR vMeshMin = eyePt - XMVectorSet(100.0f, 0.0f, 100.0f, 0.0f) + direction;
		XMVECTOR vMeshMax = eyePt + XMVectorSet(100.0f, 50.0f, 100.0f, 0.0f) + direction;

		XMStoreFloat3((XMFLOAT3*)&m_SMRenderParams.v3WorldSpaceBBox[0], vMeshMin);
		XMStoreFloat3((XMFLOAT3*)&m_SMRenderParams.v3WorldSpaceBBox[1], vMeshMax);

		m_SMRenderParams.fnpDrawFunction = GFSDK_ShadowLib_FunctionPointer(ShadowMapRenderFunction);
		m_SMRenderParams.pDrawFunctionParams = pd3dDeviceContext;

		m_pShadowLibCtx->RenderMap(m_pShadowMapHandle, &m_SMRenderParams);
#endif
		ID3D11RenderTargetView* nullRenderTargets[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = { nullptr };
		context->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, nullRenderTargets, nullptr);
	}

	//--------------------------------------------------------------------------------------
	// 
	//--------------------------------------------------------------------------------------
	void ShadowManager::ModulateShadowBuffer(ID3D11RenderTargetView* pOutputRTV)
	{
		gfsdk_float3 v3ShadowColor;
		v3ShadowColor.x = 0.3f;
		v3ShadowColor.y = 0.3f;
		v3ShadowColor.z = 0.3f;

		GFSDK_ShadowLib_RenderTargetView ColorRTV;
		ColorRTV.pRTV = pOutputRTV;

		m_pShadowLibCtx->ModulateBuffer(m_pShadowBufferHandle, &ColorRTV, v3ShadowColor, GFSDK_ShadowLib_ModulateBufferMask_R);
	}

	//--------------------------------------------------------------------------------------
	//
	//--------------------------------------------------------------------------------------
	void ShadowManager::RenderShadowBuffer(ID3D11ShaderResourceView* pDSSRV, ID3D11ShaderResourceView* pDSResolvedSRV)
	{

		if (m_SBRenderParams.eTechniqueType == GFSDK_ShadowLib_TechniqueType_PCSS &&
#ifdef TEST_EXTERNAL_MAPS
			m_ESMDesc.Desc.eMapType == GFSDK_ShadowLib_MapType_Texture &&
			m_ESMDesc.Desc.eViewType == GFSDK_ShadowLib_ViewType_Single)
#else
			m_SMDesc.eMapType == GFSDK_ShadowLib_MapType_Texture &&
			m_SMDesc.eViewType == GFSDK_ShadowLib_ViewType_Single)
#endif
		{
			m_TempResources.pDownsampledShadowMap = &m_DownsampledShadowMap;
			m_pShadowLibCtx->SetTempResources(&m_TempResources);
		}

		m_pShadowLibCtx->ClearBuffer(m_pShadowBufferHandle);

		m_SBRenderParams.DepthBufferDesc.DepthStencilSRV.pSRV = pDSSRV;

		GFSDK_ShadowLib_Status retCode;

		if (m_SBDesc.uSampleCount > 1)
		{
			m_SBRenderParams.eMSAARenderMode = GFSDK_ShadowLib_MSAARenderMode_ComplexPixelMask;
			m_SBRenderParams.DepthBufferDesc.ResolvedDepthStencilSRV.pSRV = pDSResolvedSRV;
			m_SBRenderParams.DepthBufferDesc.uComplexRefValue = 0x01;
			m_SBRenderParams.DepthBufferDesc.uSimpleRefValue = 0x00;
		}

#ifdef TEST_EXTERNAL_MAPS
		if (m_SBRenderParams.eTechniqueType == GFSDK_ShadowLib_TechniqueType_PCSS &&
			m_ESMDesc.Desc.eMapType == GFSDK_ShadowLib_MapType_Texture &&
			m_ESMDesc.Desc.eViewType == GFSDK_ShadowLib_ViewType_Single)
		{
			m_ESMDesc.Desc.bDownsample = true;
		}

		memcpy(&m_ESMDesc.m4x4LightViewMatrix, &m_matShadowView, sizeof(gfsdk_float4x4));
		memcpy(&m_ESMDesc.m4x4LightProjMatrix[0], &m_matShadowProj[0], sizeof(gfsdk_float4x4) * 4);
		//memcpy(&m_ESMDesc.fBiasZ[0], &m_fCascadePartitionsFrustum[0], sizeof(float) * 4);
		// = m_pShadowLibCtx->DevModeToggleDebugCascadeShader(m_pShadowBufferHandle, true);

		retCode = m_pShadowLibCtx->RenderBufferUsingExternalMap(&m_ESMDesc, &m_ShadowMapSRV, m_pShadowBufferHandle, &m_SBRenderParams);
#else
		m_pShadowLibCtx->RenderBuffer(m_pShadowMapHandle, m_pShadowBufferHandle, &m_SBRenderParams);
#endif
		retCode = m_pShadowLibCtx->FinalizeBuffer(m_pShadowBufferHandle, &m_ShadowBufferSRV);

		if (m_AmbientOcclusion)
		{
#if defined(_PROFILE) | defined(_DEBUG)
			GPUTimer timer(&g_TimestampQueries, m_DeviceAndContext.pDeviceContext, GPU_TIME_AO_PASS);
#endif
			if (m_ShadowBufferRTV == nullptr)
			{
				ID3D11Resource* resource;
				m_ShadowBufferSRV.pSRV->GetResource(&resource);
				D3D11_RENDER_TARGET_VIEW_DESC rtvDesc =
				{
					DXGI_FORMAT_R32_FLOAT,
					D3D11_RTV_DIMENSION_TEXTURE2D
				};
				m_DeviceAndContext.pD3DDevice->CreateRenderTargetView(resource, &rtvDesc, &m_ShadowBufferRTV);
				SAFE_RELEASE(resource);
			}

			GFSDK_SSAO_InputData_D3D11 Input;
			Input.DepthData.DepthTextureType = GFSDK_SSAO_HARDWARE_DEPTHS; 
			if (m_SBDesc.uSampleCount > 1)
				Input.DepthData.pFullResDepthTextureSRV = pDSResolvedSRV;
			else
				Input.DepthData.pFullResDepthTextureSRV = pDSSRV;
			XMMATRIX transProjection = XMMatrixTranspose(m_pCamera->GetCameraProjection());
			XMStoreFloat4x4((XMFLOAT4X4*)&Input.DepthData.ProjectionMatrix.Data, transProjection);
			Input.DepthData.ProjectionMatrix.Layout = GFSDK_SSAO_COLUMN_MAJOR_ORDER;
			Input.DepthData.MetersToViewSpaceUnits = 0.5f;

			GFSDK_SSAO_Status status;
			status = m_AOContext->RenderAO(m_DeviceAndContext.pDeviceContext, &Input, &m_AOParams, m_ShadowBufferRTV, GFSDK_SSAO_RENDER_AO);
			assert(status == GFSDK_SSAO_OK);
		}
	}


	//--------------------------------------------------------------------------------------
	// Shadow map render function
	//--------------------------------------------------------------------------------------
	void ShadowManager::ShadowMapRenderFunction(void* pParams, gfsdk_float4x4* pViewProj)
	{
		ID3D11DeviceContext* context = (ID3D11DeviceContext*)pParams;

		XMMATRIX ViewProj = XMLoadFloat4x4((XMFLOAT4X4*)pViewProj);

		//g_Engine->mpConstantBufferFrame.Data.mViewProjection = XMMatrixTranspose(ViewProj);
		//g_Engine->mpConstantBufferFrame.ApplyChanges(context);

		//g_Engine->RenderDepthOnly(context, 0, ViewProj);
	}
}