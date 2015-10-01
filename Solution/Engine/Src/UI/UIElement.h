#pragma once
#include "Texture.h"
#include "Technique.h"

namespace Rath
{
	class UIElement
	{
		friend class UIManager;
	protected:
		TexturePtr				m_pTexture;
		std::vector<UI_MESH>	m_pMesh;
		std::vector<UINT>		m_pIndex;
		UINT					m_uiIndexSize;
	public:
		UIElement() {};
		UIElement(TexturePtr texture) : m_pTexture(texture) {};

		void PushQuad(const XMFLOAT2& PositionTopLeft, const XMFLOAT2& PositionLowerRight, FLOAT Depth, const XMINT2& TextureTopLeft, const XMINT2& TextureLowerRight, const FLOAT* Color);
		void PushQuad(const XMFLOAT2& PositionTopLeft, const XMFLOAT2& PositionLowerRight, FLOAT Depth, const XMFLOAT2& TextureTopLeft, const XMFLOAT2& TextureLowerRight, const FLOAT* Color);
		void PushQuad(const XMFLOAT4& Position, FLOAT Depth, const XMFLOAT4& Texture, const FLOAT* Color);

		void Clear();
	};
}