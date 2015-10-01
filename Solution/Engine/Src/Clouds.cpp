#include "pch.h"
#include "Clouds.h"

#include "AssetLibrary.h"

namespace Rath
{
	const D3D11_INPUT_ELEMENT_DESC SKYPLANE_LAYOUT[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	struct SKYPLANE_MESH
	{
		DirectX::XMFLOAT3  Position;
		DirectX::XMFLOAT2  Tex;
	};

	const D3D11_BLEND_DESC CLOUD_BLEND =
	{
		FALSE, //BOOL AlphaToCoverageEnable;
		FALSE, //BOOL IndependentBlendEnable;
		{
			{
				TRUE, //BOOL BlendEnable;
				D3D11_BLEND_SRC_ALPHA, //D3D11_BLEND SrcBlend;
				D3D11_BLEND_INV_SRC_ALPHA, //D3D11_BLEND DestBlend;
				D3D11_BLEND_OP_ADD, //D3D11_BLEND_OP BlendOp;
				D3D11_BLEND_ZERO, //D3D11_BLEND SrcBlendAlpha;
				D3D11_BLEND_ZERO, //D3D11_BLEND DestBlendAlpha;
				D3D11_BLEND_OP_ADD, //D3D11_BLEND_OP BlendOpAlpha;
				D3D11_COLOR_WRITE_ENABLE_ALL, //UINT8 RenderTargetWriteMask;
			},
		}
	};

	const D3D11_DEPTH_STENCIL_DESC CLOUD_DEPTHSTENCIL =
	{
		TRUE,
		D3D11_DEPTH_WRITE_MASK_ZERO,
		D3D11_COMPARISON_LESS_EQUAL,
		FALSE,
		D3D11_DEFAULT_STENCIL_READ_MASK,
		D3D11_DEFAULT_STENCIL_WRITE_MASK,
		D3D11_STENCIL_OP_KEEP,
		D3D11_STENCIL_OP_KEEP,
	};

	const TechniqueSetting g_Technique[] =
	{
		{
			"CloudShader", D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
			{ "Clouds_vs", nullptr, nullptr, nullptr, "Clouds_ps" },
			SKYPLANE_LAYOUT, ARRAYSIZE(SKYPLANE_LAYOUT), nullptr, &CLOUD_DEPTHSTENCIL, &CLOUD_BLEND
		},
	};

	Clouds::Clouds()
	{
		Technique::AddTechnique(g_Technique[0]);

		m_CloudsTechnique = AssetLibrary::GetTechnique("CloudShader");

		m_CloudTexture = AssetLibrary::GetTexture("cloud001.dds");
		m_RandomTexture = AssetLibrary::GetTexture("perturb001.dds");
	}


	Clouds::~Clouds()
	{
	}

	void Clouds::InitializeSkyPlane(ID3D11Device* device, UINT skyPlaneResolution, float skyPlaneWidth, float skyPlaneTop, float skyPlaneBottom, UINT textureRepeat)
	{
		float quadSize, radius, constant, textureDelta;
		UINT i, j, index;
		float positionX, positionY, positionZ, tu, tv;

		// Create the array to hold the sky plane coordinates.
		SKYPLANE_MESH * skyPlane = new SKYPLANE_MESH[(skyPlaneResolution + 1) * (skyPlaneResolution + 1)];

		// Determine the size of each quad on the sky plane.
		quadSize = skyPlaneWidth / (float)skyPlaneResolution;

		// Calculate the radius of the sky plane based on the width.
		radius = skyPlaneWidth / 2.0f;

		// Calculate the height constant to increment by.
		constant = (skyPlaneTop - skyPlaneBottom) / (radius * radius);

		// Calculate the texture coordinate increment value.
		textureDelta = (float)textureRepeat / (float)skyPlaneResolution;

		// Loop through the sky plane and build the coordinates based on the increment values given.
		for (j = 0; j <= skyPlaneResolution; j++)
		{
			for (i = 0; i <= skyPlaneResolution; i++)
			{
				// Calculate the vertex coordinates.
				positionX = (-0.5f * skyPlaneWidth) + ((float)i * quadSize);
				positionZ = (-0.5f * skyPlaneWidth) + ((float)j * quadSize);
				positionY = skyPlaneTop - (constant * ((positionX * positionX) + (positionZ * positionZ)));

				// Calculate the texture coordinates.
				tu = (float)i * textureDelta;
				tv = (float)j * textureDelta;

				// Calculate the index into the sky plane array to add this coordinate.
				index = j * (skyPlaneResolution + 1) + i;

				// Add the coordinates to the sky plane array.
				skyPlane[index].Position.x = positionX;
				skyPlane[index].Position.y = positionY;
				skyPlane[index].Position.z = positionZ;
				skyPlane[index].Tex.x = tu;
				skyPlane[index].Tex.y = tv;
			}
		}

		// Calculate the number of vertices in the sky plane mesh.
		UINT vertexCount = (skyPlaneResolution + 1) * (skyPlaneResolution + 1) * 6;

		// Set the index count to the same as the vertex count.
		UINT indexCount = vertexCount;

		// Create the vertex array.
		SKYPLANE_MESH * vertices = new SKYPLANE_MESH[vertexCount];

		// Create the index array.
		UINT * indices = new UINT[indexCount];

		index = 0;
		// Load the vertex and index array with the sky plane array data.
		for (j = 0; j<skyPlaneResolution; j++)
		{
			for (i = 0; i<skyPlaneResolution; i++)
			{
				UINT index1 = j * (skyPlaneResolution + 1) + i;
				UINT index2 = j * (skyPlaneResolution + 1) + (i + 1);
				UINT index3 = (j + 1) * (skyPlaneResolution + 1) + i;
				UINT index4 = (j + 1) * (skyPlaneResolution + 1) + (i + 1);

				// Triangle 1 - Upper Left
				vertices[index].Position = skyPlane[index1].Position;
				vertices[index].Tex = skyPlane[index1].Tex;
				indices[index] = index;
				index++;

				// Triangle 1 - Upper Right
				vertices[index].Position = skyPlane[index2].Position;
				vertices[index].Tex = skyPlane[index2].Tex;
				indices[index] = index;
				index++;

				// Triangle 1 - Bottom Left
				vertices[index].Position = skyPlane[index3].Position;
				vertices[index].Tex = skyPlane[index3].Tex;
				indices[index] = index;
				index++;

				// Triangle 2 - Bottom Left
				vertices[index].Position = skyPlane[index3].Position;
				vertices[index].Tex = skyPlane[index3].Tex;
				indices[index] = index;
				index++;

				// Triangle 2 - Upper Right
				vertices[index].Position = skyPlane[index2].Position;
				vertices[index].Tex = skyPlane[index2].Tex;
				indices[index] = index;
				index++;

				// Triangle 2 - Bottom Right
				vertices[index].Position = skyPlane[index4].Position;
				vertices[index].Tex = skyPlane[index4].Tex;
				indices[index] = index;
				index++;
			}
		}

		delete[] skyPlane;

		m_Mesh = new Mesh(device, sizeof(SKYPLANE_MESH),
			sizeof(SKYPLANE_MESH) * vertexCount, vertices,
			sizeof(UINT) * indexCount, indices,
			0);

		delete[] vertices;
		delete[] indices;

		m_ConstantBuffer.Initialize(device);
	}

	void Clouds::CreateDevice(ID3D11Device* device)
	{
		UINT skyPlaneResolution, textureRepeat;
		float skyPlaneWidth, skyPlaneTop, skyPlaneBottom;

		// Set the sky plane parameters.
		skyPlaneResolution = 50;
		skyPlaneWidth = 10.0f;
		skyPlaneTop = 0.5f;
		skyPlaneBottom = -0.5f;
		textureRepeat = 2;

		// Set the sky plane shader related parameters.
		m_ConstantBuffer.Data.scale = 0.3f;
		m_ConstantBuffer.Data.brightness = 0.5f;

		// Initialize the translation to zero.
		m_ConstantBuffer.Data.translation = 0.0f;

		InitializeSkyPlane(device, skyPlaneResolution, skyPlaneWidth, skyPlaneTop, skyPlaneBottom, textureRepeat);
	}

	void Clouds::Update(float fElapsedTime)
	{
		m_ConstantBuffer.Data.translation += fElapsedTime / 1024.0f;
		if (m_ConstantBuffer.Data.translation > 1.0f)
		{
			m_ConstantBuffer.Data.translation -= 1.0f;
		}
	}

	void Clouds::Render(ID3D11DeviceContext* context)
	{
		m_ConstantBuffer.ApplyChanges(context);
		m_ConstantBuffer.SetPS(context, 3);

		m_CloudTexture->BindToPS(context, 0);
		m_RandomTexture->BindToPS(context, 1);

		m_CloudsTechnique->Apply(context);

		m_Mesh->Render(context);
	}
}