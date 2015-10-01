#pragma once
#include "Camera.h"

namespace Rath
{
	class EngineState
	{
	public:
		OptimizedCamera					m_Camera;
		OrthographicCamera				m_Light;
		DirectX::SimpleMath::Vector3	m_LightDirection;

		D3D11_VIEWPORT					m_Viewport;
		DXGI_SURFACE_DESC				m_BackBufferDesc;
		FLOAT							m_aspect;
		FLOAT							m_fovy;
		FLOAT							m_znear;
		FLOAT							m_zfar;

		bool							m_LightShafts;
		bool							m_DepthOfField;
		bool							m_Bloom;
		bool							m_AmbientOcclusion;

		bool							m_Fxaa;

		bool							m_fullScreen;
		bool							m_vsync;
		DXGI_RATIONAL					m_refreshRate;

		UINT							m_ShadowMapSize;
		UINT							m_ShadowMapCount;
		UINT							m_ShadowMapQuality;
		UINT							m_ShadowMapType;

#if defined(_PROFILE) | defined(_DEBUG)
		bool							m_displayShadowMaps;
		bool							m_displayShadowFrustum;
		bool							m_displayShadowBuffer;
#endif
		EngineState();
		~EngineState();

		void		WindowSizeChanged(uint32 width, uint32 height);
	};
}