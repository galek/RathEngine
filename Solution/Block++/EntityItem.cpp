#include "stdafx.h"
#include "EntityItem.h"

#include "PhysicsManager.h"

EntityItem::EntityItem() :
	DynamicNode(physx::PxTransform(), physx::PxBoxGeometry(0.15f, 0.15f, 0.15f), 0)
{
	setupFiltering(CollisionType::eEntityItem, CollisionType::ePlayer);

	m_pDynamic->userData = (void*)this;
}

EntityItem::EntityItem(_In_ DirectX::XMMATRIX world) :
	EntityItem()
{
	SetWorld(world);
}

uint32 EntityItem::Save(FILE * file)
{
	uint32 size = Entity::Save(file);

	return size;
}

uint32 EntityItem::Load(FILE * file)
{
	uint32 size = Entity::Load(file);

	if (m_pDynamic)
	{
		physx::PxTransform t;
		memcpy(&t, &mWorld, sizeof(physx::PxTransform));
		m_pDynamic->setGlobalPose(t);
	}

	return size;
}