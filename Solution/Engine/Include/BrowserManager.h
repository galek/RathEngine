#pragma once
#include "Texture.h"

namespace Rath
{
	class BrowserManager
	{
	protected:
		ID3D11Device*			m_Device;
		ID3D11DeviceContext*	m_DeferredUpdateContext;
		std::vector<Texture*>	m_Textures;
	public:
		BrowserManager(HINSTANCE hInstance);
		~BrowserManager();

		Texture* CreateBrowserTexture(uint32 width, uint32 height, const std::string& webpage);

		void CreateRecources(ID3D11Device* device);
		void Update(ID3D11DeviceContext* context);
	};
}