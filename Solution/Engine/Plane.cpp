#include "pch.h"
#include "Plane.h"

namespace Rath
{
	Plane::Plane()
	{
	}


	Plane::~Plane()
	{
	}

	void Plane::CreateDevice(ID3D11Device* device)
	{
		UINT vertexCount = 4;
		UINT indexCount = 6;

		TMesh* vertices = (TMesh*)malloc(sizeof(TMesh) * vertexCount);
		UINT* indices = (UINT*)malloc(sizeof(UINT) * indexCount);



		m_Mesh = new Mesh(device, sizeof(TMesh),
			sizeof(TMesh) * vertexCount, vertices,
			sizeof(UINT) * indexCount, indices,
			0);
	}

	void Plane::Render(ID3D11DeviceContext* context)
	{

	}
}