#pragma once
#include "Node.h"

#include "PxPhysicsAPI.h"

namespace Rath
{
	class DynamicNode : public virtual Node
	{
	protected:
		physx::PxRigidDynamic*	m_pDynamic;

		void setupFiltering(uint32 filterGroup, uint32 filterMask);
	public:
		DynamicNode(const physx::PxTransform& transform, const physx::PxGeometry& geometry, uint32 material = 0);
		~DynamicNode();

		DirectX::XMVECTOR GetVelocity() const;
		void XM_CALLCONV SetVelocity(_In_ DirectX::XMVECTOR velocity);

		DirectX::XMVECTOR GetPosition() const override;
		void XM_CALLCONV SetPosition(_In_ DirectX::XMVECTOR position) override;

		DirectX::XMMATRIX GetWorld() const override;
		void XM_CALLCONV SetWorld(_In_ XMMATRIX world) override;
	};
}
