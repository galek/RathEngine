#pragma once
#include "Block.h"
#include "Technique.h"
#include "InterfacePointers.h"

class SelectionBox
{
protected:
	Rath::TechniquePtr						mTechnique;
	Microsoft::WRL::ComPtr<ID3D11Buffer>	mpWireVertexBuffer;
public:
	SelectionBox();
	~SelectionBox();

	void CreateDevice(ID3D11Device* pd3dDevice);
	void Render(ID3D11DeviceContext* pd3dImmediateContext, const Vector3& Position, Block block);
};

