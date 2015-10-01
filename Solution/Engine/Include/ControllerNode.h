#pragma once
#include "Node.h"

#include "PxPhysicsAPI.h"

namespace Rath
{
	class ControllerNode : public virtual Node
	{
	protected:
		physx::PxController*		 m_pController;

		DirectX::SimpleMath::Vector3 mVelocity;
		DirectX::SimpleMath::Vector3 mAcceleration;

		bool	mCollided;
		bool	mGrounded;

		void setupFiltering(uint32 filterGroup, uint32 filterMask);
	public:
		ControllerNode();
		~ControllerNode();

		DirectX::XMVECTOR GetVelocity() const;
		DirectX::XMVECTOR GetAcceleration() const;

		void XM_CALLCONV SetVelocity(_In_ DirectX::XMVECTOR velocity);
		void XM_CALLCONV SetAcceleration(_In_ DirectX::XMVECTOR acceleration);
		void XM_CALLCONV Accelerate(_In_ DirectX::XMVECTOR acceleration);
		void XM_CALLCONV SetWorld(_In_ DirectX::XMMATRIX world) override;

		void virtual FrameMove(float fElapsedTime);
	};
};
