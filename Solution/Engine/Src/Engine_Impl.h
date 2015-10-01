#include "BaseEngine.h"

#include "ShadowManager.h"
#include "DeviceManager.h"
#include "BackBuffer.h"
#include "Window.h"
#include "UI/UIManager.h"

#include "Debug/TimestampQueries.h"
#include "Debug/MemoryMonitor.h"

namespace Rath
{
	class BaseEngine::Impl
	{
	public:
		Impl(BaseEngine* owner, 
			EngineState* state, 
			LPCWSTR appName,
			DWORD style = WS_CAPTION | WS_OVERLAPPED | WS_SYSMENU,
			DWORD exStyle = WS_EX_APPWINDOW,
			LPCWSTR iconResource = NULL,
			LPCWSTR smallIconResource = NULL,
			LPCWSTR menuResource = NULL,
			LPCWSTR accelResource = NULL);
		~Impl();

		void CreateSamplers(ID3D11Device* device);
		void CreateRasterizer(ID3D11Device* device);
		void CreateDevice(ID3D11Device* device);
		void CreateResources(ID3D11Device* device);
		void WindowSizeChanged(uint32 width, uint32 height);

		void Run(std::function<void(void)> TickFunction);
		void Update(float elapsedTime);
		void Render();

		void Render_Background_pass(ID3D11DeviceContext* context);
		void Render_Depth_pass(ID3D11DeviceContext* context);
		void Render_Shadow_pass(ID3D11DeviceContext* context);
		void Render_Scene_pass(ID3D11DeviceContext* context);
		void Render_PostProcess_pass(ID3D11DeviceContext* context);
		void Render_UI_pass(ID3D11DeviceContext* context);

		Window			m_Window;
		DeviceManager	m_DeviceManager;
		ShadowManager	m_ShadowManager;
		BackBuffer		m_BackBuffer;
		UIManager		m_UIManager;
		BaseEngine*     m_pOwner;
		EngineState*	m_pState;

		ConstantBuffer<GLOBAL_CB_STRUCT>				m_ConstantBufferFrame;

		Microsoft::WRL::ComPtr<ID3D11SamplerState>		m_SamplerPoint;
		Microsoft::WRL::ComPtr<ID3D11SamplerState>		m_SamplerLinear;
		Microsoft::WRL::ComPtr<ID3D11SamplerState>		m_SamplerAnisotropic;
		Microsoft::WRL::ComPtr<ID3D11SamplerState>		m_SamplerBilinear;
		Microsoft::WRL::ComPtr<ID3D11SamplerState>		m_SamplerLinearWrap;

		Microsoft::WRL::ComPtr<ID3D11RasterizerState>	m_SceneRS;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState>	m_ShadowRS;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState>	m_DepthPrepassRS;

		static BaseEngine::Impl* s_engine;

		BOOL IsMinimized();
	};
}