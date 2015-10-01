#pragma once
#include "Model.h"

#include "SimpleMath.h"
#include "AdvancedMath.h"

namespace Rath
{
	class Node : public Referencable
	{
	protected:
		DirectX::SimpleMath::Matrix	mWorld;
		Node*						pParent;
	public:
		Node();

		void virtual XM_CALLCONV SetPosition(_In_ DirectX::XMVECTOR position);
		void XM_CALLCONV SetRelativePosition(_In_ DirectX::XMVECTOR position);
		DirectX::XMVECTOR virtual GetPosition() const;
		DirectX::XMVECTOR GetDirection() const;
		DirectX::XMVECTOR GetRight() const;
		DirectX::XMMATRIX virtual GetWorld() const;
		void XM_CALLCONV Scale(_In_ DirectX::XMVECTOR scale);
		void virtual XM_CALLCONV SetWorld(_In_ DirectX::XMMATRIX world);
		void XM_CALLCONV SetRelativeWorld(_In_ DirectX::XMMATRIX world);
		void SetParent(_In_ Node* parent);
	};

	class ModelNode : public virtual Node
	{
	protected:
		Model*	mpModel;
	public:
		ModelNode(Model* pModel);
		~ModelNode();

		void Render(ID3D11DeviceContext* pd3dImmediateContext, const DirectX::XMFRUSTUM& frustum);
	};

	class AnimatedModelNode : public virtual ModelNode
	{
	protected:
		float	mfAnimatonTime;
	public:
		AnimatedModelNode(AnimatedModel* pModel);

		const DirectX::XMMATRIXLIST* GetTransforms() const;
		void FrameMove(float fElapsedTime);
	};
}