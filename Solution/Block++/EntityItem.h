#pragma once
#include "Entity.h"
#include "DynamicNode.h"

#pragma warning( disable : 4250 )
class EntityItem : public Rath::DynamicNode, public Entity
{
protected:
	float	m_LifeTime;
public:
	EntityItem();
	EntityItem(_In_ DirectX::XMMATRIX world);

	UUID(1001)
};

