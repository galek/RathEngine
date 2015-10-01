#include "pch.h"
#include "UIElement.h"
#include "AssetLibrary.h"

namespace Rath
{
	void UIElement::PushQuad(const XMFLOAT2& PositionTopLeft, const XMFLOAT2& PositionLowerRight, FLOAT Depth,
		const XMINT2& TextureTopLeft, const XMINT2& TextureLowerRight,
		const FLOAT* Color)
	{
		UINT width = 512;
		UINT height = 512;
		//m_pTexture->GetTextureSize(&width, &height);
		XMFLOAT2 min = XMFLOAT2((float)TextureTopLeft.x / width, (float)TextureTopLeft.y / height);
		XMFLOAT2 max = XMFLOAT2((float)TextureLowerRight.x / width, (float)TextureLowerRight.y / height);

		PushQuad(PositionTopLeft, PositionLowerRight, Depth, min, max, Color);
	}

	void UIElement::PushQuad(const XMFLOAT2& PositionTopLeft, const XMFLOAT2& PositionLowerRight, FLOAT Depth,
		const XMFLOAT2& TextureTopLeft, const XMFLOAT2& TextureLowerRight,
		const FLOAT* Color)
	{
		UINT size = UINT(m_pMesh.size()); 
		XMFLOAT4 color(Color);
		m_pMesh.push_back({ XMFLOAT3(PositionTopLeft.x, 1.f - PositionLowerRight.y, Depth), XMFLOAT2(TextureTopLeft.x, TextureLowerRight.y), color });
		m_pMesh.push_back({ XMFLOAT3(PositionLowerRight.x, 1.f - PositionLowerRight.y, Depth), XMFLOAT2(TextureLowerRight.x, TextureLowerRight.y), color });
		m_pMesh.push_back({ XMFLOAT3(PositionLowerRight.x, 1.f - PositionTopLeft.y, Depth), XMFLOAT2(TextureLowerRight.x, TextureTopLeft.y), color });
		m_pMesh.push_back({ XMFLOAT3(PositionTopLeft.x, 1.f - PositionTopLeft.y, Depth), XMFLOAT2(TextureTopLeft.x, TextureTopLeft.y), color });

		m_pIndex.push_back(size);
		m_pIndex.push_back(size + 3);
		m_pIndex.push_back(size + 1);

		m_pIndex.push_back(size + 1);
		m_pIndex.push_back(size + 3);
		m_pIndex.push_back(size + 2);

		m_uiIndexSize = (UINT)m_pIndex.size();
	}

	void UIElement::PushQuad(const XMFLOAT4& Position, FLOAT Depth, const XMFLOAT4& Texture, const FLOAT* Color)
	{
		UINT size = UINT(m_pMesh.size());
		XMFLOAT4 color(Color);
		m_pMesh.push_back({ XMFLOAT3(Position.x, 1.f - Position.w, Depth), XMFLOAT2(Texture.x, Texture.w), color });
		m_pMesh.push_back({ XMFLOAT3(Position.z, 1.f - Position.w, Depth), XMFLOAT2(Texture.z, Texture.w), color });
		m_pMesh.push_back({ XMFLOAT3(Position.z, 1.f - Position.y, Depth), XMFLOAT2(Texture.z, Texture.y), color });
		m_pMesh.push_back({ XMFLOAT3(Position.x, 1.f - Position.y, Depth), XMFLOAT2(Texture.x, Texture.y), color });

		m_pIndex.push_back(size);
		m_pIndex.push_back(size + 3);
		m_pIndex.push_back(size + 1);

		m_pIndex.push_back(size + 1);
		m_pIndex.push_back(size + 3);
		m_pIndex.push_back(size + 2);

		m_uiIndexSize = (UINT)m_pIndex.size();
	}

	void UIElement::Clear()
	{
		m_uiIndexSize = 0;
		m_pMesh.clear();
		m_pIndex.clear();
	}
}