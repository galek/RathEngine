#pragma once
#include "Loadable.h"

#include <list>
// --------------------------------------------------------------------------------------
// Layouts
//--------------------------------------------------------------------------------------
namespace Rath
{
	const D3D11_INPUT_ELEMENT_DESC MINIMUM_LAYOUT[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	struct MINIMUM_MESH
	{
		DirectX::XMFLOAT3  Position;
	};

	const D3D11_INPUT_ELEMENT_DESC BASIC_LAYOUT[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	struct BASIC_MESH
	{
		DirectX::XMFLOAT3  Position;
		DirectX::XMFLOAT2  Tex;
		DirectX::XMFLOAT3  Normal;
	};

	const D3D11_INPUT_ELEMENT_DESC UI_LAYOUT[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	struct UI_MESH
	{
		DirectX::XMFLOAT3  Position;
		DirectX::XMFLOAT2  Tex;
		DirectX::XMFLOAT4  Color;
	};

	const D3D11_INPUT_ELEMENT_DESC SCREENQUAD_LAYOUT[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	struct TechniqueSetting
	{
		const char*						Name;
		D3D_PRIMITIVE_TOPOLOGY			Topology;
		const char*						Shader[5];
		const D3D11_INPUT_ELEMENT_DESC*	InputElement;
		UINT							ElementCount;
		const D3D11_RASTERIZER_DESC*	RasterizerState;
		const D3D11_DEPTH_STENCIL_DESC* StencilState;
		const D3D11_BLEND_DESC*			BlendState;

		bool operator==(const TechniqueSetting &other) const;
		bool operator==(const char* name) const;
		bool operator<(const TechniqueSetting &other) const;
		bool operator<(const char* name) const;
	};

	class __declspec(uuid("{ECA7AAE0-DD82-4A41-BF0A-34FE053C0CDD}")) Technique : public Loadable
	{
		friend class AssetLibrary;
	protected:
		ID3D11VertexShader*					m_pVS;
		ID3D11GeometryShader*				m_pGS;
		ID3D11HullShader*					m_pHS;
		ID3D11DomainShader*					m_pDS;
		ID3D11PixelShader*					m_pPS;
		ID3D11RasterizerState*				m_pRS;
		ID3D11DepthStencilState*			m_pDSS;
		ID3D11BlendState*					m_pBS;
		ID3D11InputLayout*					m_pVSL;
		UINT								m_uiSampleRef;
		UINT								m_uiLDNE;
		const D3D11_INPUT_ELEMENT_DESC*		m_pIED;
		D3D_PRIMITIVE_TOPOLOGY				m_Topology;

		HRESULT						Create(ID3D11Device* pd3dDevice, LPCSTR filename, const void* data);

		static std::list<TechniqueSetting>	g_SavedSettings;
	public:
		Technique(const D3D11_INPUT_ELEMENT_DESC* pIED, UINT uiLDNE);
		Technique(const TechniqueSetting& setting);
		~Technique();

		static std::vector<Technique*> BatchCreateTechnique();
		static void AddTechnique(const TechniqueSetting & setting);

		static HRESULT LoadShader(
			_In_ LPCSTR szFileName,
			_In_ ID3D11Device * pd3dDevice,
			_Out_ ID3D11PixelShader ** pShader
			);

		static HRESULT Technique::LoadShader(
			_In_ LPCTSTR name,
			_In_ LPCTSTR shaderMain,
			_In_ LPCTSTR shaderProfile,
			_In_ ID3D11Device * pD3dDevice,
			_Out_ ID3D11PixelShader ** ppShader
			);

		static HRESULT LoadShader(
			_In_ LPCSTR szFileName,
			_In_ ID3D11Device * pd3dDevice,
			_Out_ ID3D11GeometryShader ** pShader
			);

		static HRESULT Technique::LoadShader(
			_In_ LPCTSTR name,
			_In_ LPCTSTR shaderMain,
			_In_ LPCTSTR shaderProfile,
			_In_ ID3D11Device * pD3dDevice,
			_Out_ ID3D11GeometryShader ** ppShader
			);

		static HRESULT LoadShader(
			_In_ LPCSTR szFileName,
			_In_ ID3D11Device * pd3dDevice,
			_Out_ ID3D11ComputeShader ** pShader
			);

		static HRESULT Technique::LoadShader(
			_In_ LPCTSTR name,
			_In_ LPCTSTR shaderMain,
			_In_ LPCTSTR shaderProfile,
			_In_ ID3D11Device * pD3dDevice,
			_Out_ ID3D11ComputeShader ** ppShader
			);

		static HRESULT LoadShader(
			_In_ LPCSTR szFileName,
			_In_ ID3D11Device * pd3dDevice,
			_Out_ ID3D11HullShader ** pShader
			);

		static HRESULT Technique::LoadShader(
			_In_ LPCTSTR name,
			_In_ LPCTSTR shaderMain,
			_In_ LPCTSTR shaderProfile,
			_In_ ID3D11Device * pD3dDevice,
			_Out_ ID3D11HullShader ** ppShader
			);

		static HRESULT LoadShader(
			_In_ LPCSTR szFileName,
			_In_ ID3D11Device * pd3dDevice,
			_Out_ ID3D11DomainShader ** pShader
			);

		static HRESULT Technique::LoadShader(
			_In_ LPCTSTR name,
			_In_ LPCTSTR shaderMain,
			_In_ LPCTSTR shaderProfile,
			_In_ ID3D11Device * pD3dDevice,
			_Out_ ID3D11DomainShader ** ppShader
			);

		static HRESULT LoadShader(
			_In_ LPCSTR szFileName,
			_In_ ID3D11Device * pd3dDevice,
			_Out_ ID3D11VertexShader ** pShader,
			_In_reads_opt_(layoutDescNumElements) const D3D11_INPUT_ELEMENT_DESC *layoutDesc = nullptr,
			_In_range_(0, D3D11_IA_VERTEX_INPUT_STRUCTURE_ELEMENT_COUNT)  UINT layoutDescNumElements = 0,
			_Out_opt_ ID3D11InputLayout** layout = nullptr
			);

		static HRESULT LoadShader(
			_In_ LPCTSTR name,
			_In_ LPCTSTR shaderMain,
			_In_ LPCTSTR shaderProfile,
			_In_ ID3D11Device * pD3dDevice,
			_Out_ ID3D11VertexShader ** pShader,
			_In_reads_opt_(layoutDescNumElements) const D3D11_INPUT_ELEMENT_DESC *layoutDesc = nullptr,
			_In_range_(0, D3D11_IA_VERTEX_INPUT_STRUCTURE_ELEMENT_COUNT)  UINT layoutDescNumElements = 0,
			_Out_opt_ ID3D11InputLayout** layout = nullptr
			);

		static HRESULT CompileShaderFromFile(
			_In_ LPCTSTR fileName,
			_In_ LPCTSTR shaderMain,
			_In_ LPCTSTR shaderProfile,
			_Out_ ID3DBlob ** ppBlob);

		HRESULT CreateDefaultBlendState(ID3D11Device * pd3dDevice);
		HRESULT CreateDefaultDepthState(ID3D11Device * pd3dDevice, BOOL bEnableDepth = TRUE, D3D11_DEPTH_WRITE_MASK WriteMask = D3D11_DEPTH_WRITE_MASK_ALL);
		HRESULT CreateDefaultRasterizerState(ID3D11Device * pd3dDevice, D3D11_FILL_MODE FillMode = D3D11_FILL_SOLID, D3D11_CULL_MODE CullMode = D3D11_CULL_BACK, BOOL bIsFrontCCW = FALSE);
	public:
		void Apply(ID3D11DeviceContext* pd3dImmediateContext);
	};

	_COM_SMARTPTR_TYPEDEF(Technique, __uuidof(Technique));
}