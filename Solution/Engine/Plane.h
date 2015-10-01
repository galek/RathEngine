#pragma once
#include "Mesh.h"
#include "Texture.h"

namespace Rath
{
	class Plane
	{
	protected:
		MeshPtr				m_Mesh;
		TexturePtr			m_Texture;
	public:
		Plane();
		~Plane();

		void CreateDevice(ID3D11Device* device);
		void Render(ID3D11DeviceContext* context);
	};
}
