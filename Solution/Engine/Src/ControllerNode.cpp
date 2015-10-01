#include "pch.h"
#include "ControllerNode.h"

#include "PhysicsManager.h"

namespace Rath
{
	ControllerNode::ControllerNode() :
		mVelocity(),
		mAcceleration(),
		mCollided(false),
		mGrounded(false),
		m_pController(nullptr)
	{
		physx::PxPhysics* physX = PhysicsManager::instance();
		physx::PxCapsuleControllerDesc desc;
		desc.height = 0.7f;
		desc.radius = 0.2f;
		desc.maxJumpHeight = 2.0f;
		desc.position = physx::PxExtendedVec3(46.7789955f, 141.000000f, 55.7633896f);
		desc.material = PhysicsManager::instance().GetMaterial(0);
		m_pController = PhysicsManager::instance().createController(desc);
	}

	ControllerNode::~ControllerNode()
	{
		if (m_pController != nullptr)
			m_pController->release();
	}

	void ControllerNode::setupFiltering(uint32 filterGroup, uint32 filterMask)
	{
		physx::PxFilterData filterData;
		filterData.word0 = (physx::PxU32)filterGroup; // word0 = own ID
		filterData.word1 = (physx::PxU32)filterMask;	// word1 = ID mask to filter pairs that trigger a contact callback;
		const physx::PxU32 numShapes = m_pController->getActor()->getNbShapes();
		physx::PxShape** shapes = (physx::PxShape**)malloc(sizeof(physx::PxShape*)*numShapes);
		m_pController->getActor()->getShapes(shapes, numShapes);
		for (physx::PxU32 i = 0; i < numShapes; i++)
		{
			physx::PxShape* shape = shapes[i];
			shape->setSimulationFilterData(filterData);
		}
		free(shapes);
	}

	XMVECTOR ControllerNode::GetVelocity() const
	{
		return mVelocity;
	}

	XMVECTOR ControllerNode::GetAcceleration() const
	{
		return mAcceleration;
	}

	void XM_CALLCONV ControllerNode::SetVelocity(_In_ XMVECTOR velocity)
	{
		mVelocity = velocity;
	}

	void XM_CALLCONV ControllerNode::SetAcceleration(_In_ XMVECTOR acceleration)
	{
		mAcceleration = acceleration;
	}

	void XM_CALLCONV ControllerNode::Accelerate(_In_ XMVECTOR acceleration)
	{
		mAcceleration += acceleration;
	}

	void XM_CALLCONV ControllerNode::SetWorld(_In_ DirectX::XMMATRIX world)
	{
		Node::SetWorld(world);

		physx::PxVec3 pos;
		XMStoreFloat3((XMFLOAT3*)&pos, world.r[3]);
		physx::PxExtendedVec3 position = physx::PxExtendedVec3(pos.x, pos.y, pos.z);
		m_pController->setPosition(position);
	}

	void ControllerNode::FrameMove(float fElapsedTime)
	{
		mVelocity += mAcceleration * fElapsedTime * 15.0f;
		mVelocity *= (1.0f - fElapsedTime * 5.0f);
		mAcceleration *= (1.0f - fElapsedTime * 5.0f);

		if (m_pController != nullptr)
		{
			physx::PxVec3 disp(mVelocity.x * fElapsedTime, mVelocity.y * fElapsedTime, mVelocity.z * fElapsedTime);
			physx::PxControllerFilters filter;
			filter.mFilterFlags = physx::PxQueryFlag::eSTATIC;
			const physx::PxU32 flags = m_pController->move(disp, 1.192092896e-7f, fElapsedTime, filter);
			physx::PxExtendedVec3 position = m_pController->getPosition();
			XMVECTOR pos = XMVectorSet((float)position.x, (float)position.y, (float)position.z, 1.0f);
			SetPosition(pos);

			mGrounded = (flags & physx::PxControllerCollisionFlag::eCOLLISION_DOWN) == physx::PxControllerCollisionFlag::eCOLLISION_DOWN;
		}

		if (!mGrounded)
		{
			XMVECTOR Gravity = { 0, fElapsedTime * 50.0f, 0, 0 };
			mAcceleration -= Gravity;
		}
	}
}