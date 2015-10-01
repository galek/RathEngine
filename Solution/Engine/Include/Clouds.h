#pragma once
#include "Mesh.h"
#include "Technique.h"
#include "Texture.h"
#include "GraphicTypes.h"

namespace Rath
{
	class Clouds
	{
	protected:
		struct CLOUDS_CB_STRUCT
		{
			float	translation;
			float	scale;
			float	brightness;
		};
		ConstantBuffer<CLOUDS_CB_STRUCT>  m_ConstantBuffer;

		MeshPtr				m_Mesh;
		TechniquePtr		m_CloudsTechnique;
		TexturePtr			m_CloudTexture;
		TexturePtr			m_RandomTexture;

		void InitializeSkyPlane(ID3D11Device*, UINT, float, float, float, UINT);
	public:
		Clouds();
		~Clouds();

		void CreateDevice(ID3D11Device* device);
		void Update(float fElapsedTime);
		void Render(ID3D11DeviceContext* context);
	};
}
