#include "pch.h"
#include "Engine_Impl.h"

#include "AssetLibrary.h"

namespace Rath
{
	BaseEngine::Impl::Impl(BaseEngine* owner, EngineState* state, 
		LPCWSTR appName, DWORD style, DWORD exStyle,
		LPCWSTR iconResource, LPCWSTR smallIconResource, 
		LPCWSTR menuResource, LPCWSTR accelResource) :
		m_pOwner(owner),
		m_pState(state),
		m_ShadowManager(state, std::bind(&BaseEngine::RenderScene, owner, std::placeholders::_1, std::placeholders::_2, Scene::RenderPass::ShadowPass)),
		m_Window(state, nullptr, appName, style, exStyle, iconResource, smallIconResource, menuResource, accelResource),
		m_DeviceManager(state),
		m_BackBuffer(state)
	{
		if (s_engine)
		{
			throw std::exception("Engine is a singleton");
		}

		s_engine = this;

		m_Window.RegisterMessageCallback(&BaseEngine::MessageHandler, owner);

		m_DeviceManager.RegisterCreateDeviceCallback(&BaseEngine::Impl::CreateDevice, this);
		m_DeviceManager.RegisterCreateResourcesCallback(&BaseEngine::Impl::CreateResources, this);

		m_DeviceManager.RegisterCreateDeviceCallback(&BaseEngine::CreateDevice, owner);
		m_DeviceManager.RegisterCreateResourcesCallback(&BaseEngine::CreateResources, owner);

		m_UIManager.WindowSizeChanged(state->m_BackBufferDesc.Width, state->m_BackBufferDesc.Height);
	}

	BaseEngine::Impl::~Impl()
	{
		s_engine = nullptr;
	}

	void BaseEngine::Impl::CreateSamplers(ID3D11Device* device)
	{
		D3D11_SAMPLER_DESC samDesc;
		ZeroMemory(&samDesc, sizeof(samDesc));
		samDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samDesc.AddressU = samDesc.AddressV = samDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samDesc.MaxAnisotropy = 1;
		samDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		samDesc.MinLOD = 0;
		samDesc.MaxLOD = D3D11_FLOAT32_MAX;
		DX::ThrowIfFailed(device->CreateSamplerState(&samDesc, m_SamplerLinear.ReleaseAndGetAddressOf()));

		samDesc.AddressU = samDesc.AddressV = samDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		DX::ThrowIfFailed(device->CreateSamplerState(&samDesc, m_SamplerLinearWrap.ReleaseAndGetAddressOf()));

		samDesc.AddressU = samDesc.AddressV = samDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		DX::ThrowIfFailed(device->CreateSamplerState(&samDesc, m_SamplerPoint.ReleaseAndGetAddressOf()));

		samDesc.Filter = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
		DX::ThrowIfFailed(device->CreateSamplerState(&samDesc, m_SamplerBilinear.ReleaseAndGetAddressOf()));

		samDesc.Filter = D3D11_FILTER_ANISOTROPIC;
		samDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samDesc.MaxAnisotropy = 16;
		DX::ThrowIfFailed(device->CreateSamplerState(&samDesc, m_SamplerAnisotropic.ReleaseAndGetAddressOf()));
	}

	void BaseEngine::Impl::CreateRasterizer(ID3D11Device* device)
	{
		const D3D11_RASTERIZER_DESC SCENE_RASTERIZER =
		{
			D3D11_FILL_SOLID,//D3D11_FILL_WIREFRAME,//D3D11_FILL_MODE FillMode;
			D3D11_CULL_BACK,//D3D11_CULL_MODE CullMode;
			FALSE,//BOOL FrontCounterClockwise;
			-1024,//INT DepthBias;
			0.0f,//FLOAT DepthBiasClamp;
			0.0f,//FLOAT SlopeScaledDepthBias;
			TRUE,//BOOL DepthClipEnable;
			FALSE,//BOOL ScissorEnable;
			TRUE,//BOOL MultisampleEnable;
			FALSE//BOOL AntialiasedLineEnable;   
		};

		const D3D11_RASTERIZER_DESC SHADOW_RASTERIZER =
		{
			D3D11_FILL_SOLID,//D3D11_FILL_MODE FillMode;
			D3D11_CULL_BACK,//D3D11_CULL_MODE CullMode;
			FALSE,//BOOL FrontCounterClockwise;
			1024,//INT DepthBias;
			0.0f,//FLOAT DepthBiasClamp;
			1.0f,//FLOAT SlopeScaledDepthBias;
			TRUE,//BOOL DepthClipEnable;
			FALSE,//BOOL ScissorEnable;
			TRUE,//BOOL MultisampleEnable;
			FALSE//BOOL AntialiasedLineEnable;   
		};

		const D3D11_RASTERIZER_DESC DEPTH_RASTERIZER =
		{
			D3D11_FILL_SOLID,//D3D11_FILL_MODE FillMode;
			D3D11_CULL_BACK,//D3D11_CULL_MODE CullMode;
			FALSE,//BOOL FrontCounterClockwise;
			0,//INT DepthBias;
			0.0f,//FLOAT DepthBiasClamp;
			0.0f,//FLOAT SlopeScaledDepthBias;
			TRUE,//BOOL DepthClipEnable;
			FALSE,//BOOL ScissorEnable;
			TRUE,//BOOL MultisampleEnable;
			FALSE//BOOL AntialiasedLineEnable;   
		};

		DX::ThrowIfFailed(device->CreateRasterizerState(&SCENE_RASTERIZER, m_SceneRS.ReleaseAndGetAddressOf()));
		DX::ThrowIfFailed(device->CreateRasterizerState(&SHADOW_RASTERIZER, m_ShadowRS.ReleaseAndGetAddressOf()));
		DX::ThrowIfFailed(device->CreateRasterizerState(&DEPTH_RASTERIZER, m_DepthPrepassRS.ReleaseAndGetAddressOf()));
	}

	void BaseEngine::Impl::CreateDevice(ID3D11Device* device)
	{
		CreateSamplers(device);
		CreateRasterizer(device);

		m_UIManager.CreateDevice(device);
		m_BackBuffer.CreateDevice(device);

		m_ShadowManager.CreateDevice(device, m_DeviceManager.ImmediateContext());

		m_ConstantBufferFrame.Initialize(device);

#if defined(_PROFILE) | defined(_DEBUG)
		g_TimestampQueries.CreateDevice(device);
#endif
	}

	void BaseEngine::Impl::CreateResources(ID3D11Device* device)
	{
		m_BackBuffer.CreateResources(device, m_pOwner->m_State.m_BackBufferDesc);
		m_ShadowManager.CreateResources(device, m_pOwner->m_State.m_BackBufferDesc, m_BackBuffer.GetDepthStencilSRViewRO());
	}

	void BaseEngine::Impl::WindowSizeChanged(uint32 width, uint32 height)
	{
		m_DeviceManager.WindowSizeChanged(width, height);
		m_UIManager.WindowSizeChanged(width, height);
	}

	void BaseEngine::Impl::Run(std::function<void(void)> TickFunction)
	{
		m_Window.ShowWindow();

		m_DeviceManager.Initialize(m_Window, m_pState->m_BackBufferDesc.Width, m_pState->m_BackBufferDesc.Height);

		while (m_Window.IsAlive())
		{
			if (!m_Window.IsMinimized())
			{
				TickFunction();
			}

			m_Window.MessageLoop();
		}
	}

	void BaseEngine::Impl::Update(float elapsedTime)
	{
		XMMATRIX ViewMatrix = m_pState->m_Camera.GetCameraView();
		XMMATRIX ProjMatrix = m_pState->m_Camera.GetCameraProjection();
		m_ConstantBufferFrame.Data.BindViewAndProj(ViewMatrix, ProjMatrix);
		m_ConstantBufferFrame.Data.vViewport = XMVectorSet(m_pState->m_Viewport.Width, m_pState->m_Viewport.Height, 1.f, 1.f);

		m_BackBuffer.Update(elapsedTime);
	}

	// Draws the scene
	void BaseEngine::Impl::Render()
	{
		ID3D11DeviceContext* context = m_DeviceManager.ImmediateContext();

#if defined(_PROFILE) | defined(_DEBUG)
		// Timestamp queries need to be wraped inside a disjoint query begin/end
		g_TimestampQueries.Begin(context);
#endif

		ID3D11SamplerState* const ppSampler[] =
		{
			m_SamplerPoint.Get(),
			m_SamplerLinear.Get(),
			m_SamplerAnisotropic.Get(),
			m_SamplerBilinear.Get(),
			m_SamplerLinearWrap.Get()
		};

		context->VSSetSamplers(0, ARRAYSIZE(ppSampler), ppSampler);
		context->DSSetSamplers(0, ARRAYSIZE(ppSampler), ppSampler);
		context->HSSetSamplers(0, ARRAYSIZE(ppSampler), ppSampler);
		context->GSSetSamplers(0, ARRAYSIZE(ppSampler), ppSampler);
		context->PSSetSamplers(0, ARRAYSIZE(ppSampler), ppSampler);
		context->CSSetSamplers(0, ARRAYSIZE(ppSampler), ppSampler);

		//--------------------------------------------------------------------------------------
		// STEP 0: Clear Buffer
		//--------------------------------------------------------------------------------------
		m_DeviceManager.Clear();
		m_BackBuffer.Clear(context);

		m_ConstantBufferFrame.ApplyChanges(context);
		m_ConstantBufferFrame.SetVS(context, 0);
		m_ConstantBufferFrame.SetHS(context, 0);
		m_ConstantBufferFrame.SetDS(context, 0);
		m_ConstantBufferFrame.SetPS(context, 0);


		//--------------------------------------------------------------------------------------
		// STEP 1: Depth Pre-Pass
		//--------------------------------------------------------------------------------------
		Render_Depth_pass(context);

		//--------------------------------------------------------------------------------------
		// STEP 2: Sky and Background-Pass
		//--------------------------------------------------------------------------------------
		Render_Background_pass(context);

		//--------------------------------------------------------------------------------------
		// STEP 3: Render shadow maps
		//--------------------------------------------------------------------------------------
		Render_Shadow_pass(context);

		//--------------------------------------------------------------------------------------
		// STEP 4: Forward render scene using shadow buffer 
		//--------------------------------------------------------------------------------------
		Render_Scene_pass(context);

		//--------------------------------------------------------------------------------------
		// STEP 5: Post Process Pass
		//--------------------------------------------------------------------------------------
		Render_PostProcess_pass(context);

#if defined(_PROFILE) | defined(_DEBUG)
		if (m_pState->m_displayShadowFrustum)
			m_ShadowManager.DisplayMapFrustums(m_DeviceManager.BackBuffer(), m_BackBuffer.GetDepthStencilSRViewRO());
		if (m_pState->m_displayShadowBuffer)
			m_ShadowManager.DisplayShadowBuffer(m_DeviceManager.BackBuffer());
		if (m_pState->m_displayShadowMaps)
			m_ShadowManager.DisplayShadowMaps(m_DeviceManager.BackBuffer(), 400, m_pState->m_BackBufferDesc.Height);
#endif

		//--------------------------------------------------------------------------------------
		// STEP 6: UI Pass
		//--------------------------------------------------------------------------------------
		Render_UI_pass(context);

		m_DeviceManager.Present();
	}

	void BaseEngine::Impl::Render_Background_pass(ID3D11DeviceContext* context)
	{
		DEBUGTIMER(GPU_TIME_BACKGROUND_PASS);

		XMMATRIX ViewProjMatrix = m_pState->m_Camera.GetCameraViewProjection();

		m_pOwner->RenderScene(context, ViewProjMatrix, Scene::RenderPass::BackgroundPass);

		m_BackBuffer.ResolveDepth(context);
		m_BackBuffer.ReduceDepth(context);
		m_pState->m_Camera.Optimize(m_BackBuffer.GetResolvedDepth());
		m_BackBuffer.DetectComplexPixels(context);
	}

	void BaseEngine::Impl::Render_Depth_pass(ID3D11DeviceContext* context)
	{
		DEBUGTIMER(GPU_TIME_DEPTH_PRE_PASS);

		XMMATRIX ViewProjMatrix = m_pState->m_Camera.GetCameraViewProjection();

		context->RSSetState(m_DepthPrepassRS.Get());
		m_pOwner->RenderScene(context, ViewProjMatrix, Scene::RenderPass::DepthOnlyPass);
	}

	void BaseEngine::Impl::Render_Shadow_pass(ID3D11DeviceContext* context)
	{
		DEBUGTIMER(GPU_TIME_SHADOW_MAPS);

		context->RSSetState(m_ShadowRS.Get());
		m_ShadowManager.RenderShadowMaps(context);

		m_ShadowManager.RenderShadowBuffer(
			m_BackBuffer.GetDepthStencilView(),
			m_BackBuffer.GetResolvedDepthStencilView());


	}

	void BaseEngine::Impl::Render_Scene_pass(ID3D11DeviceContext* context)
	{
		DEBUGTIMER(GPU_TIME_SCENE);

		m_BackBuffer.Begin(context);

		XMMATRIX ViewProjMatrix = m_pState->m_Camera.GetOptCameraViewProjection();

		m_ConstantBufferFrame.SetVS(context, 0);
		m_ConstantBufferFrame.SetHS(context, 0);
		m_ConstantBufferFrame.SetDS(context, 0);
		m_ConstantBufferFrame.SetPS(context, 0);

		// Set the shadow buffer as a SRV
		ID3D11ShaderResourceView* SRV_Array[] = { m_ShadowManager.GetShadowBuffer() };
		context->PSSetShaderResources(5, 1, SRV_Array);
		context->RSSetState(m_SceneRS.Get());

		m_pOwner->RenderScene(context, ViewProjMatrix, Scene::RenderPass::ScenePass);

		SRV_Array[0] = nullptr;
		context->PSSetShaderResources(5, 1, SRV_Array);
	}

	void BaseEngine::Impl::Render_PostProcess_pass(ID3D11DeviceContext* context)
	{
		m_BackBuffer.End(context, m_DeviceManager.BackBuffer());
	}

	void BaseEngine::Impl::Render_UI_pass(ID3D11DeviceContext* context)
	{
		XMMATRIX ViewProjMatrix = m_pState->m_Camera.GetOptCameraViewProjection();

#if defined(_PROFILE) | defined(_DEBUG)
		g_TimestampQueries.End(context, &g_RenderTimes);

		MemoryMonitor::MemInfo memInfo;
		g_MemoryMonitor.GetMemoryInfo(&memInfo);

		float base = (float)(log(memInfo.PagefileUsage) / log(1024));
		char suffixes[] = { 'b', 'k', 'M', 'G', 'T' };

		auto print = [&](int x, int y, const wchar_t * _Format, ...)
		{
			WCHAR buffer[_MAX_PATH];
			va_list argptr;
			va_start(argptr, _Format);
			_vsnwprintf_s(buffer, _MAX_PATH, _Format, argptr);
			va_end(argptr);
			m_UIManager.PrintText(buffer, XMINT2(x + 1, y + 1), 16, DirectX::Colors::Black);
			m_UIManager.PrintText(buffer, XMINT2(x, y ), 16, DirectX::Colors::White);
		};

		int y = 44;
		print(19, 19, L"FPS: %d", m_pOwner->m_Timer.GetFramesPerSecond());
		print(19, y, L"Depth-Pass: %.2fms", g_RenderTimes.GpuTimeMS[GPU_TIME_DEPTH_PRE_PASS]);
		print(19, y += 20, L"Shadowmaps: %.2fms", g_RenderTimes.GpuTimeMS[GPU_TIME_SHADOW_MAPS]);
		print(19, y += 20, L"Scene-Pass: %.2fms", g_RenderTimes.GpuTimeMS[GPU_TIME_SCENE]);
		print(19, y += 20, L"AO-Pass: %.2fms", g_RenderTimes.GpuTimeMS[GPU_TIME_AO_PASS]);
		print(19, y += 20, L"Bloom-Pass: %.2fms", g_RenderTimes.GpuTimeMS[GPU_TIME_BLOOM_PASS]);
		print(19, y += 20, L"DoF-Pass: %.2fms", g_RenderTimes.GpuTimeMS[GPU_TIME_DOF_PASS]);
		print(19, y += 20, L"LS-Pass: %.2fms", g_RenderTimes.GpuTimeMS[GPU_TIME_LIGHTSHAFTS_PASS]);
		print(19, y += 20, L"AA-Pass: %.2fms", g_RenderTimes.GpuTimeMS[GPU_TIME_AA_PASS]);
		print(19, y += 20, L"GPU Memory: %dMB / %dMB", memInfo.CurrentAvailableDedicatedVideoMemoryInMB, memInfo.AvailableDedicatedVideoMemoryInMB);
		print(19, y += 20, L"CPU Memory: %6.2f%cB", pow(1024, base - floor(base)), suffixes[static_cast<int>(floor(base))]);

		m_UIManager.Render(context);
#endif

		m_pOwner->RenderScene(context, ViewProjMatrix, Scene::RenderPass::UIPass);
	}

	BOOL BaseEngine::Impl::IsMinimized()
	{
		return m_Window.IsMinimized();
	}

	BaseEngine::Impl* BaseEngine::Impl::s_engine = nullptr;
}