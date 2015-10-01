#pragma once

#include "UIElement.h"
#include "Font.h"

namespace Rath
{
	class UIManager
	{
	private:
		Microsoft::WRL::ComPtr<ID3D11Device>	m_pd3dDevice;
		Microsoft::WRL::ComPtr<ID3D11Buffer>	m_pVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>	m_pIndexBuffer;
		UINT									m_uiVertexSize;
		UINT									m_uiIndexSize;

		std::vector<UIElement*>		m_Elements;
		FontElement*				m_DefaultFont;

		TechniquePtr				m_pUITechnique;

		void	CopyBuffer(UIElement* pElement, ID3D11DeviceContext* pd3dDeviceContext);
	public:
		UIManager();
		~UIManager();

		void AddElement(UIElement* element);
		void RemoveElement(UIElement* element);

		void PrintText(LPCWCHAR text, const XMFLOAT2& position, FLOAT size = 1.f, const FLOAT ColorRGBA[4] = DirectX::Colors::White, FLOAT depth = 0.f);
		void PrintText(LPCWCHAR text, const XMINT2& position, UINT size = 16, const FLOAT ColorRGBA[4] = DirectX::Colors::White, FLOAT depth = 0.f);

		void CreateDevice(ID3D11Device* device);
		void WindowSizeChanged(uint32 width, uint32 height);
		void Render(ID3D11DeviceContext* context);
	};


}