#include "pch.h"
#include "Node.h"

namespace Rath
{
	Node::Node() :
		pParent(nullptr)
	{
	}


	void XM_CALLCONV Node::SetPosition(_In_ XMVECTOR position)
	{
		if (pParent != nullptr)
		{
			XMMATRIX world = XMMatrixInverse(nullptr, pParent->GetWorld());
			mWorld.Translation(XMVector4Transform(position, world));
		}
		else
		{
			mWorld.Translation(position);
		}
	}

	void XM_CALLCONV Node::SetRelativePosition(_In_ XMVECTOR position)
	{
		mWorld.Translation(position);
	}

	XMVECTOR Node::GetPosition() const
	{
		if (pParent != nullptr)
		{
			XMMATRIX world = mWorld;
			world *= pParent->GetWorld();
			return world.r[3];
		}
		else
		{
			return mWorld.Translation();
		}
	}

	XMVECTOR Node::GetDirection() const
	{
		if (pParent != nullptr)
		{
			XMMATRIX world = mWorld;
			world *= pParent->GetWorld();
			return world.r[2];
		}
		else
		{
			return mWorld.Backward();
		}
	}

	XMVECTOR Node::GetRight() const
	{
		if (pParent != nullptr)
		{
			XMMATRIX world = mWorld;
			world *= pParent->GetWorld();
			return world.r[0];
		}
		else
		{
			return mWorld.Right();
		}
	}

	void XM_CALLCONV Node::Scale(_In_ XMVECTOR scale)
	{
		mWorld *= XMMatrixScalingFromVector(scale);
	}

	void XM_CALLCONV Node::SetWorld(_In_ XMMATRIX world)
	{
		if (pParent != nullptr)
		{
			XMMATRIX invWorld = XMMatrixInverse(nullptr, pParent->GetWorld());
			mWorld = world * invWorld;
		}
		else
		{
			mWorld = world;
		}
	}

	void XM_CALLCONV Node::SetRelativeWorld(_In_ XMMATRIX world)
	{
		mWorld = world;
	}

	XMMATRIX Node::GetWorld() const
	{
		XMMATRIX world = mWorld;
		if (pParent != nullptr)
			world *= pParent->GetWorld();
		return world;
	}

	void Node::SetParent(_In_ Node* parent)
	{
		pParent = parent;
	}

	ModelNode::ModelNode(Model* pModel) :
		mpModel(pModel)
	{

	}

	ModelNode::~ModelNode()
	{
		SAFE_RELEASE(mpModel);
	}

	void ModelNode::Render(ID3D11DeviceContext* pd3dImmediateContext, const DirectX::XMFRUSTUM& frustum)
	{
		//	if (frustum.Intersects(mpModel->GetBoundingBox()))
		mpModel->Render(pd3dImmediateContext);
	}

	AnimatedModelNode::AnimatedModelNode(AnimatedModel* pModel) :
		ModelNode(pModel)
	{

	}

	const DirectX::XMMATRIXLIST* AnimatedModelNode::GetTransforms() const
	{
		return static_cast<AnimatedModel*>(mpModel)->GetTransforms(mfAnimatonTime);
	};

	void AnimatedModelNode::FrameMove(float fElapsedTime)
	{
		mfAnimatonTime += fElapsedTime;
	}
}