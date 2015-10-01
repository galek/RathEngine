#include "pch.h"
#include "DynamicNode.h"

#include "PhysicsManager.h"

namespace Rath
{
	DynamicNode::DynamicNode(const physx::PxTransform& transform, const physx::PxGeometry& geometry, uint32 material)
	{
		Node::SetWorld(XMLoadFloat4x4((XMFLOAT4X4*)&physx::PxMat44(transform)));

		m_pDynamic = PhysicsManager::instance().createDynamic(transform, geometry, material);
		if (m_pDynamic)
		{
			m_pDynamic->setAngularDamping(0.5f);
			PhysicsManager::instance().AddActor(m_pDynamic);
		}
	}


	DynamicNode::~DynamicNode()
	{
		if (m_pDynamic)
		{
			PhysicsManager::instance().RemoveActor(m_pDynamic);
			m_pDynamic->release();
		}
	}

	void DynamicNode::setupFiltering(uint32 filterGroup, uint32 filterMask)
	{
		physx::PxFilterData filterData;
		filterData.word0 = (physx::PxU32)filterGroup; // word0 = own ID
		filterData.word1 = (physx::PxU32)filterMask;	// word1 = ID mask to filter pairs that trigger a contact callback;
		const physx::PxU32 numShapes = m_pDynamic->getNbShapes();
		physx::PxShape** shapes = (physx::PxShape**)malloc(sizeof(physx::PxShape*)*numShapes);
		m_pDynamic->getShapes(shapes, numShapes);
		for (physx::PxU32 i = 0; i < numShapes; i++)
		{
			physx::PxShape* shape = shapes[i];
			shape->setSimulationFilterData(filterData);
		}
		free(shapes);
	}

	DirectX::XMVECTOR DynamicNode::GetVelocity() const
	{
		if (m_pDynamic)
		{
			physx::PxVec3 v = m_pDynamic->getLinearVelocity();
			return XMLoadFloat3((XMFLOAT3*)&v);
		}
		else
		{
			return g_XMZero;
		}
	}

	void XM_CALLCONV DynamicNode::SetVelocity(_In_ DirectX::XMVECTOR velocity)
	{
		if (m_pDynamic)
		{
			physx::PxVec3 v;
			XMStoreFloat3((XMFLOAT3*)&v, velocity);
			m_pDynamic->setLinearVelocity(v);
		}
	}

	DirectX::XMVECTOR DynamicNode::GetPosition() const
	{
		if (m_pDynamic)
		{
			physx::PxTransform t = m_pDynamic->getGlobalPose();
			return XMLoadFloat3((XMFLOAT3*)&t.p);
		}
		else
		{
			return Node::GetPosition();
		}
	}

	void XM_CALLCONV DynamicNode::SetPosition(_In_ DirectX::XMVECTOR position)
	{
		Node::SetPosition(position);
		if (m_pDynamic)
		{
			physx::PxTransform t = m_pDynamic->getGlobalPose();
			XMStoreFloat3((XMFLOAT3*)&t.p, position);
			m_pDynamic->setGlobalPose(t);
		}
	}

	DirectX::XMMATRIX DynamicNode::GetWorld() const
	{
		if (m_pDynamic)
		{
			return XMLoadFloat4x4((XMFLOAT4X4*)&physx::PxMat44(m_pDynamic->getGlobalPose()));
		}
		else
		{
			return XMMatrixIdentity();
		}
	}

	void XM_CALLCONV DynamicNode::SetWorld(_In_ XMMATRIX world)
	{
		Node::SetWorld(world);
		if (m_pDynamic)
		{
			physx::PxMat44 t;
			memcpy(&t, &mWorld, sizeof(physx::PxMat44));
			m_pDynamic->setGlobalPose(physx::PxTransform(t));
		}
	}
}
