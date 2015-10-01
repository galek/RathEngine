#pragma once
#include "Loadable.h"
#include "SimpleMath.h"

namespace Rath
{
	struct TAnimatedMesh
	{
		DirectX::SimpleMath::Vector3	Position;
		DirectX::SimpleMath::Vector2	Tex;
		DirectX::SimpleMath::Vector3	Normal;
		unsigned char					BoneWeights[4];
		unsigned char					BoneIndices[4];
	};

	struct TMesh
	{
		DirectX::SimpleMath::Vector3  Position;
		DirectX::SimpleMath::Vector2  Tex;
		DirectX::SimpleMath::Vector3  Normal;
	};

	class __declspec(uuid("{6D57F893-5791-4143-B44A-CAC8398CFD31}")) Mesh : public Referencable
	{
	protected:
		ID3D11Buffer*	m_pVertexBuffer;
		ID3D11Buffer*	m_pIndexBuffer;
		UINT			m_stride;
		UINT			m_offset;
		UINT			m_indexSize;
		UINT			m_material;
		virtual ~Mesh();
	public:
		Mesh(ID3D11Device* device, INT Stride, UINT VertexSize, void* VertexBuffer, UINT IndexSize, UINT* IndexBuffer, UINT Material);
		void Render(ID3D11DeviceContext* context);
		UINT getMaterialIndex() { return m_material; };
	};

	class __declspec(uuid("{F6AF6398-B0B5-44EC-890B-15179B658B22}")) ID3D11Mesh : public Referencable
	{
	protected:
		ID3D11Buffer*	m_pVertexBuffer;
		ID3D11Buffer*	m_pIndexBuffer;
		UINT			m_stride;
		UINT			m_offset;
		UINT			m_indexSize;
		~ID3D11Mesh() override;

		void Create(ID3D11Device* device, UINT VertexSize, void* VertexBuffer, UINT IndexSize, UINT* IndexBuffer);
	public:
		template<typename TypeT>
		ID3D11Mesh(ID3D11Device* device, UINT VertexCount, TypeT* VertexBuffer, UINT IndexCount, UINT* IndexBuffer)
		{
			m_stride = sizeof(TypeT);
			m_indexSize = IndexCount;
			Create(device, VertexCount * sizeof(TypeT), (void*)VertexBuffer, IndexCount * sizeof(UINT), IndexBuffer);
		};

		template<typename TypeT>
		ID3D11Mesh(ID3D11Device* device, const std::vector<TypeT>& vertexbuffer, const std::vector<UINT>& indexbuffer)
		{
			m_stride = sizeof(TypeT);
			m_indexSize = (UINT)indexbuffer.size();
			Create(device, (UINT)vertexbuffer.size() * sizeof(TypeT), (void*)&vertexbuffer[0], m_indexSize * sizeof(UINT), &indexbuffer[0]);
		};

		template<typename TypeT>
		ID3D11Mesh(ID3D11Device* device, const std::vector<TypeT>& vertexbuffer)
		{
			m_stride = sizeof(TypeT);
			m_indexSize = (UINT)vertexbuffer.size();
			Create(device, m_indexSize * sizeof(TypeT), (void*)&vertexbuffer[0], 0, nullptr);
		};

		void Render(ID3D11DeviceContext* context);
	};

	class __declspec(uuid("{B185DD56-3B86-4EC5-858C-8361A8A14680}")) ID3D11InstancedMesh : public ID3D11Mesh
	{
	protected:
		ID3D11Buffer*	m_pInstanceBuffer;
		UINT			m_instanceStride;
		UINT			m_instanceOffset;
		UINT			m_instanceSize;
		UINT			m_instanceRealSize;
		~ID3D11InstancedMesh() override;

		void CreateInstancing(ID3D11Device* device, ID3D11DeviceContext* context, UINT InstanceSize, void* InstanceBuffer);
	public:
		template<typename TypeT>
		ID3D11InstancedMesh(ID3D11Device* device, UINT VertexCount, TypeT* VertexBuffer, UINT IndexCount, UINT* IndexBuffer) :
			ID3D11Mesh(device, VertexCount, VertexBuffer, IndexCount, IndexBuffer),
			m_pInstanceBuffer(nullptr),
			m_instanceStride(0),
			m_instanceOffset(0),
			m_instanceSize(0),
			m_instanceRealSize(0)
		{
		};

		template<typename TypeT>
		ID3D11InstancedMesh(ID3D11Device* device, const std::vector<TypeT>& vertexbuffer, const std::vector<UINT>& indexbuffer) :
			ID3D11Mesh(device, vertexbuffer, indexbuffer),
			m_pInstanceBuffer(nullptr),
			m_instanceStride(0),
			m_instanceOffset(0),
			m_instanceSize(0),
			m_instanceRealSize(0)
		{
		};

		template<typename TypeT>
		ID3D11InstancedMesh(ID3D11Device* device, const std::vector<TypeT>& vertexbuffer) :
			ID3D11Mesh(device, vertexbuffer),
			m_pInstanceBuffer(nullptr),
			m_instanceStride(0),
			m_instanceOffset(0),
			m_instanceSize(0),
			m_instanceRealSize(0)
		{
		};

		template<typename TypeT>
		void CreateInstancing(ID3D11Device* device, ID3D11DeviceContext* context, UINT instanceCount, TypeT* instancebuffer)
		{
			m_instanceStride = sizeof(TypeT);
			m_instanceSize = instanceCount;
			CreateInstancing(device, context, instanceCount * sizeof(TypeT), (void*)instancebuffer);
		};

		template<typename TypeT>
		void CreateInstancing(ID3D11Device* device, ID3D11DeviceContext* context, const std::vector<TypeT>& instancebuffer)
		{
			m_instanceStride = sizeof(TypeT);
			m_instanceSize = (UINT)instancebuffer.size();
			CreateInstancing(device, context, m_instanceSize * sizeof(TypeT), (void*)&instancebuffer[0]);
		};

		void Render(ID3D11DeviceContext* context);
	};

	_COM_SMARTPTR_TYPEDEF(Mesh, __uuidof(Mesh));
	_COM_SMARTPTR_TYPEDEF(ID3D11Mesh, __uuidof(ID3D11Mesh));
	_COM_SMARTPTR_TYPEDEF(ID3D11InstancedMesh, __uuidof(ID3D11InstancedMesh));
}