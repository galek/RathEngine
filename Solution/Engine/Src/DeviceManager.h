#pragma once
#include "AssetLibrary.h"
#include "EngineState.h"

namespace Rath
{
	class DeviceManager
	{
	public:
		typedef std::function<void(ID3D11Device*)>	BoundCreateDeviceFunction;
		typedef std::function<void(void)>			BoundDestroyDeviceFunction;
		typedef std::function<void(ID3D11Device*)>	BoundCreateResourcesFunction;
	protected:
		std::unique_ptr<AssetLibrary>	m_library;
		// Application state
		HWND                        m_window;
		DXGI_SURFACE_DESC			m_backBufferDesc;
		bool                        m_fullScreen;
		bool                        m_vsync;
		DXGI_RATIONAL               m_refreshRate;
		uint32                      m_numVSYNCIntervals;

		// Direct3D Objects
		D3D_FEATURE_LEVEL                               m_featureLevel;
		Microsoft::WRL::ComPtr<ID3D11Device>            m_d3dDevice;
		Microsoft::WRL::ComPtr<ID3D11Device1>           m_d3dDevice1;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext>     m_d3dContext;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext1>    m_d3dContext1;

#ifndef NDEBUG
		Microsoft::WRL::ComPtr<ID3D11Debug>				m_d3dDebug;
#endif

		Microsoft::WRL::ComPtr<IDXGIDevice1>			m_dxgiDevice;
		Microsoft::WRL::ComPtr<IDXGIFactory1>           m_dxgiFactory;
		Microsoft::WRL::ComPtr<IDXGIFactory2>           m_dxgiFactory2;
		Microsoft::WRL::ComPtr<IDXGIAdapter>            m_dxgiAdapter;
		Microsoft::WRL::ComPtr<IDXGIOutput>             m_dxgiOutput;

		// Rendering resources
		Microsoft::WRL::ComPtr<IDXGISwapChain>          m_swapChain;
		Microsoft::WRL::ComPtr<IDXGISwapChain1>         m_swapChain1;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>  m_renderTargetView;

		std::vector<BoundCreateDeviceFunction>			m_CreateDeviceCallbacks;
		std::vector<BoundDestroyDeviceFunction>			m_DestroyDeviceCallbacks;
		std::vector<BoundCreateResourcesFunction>		m_CreateResourcesCallbacks;

		void	GetFittingSwapchain();

		void	OnDeviceLost();
	public:
		DeviceManager(EngineState* state);
		~DeviceManager();

		void CreateDevice();
		void CreateResources();

		void WindowSizeChanged(uint32 width, uint32 height);

		// Rendering helpers
		void Clear();
		void Present();

		// Initialization and management
		void Initialize(HWND window, uint32 width, uint32 height);

		ID3D11Device*               Device() const				{ return m_d3dDevice.Get(); };
		ID3D11DeviceContext*        ImmediateContext() const	{ return m_d3dContext.Get(); };
		IDXGISwapChain*             SwapChain() const			{ return m_swapChain.Get(); };
		ID3D11RenderTargetView*     BackBuffer() const			{ return m_renderTargetView.Get(); };

		template<class ClassT>
		void RegisterCreateDeviceCallback(void(ClassT::*CreateDeviceFunction)(ID3D11Device*), ClassT* c)
		{
			BoundCreateDeviceFunction bound_member_fn = std::bind(CreateDeviceFunction, c, std::placeholders::_1);
			m_CreateDeviceCallbacks.emplace_back(bound_member_fn);
		};

		template<class ClassT>
		void RegisterDestroyDeviceCallback(void(ClassT::*DestroyDeviceFunction)(void), ClassT* c)
		{
			BoundDestroyDeviceFunction bound_member_fn = std::bind(DestroyDeviceFunction, c);
			m_DestroyDeviceCallbacks.emplace_back(bound_member_fn);
		};

		template<class ClassT>
		void RegisterCreateResourcesCallback(void(ClassT::*CreateResourcesFunction)(ID3D11Device*), ClassT* c)
		{
			BoundCreateResourcesFunction bound_member_fn = std::bind(CreateResourcesFunction, c, std::placeholders::_1);
			m_CreateResourcesCallbacks.emplace_back(bound_member_fn);
		};
	};
}
