#include "pch.h"
#include "BrowserManager.h"

#include <include/cef_app.h>
#include <include/cef_client.h>
#include <include/cef_render_handler.h>

// When generating projects with CMake the CEF_USE_SANDBOX value will be defined
// automatically if using the required compiler version. Pass -DUSE_SANDBOX=OFF
// to the CMake command-line to disable use of the sandbox.
// Uncomment this line to manually enable sandbox support.
// #define CEF_USE_SANDBOX 1
#ifdef _WIN64
#if defined(CEF_USE_SANDBOX)
// The cef_sandbox.lib static library is currently built with VS2013. It may not
// link successfully with other VS versions.
#pragma comment(lib, "..\\cef3\\lib64\\cef_sandbox.lib")
#endif
#pragma comment(lib, "..\\cef3\\lib64\\libcef.lib")
#elif

#endif

namespace Rath
{
	class RenderHandler : public CefRenderHandler, public Texture
	{
	protected:
		class BrowserClient : public CefClient
		{
		public:
			BrowserClient(RenderHandler *renderHandler) :
				m_renderHandler(renderHandler)
			{
			}

			virtual CefRefPtr<CefRenderHandler> GetRenderHandler()
			{
				return m_renderHandler;
			}

			CefRefPtr<CefRenderHandler> m_renderHandler;

			IMPLEMENT_REFCOUNTING(BrowserClient);
		};

		int	width;
		int	height;
		ID3D11DeviceContext*		pDeviceContext;

		CefRefPtr<BrowserClient>	browserClient;
		CefRefPtr<CefBrowser>		browser;
	public:
		RenderHandler(int w, int h, const std::string& webpage) :
			width(w),
			height(h),
			pDeviceContext(nullptr)
		{
			AddRef();

			CefWindowInfo window_info;
			CefBrowserSettings browserSettings;
			HWND windowHandle = 0;
			window_info.SetAsWindowless(windowHandle, false);

			browserClient = new BrowserClient(this);
			browser = CefBrowserHost::CreateBrowserSync(window_info, browserClient.get(), webpage, browserSettings, nullptr);
		}

		RenderHandler(int w, int h, const std::string& webpage, ID3D11Device* device, ID3D11DeviceContext* context) :
			RenderHandler(w, h, webpage)
		{
			Create(device, context);
		}

		void Create(ID3D11Device* device, ID3D11DeviceContext* context)
		{
			SAFE_RELEASE(m_texture);
			SAFE_RELEASE(m_textureSRV);

			pDeviceContext = context;

			CD3D11_TEXTURE2D_DESC Desc(DXGI_FORMAT_R8G8B8A8_UNORM, width, height);
			device->CreateTexture2D(&Desc, nullptr, (ID3D11Texture2D**)&m_texture);

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = { DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_SRV_DIMENSION_TEXTURE2D };
			srvDesc.Texture3D.MipLevels = 1;
			srvDesc.Texture3D.MostDetailedMip = 0;
			device->CreateShaderResourceView(m_texture, &srvDesc, &m_textureSRV);
		}

		bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect)
		{
			rect = CefRect(0, 0, width, height);
			return true;
		}

		void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList &dirtyRects, const void *buffer, int width, int height)
		{
			D3D11_BOX destRegion = { 0, 0, 0, (UINT)width, (UINT)height, 1 };
			pDeviceContext->UpdateSubresource(m_texture, 0, &destRegion, buffer, width * sizeof(uint32), 0);
		}

		// CefBase interface
		IMPLEMENT_REFCOUNTING(RenderHandler);
	};

	BrowserManager::BrowserManager(HINSTANCE hInstance) :
		m_Device(nullptr),
		m_DeferredUpdateContext(nullptr)
	{
		void* sandbox_info = NULL;

#if defined(CEF_USE_SANDBOX)
		// Manage the life span of the sandbox information object. This is necessary
		// for sandbox support on Windows. See cef_sandbox_win.h for complete details.
		CefScopedSandboxInfo scoped_sandbox;
		sandbox_info = scoped_sandbox.sandbox_info();
#endif
		// Provide CEF with command-line arguments.
		CefMainArgs main_args(hInstance);

		// CEF applications have multiple sub-processes (render, plugin, GPU, etc)
		// that share the same executable. This function checks the command-line and,
		// if this is a sub-process, executes the appropriate logic.
		int exit_code = CefExecuteProcess(main_args, nullptr, sandbox_info);
		if (exit_code >= 0) 
		{
			// The sub-process has completed so return here.
			throw exit_code;
		}

		// Specify CEF global settings here.
		CefSettings settings;
#if !defined(CEF_USE_SANDBOX)
		settings.no_sandbox = true;
#endif

		// Initialize CEF.
		CefInitialize(main_args, settings, nullptr, sandbox_info);
	}

	BrowserManager::~BrowserManager()
	{
		SAFE_RELEASE(m_DeferredUpdateContext);

		CefShutdown();
		for (auto it : m_Textures) if (it)
		{
			RenderHandler* tex = (RenderHandler*)it;
			delete tex;
		}
	}

	void BrowserManager::CreateRecources(ID3D11Device* device)
	{
		SAFE_RELEASE(m_DeferredUpdateContext);

		m_Device = device;
		device->CreateDeferredContext(0, &m_DeferredUpdateContext);

		for (auto it : m_Textures)
		{
			RenderHandler* tex = (RenderHandler*)it;
			tex->Create(device, m_DeferredUpdateContext);
		}
	}

	void BrowserManager::Update(ID3D11DeviceContext* context)
	{
		ID3D11CommandList* commandlist;
		m_DeferredUpdateContext->FinishCommandList(false, &commandlist);
		context->ExecuteCommandList(commandlist, true);
		commandlist->Release();
	}

	Texture* BrowserManager::CreateBrowserTexture(uint32 width, uint32 height, const std::string& webpage)
	{
		RenderHandler* renderHandler = nullptr;
		if (m_Device)
		{
			renderHandler = new RenderHandler((int)width, (int)height, webpage, m_Device, m_DeferredUpdateContext);
		}
		else
		{
			renderHandler = new RenderHandler((int)width, (int)height, webpage);
		}
		if (renderHandler)
		{
			m_Textures.push_back(renderHandler);
		}
		return renderHandler;
	}
}