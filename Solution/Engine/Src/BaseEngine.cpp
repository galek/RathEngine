#include "pch.h"
#include "BaseEngine.h"

#include "Engine_Impl.h"
#include "PhysicsManager.h"

#include "GamePad.h"
#include "Keyboard.h"
#include "Mouse.h"

std::unique_ptr<GamePad>	g_gamepad(new GamePad);
std::unique_ptr<Keyboard>	g_keyboard(new Keyboard);
std::unique_ptr<Mouse>		g_mouse(new Mouse);

namespace Rath
{
	void BaseEngine::GLOBAL_CB_STRUCT::BindViewAndProj(const XMMATRIX& view, const XMMATRIX& proj)
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

	BaseEngine::BaseEngine(LPCWSTR appName,
		LPCWSTR iconResource,
		LPCWSTR smallIconResource,
		LPCWSTR menuResource,
		LPCWSTR accelResource ) :
		m_in_sizemove(false)
	{
		HRESULT hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);
		//if (FAILED(hr))
		//	return 1;

		pImpl.reset(new Impl(this, &m_State, 
			appName, 
			WS_CAPTION | WS_OVERLAPPED | WS_SYSMENU | WS_SIZEBOX | WS_MAXIMIZEBOX | WS_MINIMIZEBOX, //WS_POPUP
			WS_EX_APPWINDOW, 
			iconResource, smallIconResource, 
			menuResource, accelResource));
	}

	BaseEngine::~BaseEngine()
	{
		pImpl.reset();

		CoUninitialize();
	}

	ID3D11Device* BaseEngine::GetDevice() const
	{
		return pImpl->m_DeviceManager.Device();
	}

	ID3D11DeviceContext* BaseEngine::GetDeviceContext() const
	{
		return pImpl->m_DeviceManager.ImmediateContext();
	}

	void BaseEngine::CreateDevice(ID3D11Device* device)
	{
	}

	void BaseEngine::CreateResources(ID3D11Device* device)
	{
	}

	// Executes basic game loop.
	void BaseEngine::Tick()
	{
		m_Timer.Tick([&]()
		{
			Update(m_Timer);
		});

		if (m_Timer.GetFrameCount() == 0)
			return;

		pImpl->Render();
	}

	// Updates the world
	void BaseEngine::Update(DX::StepTimer const& timer)
	{
		float elapsedTime = float(timer.GetElapsedSeconds());

		//m_BackBuffer.FrameMove(elapsedTime);
		PhysicsManager::FrameMove(elapsedTime);

		FrameMove(elapsedTime);

		XMVECTOR eyePt = m_State.m_Camera.GetPosition();
		XMVECTOR lootAt = XMVectorSubtract(eyePt, m_State.m_LightDirection * 20.0f);
		m_State.m_Light.SetViewParams(eyePt, lootAt);

		pImpl->Update(elapsedTime);
	}

	void BaseEngine::FrameMove(float fElapsedTime)
	{

	}

	int BaseEngine::Run()
	{
		pImpl->Run(std::bind(&BaseEngine::Tick, this));

		return 0;
	}

	void BaseEngine::PrintText(LPCWCHAR text, const XMFLOAT2& position, FLOAT size, const FLOAT ColorRGBA[4], FLOAT depth)
	{
		pImpl->m_UIManager.PrintText(text, position, size, ColorRGBA, depth);
	}

	void BaseEngine::PrintText(LPCWCHAR text, const XMINT2& position, UINT size, const FLOAT ColorRGBA[4], FLOAT depth)
	{
		pImpl->m_UIManager.PrintText(text, position, size, ColorRGBA, depth);
	}

	// Message handlers
	void BaseEngine::OnActivated()
	{
		// TODO: Game is becoming active window
	}

	void BaseEngine::OnDeactivated()
	{
		// TODO: Game is becoming background window
	}

	void BaseEngine::OnSuspending()
	{
		m_suspended = true;

		// TODO: Game is being power-suspended (or minimized)
	}

	void BaseEngine::OnResuming()
	{
		m_suspended = false;

		m_Timer.ResetElapsedTime();

		// TODO: Game is being power-resumed (or returning from minimize)
	}

	void BaseEngine::OnWindowSizeChanged(uint32 width, uint32 height)
	{
		if ((m_State.m_BackBufferDesc.Width != width) || 
			(m_State.m_BackBufferDesc.Height != height))
		{
			m_State.WindowSizeChanged(width, height);
			pImpl->WindowSizeChanged(width, height);
		}
	}

	LRESULT	BaseEngine::MessageHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_SIZE:
			if (wParam == SIZE_MINIMIZED)
			{
				if (!pImpl->IsMinimized())
				{
					if (!m_suspended)
						OnSuspending();
				}
			}
			else if (pImpl->IsMinimized())
			{
				if (m_suspended)
					OnResuming();
			}
			else if (!m_in_sizemove)
			{
				OnWindowSizeChanged(LOWORD(lParam), HIWORD(lParam));
			}
			break;

			case WM_ENTERSIZEMOVE:
				m_in_sizemove = true;
				break;

		case WM_EXITSIZEMOVE:
			m_in_sizemove = false;
			RECT rc;
			GetClientRect(hWnd, &rc);

			OnWindowSizeChanged(rc.right - rc.left, rc.bottom - rc.top);
			break;

		case WM_GETMINMAXINFO:
		{
			auto info = reinterpret_cast<MINMAXINFO*>(lParam);
			info->ptMinTrackSize.x = 320;
			info->ptMinTrackSize.y = 200;
		}
		break;

		case WM_ACTIVATEAPP:
			if (wParam)
				OnActivated();
			else
				OnDeactivated();
			break;

		case WM_POWERBROADCAST:
			switch (wParam)
			{
			case PBT_APMQUERYSUSPEND:
				if (!m_suspended)
					OnSuspending();
				return true;

			case PBT_APMRESUMESUSPEND:
				if (!pImpl->IsMinimized())
				{
					if (m_suspended)
						OnResuming();
				}
				return true;
			}
			break;

		case WM_KEYDOWN:
			if (wParam == VK_ESCAPE)
				pImpl->m_Window.Destroy();
			break;
		}

		g_mouse->ProcessMessage(uMsg, wParam, lParam);
		g_keyboard->ProcessMessage(uMsg, wParam, lParam);

		return 0;
	}

	void BaseEngine::RenderScene(ID3D11DeviceContext* context, const XMMATRIX& mViewProj, Scene::RenderPass pass)
	{

	}
}