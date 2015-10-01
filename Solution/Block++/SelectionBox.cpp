#include "stdafx.h"
#include "SelectionBox.h"
#include "AssetLibrary.h"

const D3D11_DEPTH_STENCIL_DESC WIRE_DEPTHSTENCIL =
{
	TRUE,
	D3D11_DEPTH_WRITE_MASK_ZERO,
	D3D11_COMPARISON_ALWAYS,
	FALSE,
	D3D11_DEFAULT_STENCIL_READ_MASK,
	D3D11_DEFAULT_STENCIL_WRITE_MASK,
	D3D11_STENCIL_OP_KEEP,
	D3D11_STENCIL_OP_KEEP,
};

const D3D11_RASTERIZER_DESC WIRE_RASTERIZER =
{
	D3D11_FILL_SOLID,//D3D11_FILL_WIREFRAME,//D3D11_FILL_MODE FillMode;
	D3D11_CULL_NONE,//D3D11_CULL_MODE CullMode;
	FALSE,//BOOL FrontCounterClockwise;
	-4096,//INT DepthBias;
	0.0f,//FLOAT DepthBiasClamp;
	-2.0f,//FLOAT SlopeScaledDepthBias;
	TRUE,//BOOL DepthClipEnable;
	FALSE,//BOOL ScissorEnable;
	TRUE,//BOOL MultisampleEnable;
	TRUE//BOOL AntialiasedLineEnable;   
};

const D3D11_INPUT_ELEMENT_DESC WIRE_LAYOUT[] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

const Rath::TechniqueSetting WireTechnique =
{
	"WireShader", D3D11_PRIMITIVE_TOPOLOGY_LINELIST,
	{ "WireShader_vs", nullptr, nullptr, nullptr, "WireShader_ps" },
	WIRE_LAYOUT, ARRAYSIZE(WIRE_LAYOUT), &WIRE_RASTERIZER, &WIRE_DEPTHSTENCIL, nullptr
};

SelectionBox::SelectionBox()
{
	mTechnique = Rath::AssetLibrary::GetTechnique("WireShader", &WireTechnique);
}


SelectionBox::~SelectionBox()
{
}

void SelectionBox::CreateDevice(ID3D11Device* pd3dDevice)
{
	D3D11_BUFFER_DESC		bd;
	bd.Usage				= D3D11_USAGE_DYNAMIC;
	bd.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;
	bd.BindFlags			= D3D11_BIND_VERTEX_BUFFER;
	bd.ByteWidth			= (UINT)sizeof(XMFLOAT3) * 256;
	bd.MiscFlags			= 0;
	bd.StructureByteStride	= 0;
	pd3dDevice->CreateBuffer(&bd, nullptr, mpWireVertexBuffer.ReleaseAndGetAddressOf());
}

void SelectionBox::Render(ID3D11DeviceContext* pd3dImmediateContext, const Vector3& Position, Block block)
{
	if (block.mBlocktype != Blocktype::Air && block.mBlocktype != Blocktype::Void)
	{
		std::vector<Vector3> vb; vb.reserve(24);
		std::vector<BLOCK_MESH> mesh; mesh.reserve(6);

		block.GetMesh(Position, Vector3(0, 0, 0), 0xFF, mesh);
		for (auto & it : mesh)
		{
			Vector3 position[] =
			{
				it.Position - it.Tangent - it.Binormal,
				it.Position - it.Tangent + it.Binormal,
				it.Position + it.Tangent - it.Binormal,
				it.Position + it.Tangent + it.Binormal,
			};

			vb.emplace_back(position[0]);
			vb.emplace_back(position[1]);

			vb.emplace_back(position[1]);
			vb.emplace_back(position[3]);

			vb.emplace_back(position[3]);
			vb.emplace_back(position[2]);

			vb.emplace_back(position[2]);
			vb.emplace_back(position[0]);
		}

		UINT mVertexCount = (UINT)vb.size();

		if (mVertexCount > 0)
		{
			D3D11_MAPPED_SUBRESOURCE	vbMapped;
			if (pd3dImmediateContext->Map(mpWireVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &vbMapped) == S_OK)
			{
				CopyMemory(vbMapped.pData, &vb[0], sizeof(Vector3) * min(mVertexCount, 256));
				pd3dImmediateContext->Unmap(mpWireVertexBuffer.Get(), 0);
			}

			UINT mStride = sizeof(Vector3);
			UINT mOffset = 0;

			mTechnique->Apply(pd3dImmediateContext);

			pd3dImmediateContext->IASetVertexBuffers(0, 1, mpWireVertexBuffer.GetAddressOf(), &mStride, &mOffset);
			pd3dImmediateContext->Draw(mVertexCount, 0);
		}
	}
}