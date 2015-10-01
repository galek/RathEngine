#pragma once
#include "EngineState.h"
#include "Scene.h"
#include "StepTimer.h"

#include "NodeController.h"
#include "PhysicsManager.h"

namespace Rath
{
	class ShadowManager;
	class Window;
	class Device;
	class BaseEngine : public PhysicsManager
	{
	protected:
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
	private:
		class Impl;
		std::unique_ptr<Impl> pImpl;
		// Game state
		DX::StepTimer	m_Timer;

		bool			m_in_sizemove;
			
		void Update(DX::StepTimer const& timer);
		// Basic game loop
		void Tick();
	protected:
		EngineState		m_State;
		bool            m_suspended;

		ID3D11Device*			GetDevice() const;
		ID3D11DeviceContext*	GetDeviceContext() const;

		// Messages
		void OnActivated();
		void OnDeactivated();
		void OnSuspending();
		void OnResuming();
		void OnWindowSizeChanged(uint32 width, uint32 height);

		void virtual CreateDevice(ID3D11Device* device);
		void virtual CreateResources(ID3D11Device* device);
		void virtual RenderScene(ID3D11DeviceContext* context, const XMMATRIX& mViewProj, Scene::RenderPass pass);
		void virtual FrameMove(float fElapsedTime);

		LRESULT	MessageHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	public:
		BaseEngine(LPCWSTR appName,
			LPCWSTR iconResource = NULL,
			LPCWSTR smallIconResource = NULL,
			LPCWSTR menuResource = NULL,
			LPCWSTR accelResource = NULL);
		~BaseEngine();

		int Run();

		void PrintText(LPCWCHAR text, const XMFLOAT2& position, FLOAT size = 1.f, const FLOAT ColorRGBA[4] = DirectX::Colors::White, FLOAT depth = 0.f);
		void PrintText(LPCWCHAR text, const XMINT2& position, UINT size = 16, const FLOAT ColorRGBA[4] = DirectX::Colors::White, FLOAT depth = 0.f);
	};
}
