#include "pch.h"
#include "Technique.h"

#include <D3Dcompiler.h>
#include <Shlwapi.h>

namespace Rath
{
	bool TechniqueSetting::operator==(const TechniqueSetting &other) const
	{
		return strcmp(Name, other.Name) == 0;
	}

	bool TechniqueSetting::operator==(const char* name) const
	{
		return strcmp(Name, name) == 0;
	}

	bool TechniqueSetting::operator<(const TechniqueSetting &other) const
	{
		return strcmp(Name, other.Name) < 0;
	}

	bool TechniqueSetting::operator<(const char* name) const
	{
		return strcmp(Name, name) < 0;
	}

	const D3D11_RASTERIZER_DESC DEFAULT_RASTERIZER =
	{
		D3D11_FILL_SOLID,//D3D11_FILL_MODE FillMode;
		D3D11_CULL_BACK,//D3D11_CULL_MODE CullMode;
		FALSE,//BOOL FrontCounterClockwise;
		0,//INT DepthBias;
		0.0,//FLOAT DepthBiasClamp;
		0.0,//FLOAT SlopeScaledDepthBias;
		TRUE,//BOOL DepthClipEnable;
		FALSE,//BOOL ScissorEnable;
		TRUE,//BOOL MultisampleEnable;
		FALSE//BOOL AntialiasedLineEnable;   
	};

	const D3D11_BLEND_DESC UI_BLEND =
	{
		FALSE, //BOOL AlphaToCoverageEnable;
		FALSE, //BOOL IndependentBlendEnable;
		{
			{
				TRUE, //BOOL BlendEnable;
				D3D11_BLEND_SRC_ALPHA, //D3D11_BLEND SrcBlend;
				D3D11_BLEND_INV_SRC_ALPHA, //D3D11_BLEND DestBlend;
				D3D11_BLEND_OP_ADD, //D3D11_BLEND_OP BlendOp;
				D3D11_BLEND_ZERO, //D3D11_BLEND SrcBlendAlpha;
				D3D11_BLEND_ZERO, //D3D11_BLEND DestBlendAlpha;
				D3D11_BLEND_OP_ADD, //D3D11_BLEND_OP BlendOpAlpha;
				D3D11_COLOR_WRITE_ENABLE_ALL, //UINT8 RenderTargetWriteMask;
			},
		}
	};

	const D3D11_BLEND_DESC DEFAULT_BLEND =
	{
		FALSE, //BOOL AlphaToCoverageEnable;
		FALSE, //BOOL IndependentBlendEnable;
		{
			{
				FALSE, //BOOL BlendEnable;
				D3D11_BLEND_ONE, //D3D11_BLEND SrcBlend;
				D3D11_BLEND_ZERO, //D3D11_BLEND DestBlend;
				D3D11_BLEND_OP_ADD, //D3D11_BLEND_OP BlendOp;
				D3D11_BLEND_ONE, //D3D11_BLEND SrcBlendAlpha;
				D3D11_BLEND_ZERO, //D3D11_BLEND DestBlendAlpha;
				D3D11_BLEND_OP_ADD, //D3D11_BLEND_OP BlendOpAlpha;
				D3D11_COLOR_WRITE_ENABLE_ALL, //UINT8 RenderTargetWriteMask;
			},
		}
	};

	const D3D11_DEPTH_STENCIL_DESC UI_DEPTHSTENCIL =
	{
		TRUE,
		D3D11_DEPTH_WRITE_MASK_ALL,
		D3D11_COMPARISON_LESS_EQUAL,
		FALSE,
		D3D11_DEFAULT_STENCIL_READ_MASK,
		D3D11_DEFAULT_STENCIL_WRITE_MASK,
		D3D11_STENCIL_OP_KEEP,
		D3D11_STENCIL_OP_KEEP,
	};

	const D3D11_DEPTH_STENCIL_DESC SCENE_DEPTHSTENCIL =
	{
		TRUE,
		D3D11_DEPTH_WRITE_MASK_ALL,
		D3D11_COMPARISON_LESS_EQUAL,
		FALSE,
		D3D11_DEFAULT_STENCIL_READ_MASK,
		D3D11_DEFAULT_STENCIL_WRITE_MASK,
		D3D11_STENCIL_OP_KEEP,
		D3D11_STENCIL_OP_KEEP,
	};

	const D3D11_RASTERIZER_DESC BLOCK_RASTERIZER =
	{
		D3D11_FILL_SOLID,//D3D11_FILL_WIREFRAME,//D3D11_FILL_MODE FillMode;
		D3D11_CULL_BACK,//D3D11_CULL_MODE CullMode;
		FALSE,//BOOL FrontCounterClockwise;
		0,//INT DepthBias;
		0.0,//FLOAT DepthBiasClamp;
		0.0,//FLOAT SlopeScaledDepthBias;
		TRUE,//BOOL DepthClipEnable;
		FALSE,//BOOL ScissorEnable;
		TRUE,//BOOL MultisampleEnable;
		FALSE//BOOL AntialiasedLineEnable;   
	};

	const D3D11_RASTERIZER_DESC SHADOW_RASTERIZER =
	{
		D3D11_FILL_SOLID,//D3D11_FILL_MODE FillMode;
		D3D11_CULL_NONE,//D3D11_CULL_MODE CullMode;
		FALSE,//BOOL FrontCounterClockwise;
		0,//INT DepthBias;
		0.0,//FLOAT DepthBiasClamp;
		1.0,//FLOAT SlopeScaledDepthBias;
		TRUE,//BOOL DepthClipEnable;
		FALSE,//BOOL ScissorEnable;
		TRUE,//BOOL MultisampleEnable;
		FALSE//BOOL AntialiasedLineEnable;   
	};

	const TechniqueSetting g_Techniques[] =
	{
		{
			"SkyShader", D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST,
			{ "Sky_vs", "Sky_hs", "Sky_ds", nullptr, "Sky_ps" },
			MINIMUM_LAYOUT, ARRAYSIZE(MINIMUM_LAYOUT), nullptr, nullptr, &DEFAULT_BLEND
		},
		{
			"CopyBackBufferShader", D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
			{ "ScreenTriangle_vs", nullptr, nullptr, nullptr, "CopyBackBuffer_ps" },
			nullptr, 0, &DEFAULT_RASTERIZER, nullptr, nullptr
		},
		{
			"ScreenQuadShader", D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
			{ "ScreenQuad_vs", nullptr, nullptr, nullptr, nullptr },
			SCREENQUAD_LAYOUT, ARRAYSIZE(SCREENQUAD_LAYOUT), nullptr, nullptr, nullptr
		},
		{
			"UIShader", D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
			{ "UI_vs", nullptr, nullptr, nullptr, "UI_ps" },
			UI_LAYOUT, ARRAYSIZE(UI_LAYOUT), nullptr, &UI_DEPTHSTENCIL, &UI_BLEND
		}
	};

	std::list<TechniqueSetting> Technique::g_SavedSettings(g_Techniques, g_Techniques + ARRAYSIZE(g_Techniques));

	void Technique::AddTechnique(const TechniqueSetting & setting)
	{
		auto it = std::find(g_SavedSettings.begin(), g_SavedSettings.end(), setting);
		if (it == g_SavedSettings.end())
			g_SavedSettings.push_back(setting);
	}

	Technique::Technique(const D3D11_INPUT_ELEMENT_DESC* pIED, UINT uiLDNE) :
		m_Topology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST),//D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST),
		m_pIED(pIED),
		m_uiLDNE(uiLDNE),
		m_pVS(nullptr),
		m_pGS(nullptr),
		m_pHS(nullptr),
		m_pDS(nullptr),
		m_pPS(nullptr),
		m_pRS(nullptr),
		m_pDSS(nullptr),
		m_pBS(nullptr),
		m_pVSL(nullptr)
	{}

	Technique::Technique(const TechniqueSetting& setting) :
		m_Topology(setting.Topology),
		m_pIED(setting.InputElement),
		m_uiLDNE(setting.ElementCount),
		m_pVS(nullptr),
		m_pGS(nullptr),
		m_pHS(nullptr),
		m_pDS(nullptr),
		m_pPS(nullptr),
		m_pRS(nullptr),
		m_pDSS(nullptr),
		m_pBS(nullptr),
		m_pVSL(nullptr)
	{}

	Technique::~Technique()
	{
		SAFE_RELEASE(m_pVS);
		SAFE_RELEASE(m_pGS);
		SAFE_RELEASE(m_pHS);
		SAFE_RELEASE(m_pDS);
		SAFE_RELEASE(m_pPS);
		SAFE_RELEASE(m_pRS);
		SAFE_RELEASE(m_pDSS);
		SAFE_RELEASE(m_pBS);
		SAFE_RELEASE(m_pVSL);
	}

	HRESULT	Technique::Create(ID3D11Device* pd3dDevice, LPCSTR filename, const void* data)
	{
		HRESULT hr = S_OK;
		CHAR buffer[_MAX_PATH];

		SAFE_RELEASE(m_pVS);
		SAFE_RELEASE(m_pGS);
		SAFE_RELEASE(m_pHS);
		SAFE_RELEASE(m_pDS);
		SAFE_RELEASE(m_pPS);
		SAFE_RELEASE(m_pRS);
		SAFE_RELEASE(m_pDSS);
		SAFE_RELEASE(m_pBS);
		SAFE_RELEASE(m_pVSL);

		if (data)
		{
			const TechniqueSetting* setting = (const TechniqueSetting*)data;
			if (setting->Shader[0])
			{
				sprintf_s(buffer, _MAX_PATH, "%s%s.cso", SHADERFOLDER, setting->Shader[0]);
				if (PathFileExistsA(buffer))
					V(LoadShader(buffer, pd3dDevice, &m_pVS, m_pIED, m_uiLDNE, &m_pVSL));
			}

			if (setting->Shader[1])
			{
				sprintf_s(buffer, _MAX_PATH, "%s%s.cso", SHADERFOLDER, setting->Shader[1]);
				if (PathFileExistsA(buffer))
					V(LoadShader(buffer, pd3dDevice, &m_pHS));
			}

			if (setting->Shader[2])
			{
				sprintf_s(buffer, _MAX_PATH, "%s%s.cso", SHADERFOLDER, setting->Shader[2]);
				if (PathFileExistsA(buffer))
					V(LoadShader(buffer, pd3dDevice, &m_pDS));
			}

			if (setting->Shader[3])
			{
				sprintf_s(buffer, _MAX_PATH, "%s%s.cso", SHADERFOLDER, setting->Shader[3]);
				if (PathFileExistsA(buffer))
					V(LoadShader(buffer, pd3dDevice, &m_pGS));
			}

			if (setting->Shader[4])
			{
				sprintf_s(buffer, _MAX_PATH, "%s%s.cso", SHADERFOLDER, setting->Shader[4]);
				if (PathFileExistsA(buffer))
					V(LoadShader(buffer, pd3dDevice, &m_pPS));
			}

			if (setting->RasterizerState)
			{
				pd3dDevice->CreateRasterizerState(setting->RasterizerState, &m_pRS);
			}

			if (setting->StencilState)
			{
				pd3dDevice->CreateDepthStencilState(setting->StencilState, &m_pDSS);
			}

			if (setting->BlendState)
			{
				pd3dDevice->CreateBlendState(setting->BlendState, &m_pBS);
			}
		}
		else
		{
			sprintf_s(buffer, _MAX_PATH, "%s%s_vs.cso", SHADERFOLDER, filename);
			if (PathFileExistsA(buffer))
				V(LoadShader(buffer, pd3dDevice, &m_pVS, m_pIED, m_uiLDNE, &m_pVSL));

			sprintf_s(buffer, _MAX_PATH, "%s%s_hs.cso", SHADERFOLDER, filename);
			if (PathFileExistsA(buffer))
				V(LoadShader(buffer, pd3dDevice, &m_pHS));

			sprintf_s(buffer, _MAX_PATH, "%s%s_ds.cso", SHADERFOLDER, filename);
			if (PathFileExistsA(buffer))
				V(LoadShader(buffer, pd3dDevice, &m_pDS));

			sprintf_s(buffer, _MAX_PATH, "%s%s_gs.cso", SHADERFOLDER, filename);
			if (PathFileExistsA(buffer))
				V(LoadShader(buffer, pd3dDevice, &m_pGS));

			sprintf_s(buffer, _MAX_PATH, "%s%s_ps.cso", SHADERFOLDER, filename);
			if (PathFileExistsA(buffer))
				V(LoadShader(buffer, pd3dDevice, &m_pPS));
		}

		return hr;
	}

	HRESULT Technique::CreateDefaultBlendState(ID3D11Device * pd3dDevice)
	{
		HRESULT hr;

		D3D11_BLEND_DESC DefaultBlendStateDesc;
		ZeroMemory(&DefaultBlendStateDesc, sizeof(DefaultBlendStateDesc));
		DefaultBlendStateDesc.IndependentBlendEnable = FALSE;
		for (int i = 0; i < _countof(DefaultBlendStateDesc.RenderTarget); i++)
			DefaultBlendStateDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		V(pd3dDevice->CreateBlendState(&DefaultBlendStateDesc, &m_pBS));

		return hr;
	}

	HRESULT Technique::CreateDefaultDepthState(ID3D11Device * pd3dDevice, BOOL bEnableDepth, D3D11_DEPTH_WRITE_MASK WriteMask)
	{
		HRESULT hr;

		D3D11_DEPTH_STENCIL_DESC DSDesc;
		ZeroMemory(&DSDesc, sizeof(DSDesc));
		DSDesc.DepthEnable = bEnableDepth;
		DSDesc.DepthWriteMask = WriteMask;
		DSDesc.DepthFunc = D3D11_COMPARISON_GREATER;
		V(pd3dDevice->CreateDepthStencilState(&DSDesc, &m_pDSS));

		return hr;
	}

	HRESULT Technique::CreateDefaultRasterizerState(ID3D11Device * pd3dDevice, D3D11_FILL_MODE FillMode, D3D11_CULL_MODE CullMode, BOOL bIsFrontCCW)
	{
		HRESULT hr;

		D3D11_RASTERIZER_DESC RSDesc;
		ZeroMemory(&RSDesc, sizeof(RSDesc));
		RSDesc.FillMode = FillMode;
		RSDesc.CullMode = CullMode;
		RSDesc.FrontCounterClockwise = bIsFrontCCW;
		V(pd3dDevice->CreateRasterizerState(&RSDesc, &m_pRS));

		return hr;
	}

	HRESULT Technique::LoadShader(LPCSTR szFileName, ID3D11Device * pd3dDevice, ID3D11VertexShader ** pShader, const D3D11_INPUT_ELEMENT_DESC *layoutDesc, UINT layoutDescNumElements, ID3D11InputLayout** layout)
	{
		HRESULT hr = S_OK;
		FILE * file = nullptr;
		fopen_s(&file, szFileName, "rb");
		if (file == nullptr)
			return D3D11_ERROR_FILE_NOT_FOUND;
		fseek(file, 0, SEEK_END);
		size_t size = ftell(file);

		char* buffer = new char[size];
		fseek(file, 0, SEEK_SET);
		fread(&buffer[0], size, 1, file);

		hr = pd3dDevice->CreateVertexShader(buffer, size, nullptr, pShader);
		if (layout && layoutDesc)
			hr = pd3dDevice->CreateInputLayout(layoutDesc, layoutDescNumElements, buffer, size, layout);
		delete[] buffer;
		fclose(file);
		return hr;
	}

	HRESULT Technique::LoadShader(LPCSTR szFileName, ID3D11Device * pd3dDevice, ID3D11PixelShader ** pShader)
	{
		HRESULT hr = S_OK;
		FILE * file = nullptr;
		fopen_s(&file, szFileName, "rb");
		if (file == nullptr)
			return D3D11_ERROR_FILE_NOT_FOUND;
		fseek(file, 0, SEEK_END);
		size_t size = ftell(file);

		char* buffer = new char[size];
		fseek(file, 0, SEEK_SET);
		fread(&buffer[0], size, 1, file);

		hr = pd3dDevice->CreatePixelShader(buffer, size, nullptr, pShader);

		delete[] buffer;
		fclose(file);
		return hr;
	}

	HRESULT Technique::LoadShader(LPCSTR szFileName, ID3D11Device * pd3dDevice, ID3D11GeometryShader ** pShader)
	{
		HRESULT hr = S_OK;
		FILE * file = nullptr;
		fopen_s(&file, szFileName, "rb");
		if (file == nullptr)
			return D3D11_ERROR_FILE_NOT_FOUND;
		fseek(file, 0, SEEK_END);
		size_t size = ftell(file);

		char* buffer = new char[size];
		fseek(file, 0, SEEK_SET);
		fread(&buffer[0], size, 1, file);

		hr = pd3dDevice->CreateGeometryShader(buffer, size, nullptr, pShader);

		delete[] buffer;
		fclose(file);
		return hr;
	}

	HRESULT Technique::LoadShader(LPCSTR szFileName, ID3D11Device * pd3dDevice, ID3D11HullShader ** pShader)
	{
		HRESULT hr = S_OK;
		FILE * file = nullptr;
		fopen_s(&file, szFileName, "rb");
		if (file == nullptr)
			return D3D11_ERROR_FILE_NOT_FOUND;
		fseek(file, 0, SEEK_END);
		size_t size = ftell(file);

		char* buffer = new char[size];
		fseek(file, 0, SEEK_SET);
		fread(&buffer[0], size, 1, file);

		hr = pd3dDevice->CreateHullShader(buffer, size, nullptr, pShader);

		delete[] buffer;
		fclose(file);
		return hr;
	}

	HRESULT Technique::LoadShader(LPCSTR szFileName, ID3D11Device * pd3dDevice, ID3D11DomainShader ** pShader)
	{
		HRESULT hr = S_OK;
		FILE * file = nullptr;
		fopen_s(&file, szFileName, "rb");
		if (file == nullptr)
			return D3D11_ERROR_FILE_NOT_FOUND;
		fseek(file, 0, SEEK_END);
		size_t size = ftell(file);

		char* buffer = new char[size];
		fseek(file, 0, SEEK_SET);
		fread(&buffer[0], size, 1, file);

		hr = pd3dDevice->CreateDomainShader(buffer, size, nullptr, pShader);

		delete[] buffer;
		fclose(file);
		return hr;
	}

	HRESULT Technique::LoadShader(LPCSTR szFileName, ID3D11Device * pd3dDevice, ID3D11ComputeShader ** pShader)
	{
		HRESULT hr = S_OK;
		FILE * file = nullptr;
		fopen_s(&file, szFileName, "rb");
		if (file == nullptr)
			return D3D11_ERROR_FILE_NOT_FOUND;
		fseek(file, 0, SEEK_END);
		size_t size = ftell(file);

		char* buffer = new char[size];
		fseek(file, 0, SEEK_SET);
		fread(&buffer[0], size, 1, file);

		hr = pd3dDevice->CreateComputeShader(buffer, size, nullptr, pShader);

		delete[] buffer;
		fclose(file);
		return hr;
	}

	HRESULT Technique::CompileShaderFromFile(LPCTSTR fileName, LPCTSTR shaderMain, LPCTSTR shaderProfile, ID3DBlob **ppBlob)
	{
		char pShaderMainAsChar[128];
		char pShaderProfileAsChar[128];
		size_t count;
		wcstombs_s(&count, pShaderMainAsChar, shaderMain, 128);
		wcstombs_s(&count, pShaderProfileAsChar, shaderProfile, 128);

		// use DirectX to compile the shader file
		ID3DBlob *pErrorBlob = nullptr;
		D3D10_SHADER_MACRO pShaderMacros[1] = { NULL, NULL };

		HRESULT hr = D3DCompileFromFile(
			fileName,			  // fileName
			pShaderMacros,        // macro define's
			nullptr,                 // includes
			pShaderMainAsChar,    // main function name
			pShaderProfileAsChar, // shader profile/feature level
			0,                    // flags 1
			0,                    // flags 2
			ppBlob,               // blob data with compiled code
			&pErrorBlob           // any compile errors stored here
			);

		if (pErrorBlob)
		{
			pErrorBlob->Release();
		}

		return hr;
	}

	HRESULT Technique::LoadShader(
		LPCTSTR name,
		LPCTSTR shaderMain,
		LPCTSTR shaderProfile,
		ID3D11Device      *pD3dDevice,
		ID3D11GeometryShader **ppShader)
	{
		HRESULT hr = S_OK;

		ID3DBlob *pCompiledBlob = nullptr;
		V_RETURN(CompileShaderFromFile(name, shaderMain, shaderProfile, &pCompiledBlob));
		V_RETURN(pD3dDevice->CreateGeometryShader(pCompiledBlob->GetBufferPointer(), pCompiledBlob->GetBufferSize(), nullptr, ppShader));

		return hr;
	}

	HRESULT Technique::LoadShader(
		LPCTSTR name,
		LPCTSTR shaderMain,
		LPCTSTR shaderProfile,
		ID3D11Device      *pD3dDevice,
		ID3D11HullShader **ppShader)
	{
		HRESULT hr = S_OK;

		ID3DBlob *pCompiledBlob = nullptr;
		V_RETURN(CompileShaderFromFile(name, shaderMain, shaderProfile, &pCompiledBlob));
		V_RETURN(pD3dDevice->CreateHullShader(pCompiledBlob->GetBufferPointer(), pCompiledBlob->GetBufferSize(), nullptr, ppShader));

		return hr;
	}

	HRESULT Technique::LoadShader(
		LPCTSTR name,
		LPCTSTR shaderMain,
		LPCTSTR shaderProfile,
		ID3D11Device      *pD3dDevice,
		ID3D11DomainShader **ppShader)
	{
		HRESULT hr = S_OK;

		ID3DBlob *pCompiledBlob = nullptr;
		V_RETURN(CompileShaderFromFile(name, shaderMain, shaderProfile, &pCompiledBlob));
		V_RETURN(pD3dDevice->CreateDomainShader(pCompiledBlob->GetBufferPointer(), pCompiledBlob->GetBufferSize(), nullptr, ppShader));

		return hr;
	}

	HRESULT Technique::LoadShader(
		LPCTSTR name,
		LPCTSTR shaderMain,
		LPCTSTR shaderProfile,
		ID3D11Device      *pD3dDevice,
		ID3D11ComputeShader **ppShader)
	{
		HRESULT hr = S_OK;

		ID3DBlob *pCompiledBlob = nullptr;
		V_RETURN(CompileShaderFromFile(name, shaderMain, shaderProfile, &pCompiledBlob));
		V_RETURN(pD3dDevice->CreateComputeShader(pCompiledBlob->GetBufferPointer(), pCompiledBlob->GetBufferSize(), nullptr, ppShader));

		return hr;
	}

	HRESULT Technique::LoadShader(
		LPCTSTR name,
		LPCTSTR shaderMain,
		LPCTSTR shaderProfile,
		ID3D11Device      *pD3dDevice,
		ID3D11PixelShader **ppShader)
	{
		HRESULT hr = S_OK;

		ID3DBlob *pCompiledBlob = nullptr;
		V_RETURN(CompileShaderFromFile(name, shaderMain, shaderProfile, &pCompiledBlob));
		V_RETURN(pD3dDevice->CreatePixelShader(pCompiledBlob->GetBufferPointer(), pCompiledBlob->GetBufferSize(), nullptr, ppShader));

		return hr;
	}

	HRESULT Technique::LoadShader(
		LPCTSTR name,
		LPCTSTR shaderMain,
		LPCTSTR shaderProfile,
		ID3D11Device * pD3dDevice,
		ID3D11VertexShader ** ppShader,
		const D3D11_INPUT_ELEMENT_DESC *layoutDesc,
		UINT layoutDescNumElements,
		ID3D11InputLayout** layout)
	{
		HRESULT hr = S_OK;

		ID3DBlob *pCompiledBlob = nullptr;
		V_RETURN(CompileShaderFromFile(name, shaderMain, shaderProfile, &pCompiledBlob));
		V_RETURN(pD3dDevice->CreateVertexShader(pCompiledBlob->GetBufferPointer(), pCompiledBlob->GetBufferSize(), nullptr, ppShader));
		if (layout) V_RETURN(pD3dDevice->CreateInputLayout(layoutDesc, layoutDescNumElements, pCompiledBlob->GetBufferPointer(), pCompiledBlob->GetBufferSize(), layout));

		return hr;
	}

	void Technique::Apply(ID3D11DeviceContext* pd3dImmediateContext)
	{
		if (m_pRS)
		{
			pd3dImmediateContext->RSSetState(m_pRS);
		}

		if (m_pDSS)
		{
			pd3dImmediateContext->OMSetDepthStencilState(m_pDSS, 0);
		}

		if (m_pBS)
		{
			FLOAT blendFactor[] = { 0.f, 0.f, 0.f, 0.f };
			pd3dImmediateContext->OMSetBlendState(m_pBS, blendFactor, 0xffffffff);
		}

		pd3dImmediateContext->IASetInputLayout(m_pVSL);
		pd3dImmediateContext->VSSetShader(m_pVS, nullptr, 0);
		pd3dImmediateContext->GSSetShader(m_pGS, nullptr, 0);
		pd3dImmediateContext->HSSetShader(m_pHS, nullptr, 0);
		pd3dImmediateContext->DSSetShader(m_pDS, nullptr, 0);
		pd3dImmediateContext->PSSetShader(m_pPS, nullptr, 0);
		pd3dImmediateContext->IASetPrimitiveTopology(m_Topology);
	}

	std::vector<Technique*> Technique::BatchCreateTechnique()
	{
		std::vector<Technique*> list;
		list.reserve(g_SavedSettings.size());
		for (auto && it : g_SavedSettings)
		{
			Technique* tech = new Technique(it);
			list.push_back(tech);
		}
		return list;
	}
}