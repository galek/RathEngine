#include "stdafx.h"
#include "Entity.h"
#include "Block.h"
//
//INT AABBSweep(const XMBOUNDINGBOX& staticbox, const XMBOUNDINGBOX& movingbox, const XMVECTOR& velocity, FLOAT& ftime)
//{
//	XMVECTOR CenterS = staticbox.Center;
//	XMVECTOR ExtentsS = staticbox.Extents;
//	XMVECTOR CenterM = movingbox.Center;
//	XMVECTOR ExtentsM = movingbox.Extents;
//
//	XMVECTOR MinS = CenterS - ExtentsS;
//	XMVECTOR MaxS = CenterS + ExtentsS;
//	XMVECTOR MinM = CenterM - ExtentsM;
//	XMVECTOR MaxM = CenterM + ExtentsM;
//
//	XMVECTOR VSelect = XMVectorLess(velocity, g_XMZero);
//	XMVECTOR ZSelect = XMVectorEqual(velocity, g_XMZero);
//
//	XMVECTOR InvEntry = XMVectorSelect(
//		MinS - MaxM,
//		MaxS - MinM,
//		VSelect);
//	XMVECTOR InvExit = XMVectorSelect(
//		MaxS - MinM,
//		MinS - MaxM,
//		VSelect);
//
//	XMVECTOR InvVelocity = XMVectorReciprocalEst(velocity);
//	XMVECTOR Entry = XMVectorMultiply(InvEntry, InvVelocity);
//	XMVECTOR Exit = XMVectorMultiply(InvExit, InvVelocity);
//
//	Entry = XMVectorSelect(Entry, g_XMNegativeOne, ZSelect);
//	Exit = XMVectorSelect(Exit, g_XMOne, ZSelect);
//
//	XMVECTOR EntrySelect = XMVectorAndInt(XMVectorLess(Entry, g_XMZero),
//										  XMVectorOrInt(XMVectorLessOrEqual(MaxS, MinM),
//														XMVectorLessOrEqual(MaxM, MinS)));
//
//	bool  invalid =  XMVector3Greater(Entry, XMVectorReplicate(ftime)) ||
//					 XMVector3Less(Entry, XMVectorReplicate(-ftime)) ||
//					 DirectX::Internal::XMVector3AnyTrue(EntrySelect);
//
//	XMVECTOR EntryTime = _mm_max_ps(Entry, _mm_permute_ps(Entry, 0x11));
//	EntryTime = _mm_max_ps(EntryTime, _mm_permute_ps(EntryTime, 0xCA));
//
//	XMVECTOR ExitTime = _mm_min_ps(Exit, _mm_permute_ps(Exit, 0x11));
//	ExitTime = _mm_min_ps(ExitTime, _mm_permute_ps(ExitTime, 0xCA));
//
//	INT collisionAxis = -1;
//	if (XMVector3Less(EntryTime, ExitTime) && !invalid)
//	{
//		ftime = _mm_cvtss_f32(EntryTime);
//
//		XMVECTOR ESelect = XMVectorEqual(Entry, EntryTime);
//		int iTest = _mm_movemask_ps(ESelect);
//
//		if ((iTest & 4) && !(iTest & 3))
//			collisionAxis = 2;
//		else if ((iTest & 2) && !(iTest & 5))
//			collisionAxis = 1;
//		else if ((iTest & 1) && !(iTest & 6))
//			collisionAxis = 0;
//	}
//	return collisionAxis;
//}
//
//bool Entity::wouldCollideWithCeiling(const XMBOUNDINGBOX& movingbox, const std::vector<BoundingNode>& staticboxes, FLOAT y_increase)
//{
//	XMBOUNDINGBOX bb = movingbox;
//	bb.Center += XMVectorSet(0, y_increase, 0, 0);
//	for (auto& it : staticboxes) if (bb.Intersects(it.boundingbox))
//		return true;
//	return false;
//}
//
//bool Entity::isStepUp(const XMBOUNDINGBOX& staticbox, const XMBOUNDINGBOX& movingbox, const std::vector<BoundingNode>& staticboxes, FLOAT stepheight)
//{
//	FLOAT MinEdgeM = XMVectorGetY(movingbox.Center - movingbox.Extents);
//	FLOAT MaxEdgeS = XMVectorGetY(staticbox.Center + staticbox.Extents);
//	bool step_up =
//		(MinEdgeM < MaxEdgeS) &&
//		(MinEdgeM + stepheight > MaxEdgeS) &&
//		(!wouldCollideWithCeiling(movingbox, staticboxes, MaxEdgeS - MinEdgeM));
//	return step_up;
//}
//
//World* Entity::g_pWorld = nullptr;
//
//void Entity::GetBoundingBox(XMBOUNDINGBOX * aabb) const
//{
//	*aabb = XMBOUNDINGBOX(XMVectorSet(0.0f, 0.9f, 0.0f, 0.0f), XMVectorSet(0.4f, 0.9f, 0.4f, 0.0f));
//}
//
//void Entity::FrameMove(FLOAT fElapsedTime)
//{
//	XMBOUNDINGBOX movingBox, newMovingBox, broadphaseBox;
//
//	mVelocity += mAcceleration * fElapsedTime * 15.0f;
//	mVelocity *= (1.0f - fElapsedTime * 5.0f);
//	mAcceleration *= (1.0f - fElapsedTime * 5.0f);
//
//	GetBoundingBox(&movingBox);
//	XMVECTOR Center = movingBox.Center;
//	XMVECTOR mPosition = GetPosition();
//	//movingBox.Center += mPosition;
//
//	newMovingBox = movingBox;
//	newMovingBox.Center += mVelocity * fElapsedTime;
//
//	XMBOUNDINGBOX::CreateMerged(broadphaseBox, movingBox, newMovingBox);
//	broadphaseBox.Extents += XMVectorReplicate(0.1f);
//
//	VectorInt3 Min = VectorInt3(mPosition + broadphaseBox.Center - broadphaseBox.Extents);
//	VectorInt3 Max = VectorInt3(mPosition + broadphaseBox.Center + broadphaseBox.Extents);
//	VectorInt3 Pos;
//
//	std::vector<BoundingNode> vCollisionMove; vCollisionMove.reserve(8);
//
//	for (Pos.x = Min.x; Pos.x <= Max.x; Pos.x++)
//		for (Pos.z = Min.z; Pos.z <= Max.z; Pos.z++)
//			for (Pos.y = Min.y; Pos.y <= Max.y; Pos.y++)
//			{
//				Block block = g_pWorld->GetBlock(Pos);
//				if (block.mBlocktype == Blocktype::Air || block.mPassable)
//					continue;
//
//				const std::vector<XMBOUNDINGBOX> * bb = block.GetBoundingShape();
//
//				XMVECTOR Position = Pos.Float() - mPosition;
//				if (bb != nullptr) for (auto it = bb->begin(); it != bb->end(); ++it)
//				{
//					XMBOUNDINGBOX box = (*it);
//					box.Center += Position;
//					if (broadphaseBox.Intersects(box))
//					{
//						vCollisionMove.push_back({ box, false });
//					}
//				}
//			}
//
//	bool grounded = false;
//	bool collided = false;
//
//	size_t maxcount = 6;
//
//	FLOAT eTime = fElapsedTime;
//	while (eTime > 1.192092896e-7f && maxcount--)
//	{
//		//movingBox.Center = Center;// +mPosition;
//
//		INT nearest_collided = -1;
//		FLOAT nearest_dtime = FLT_MAX;
//		INT nearest_boxindex = -1;
//		INT index = -1;
//		for (auto& it : vCollisionMove)
//		{
//			index++;
//			if (it.stepup)
//				continue;
//
//			FLOAT dtime_tmp = eTime;
//			INT collided = AABBSweep(it.boundingbox, movingBox, mVelocity, dtime_tmp);
//
//			if (collided == -1 || dtime_tmp > nearest_dtime)
//				continue;
//
//			nearest_dtime = dtime_tmp;
//			nearest_collided = collided;
//			nearest_boxindex = index;
//		}
//
//		if (nearest_collided == -1)
//		{
//			mPosition += mVelocity * eTime;
//			eTime = 0.0f;
//		}
//		else
//		{
//			// Check for stairs.
//			bool step_up = (nearest_collided != 1) && isStepUp(vCollisionMove[nearest_boxindex].boundingbox, movingBox, vCollisionMove, 0.6f);
//
//			const XMVECTOR Normal[3] =
//			{
//				{ 1.0f, 0.0f, 0.0f, 0.0f },
//				{ 0.0f, 1.0f, 0.0f, 0.0f },
//				{ 0.0f, 0.0f, 1.0f, 0.0f },
//			};
//
//			if (nearest_dtime < 0.0f)
//			{
//				if (!step_up)
//				{
//					XMVECTOR ErrorCorrect = Normal[nearest_collided] * XMLoadFloat3(&mVelocity);
//					mPosition = XMVectorMultiplyAdd(XMVectorReplicate(nearest_dtime), ErrorCorrect, mPosition);
//				}
//			}
//			else
//			{
//				mPosition += mVelocity * nearest_dtime;
//				eTime -= nearest_dtime;
//
//				if ((nearest_collided == 1) && (XMVectorGetY(mVelocity) <= 0.0f))
//				{
//					grounded = true;
//				}
//				if ((nearest_collided == 0) || (nearest_collided == 2))
//				{
//					collided = true;
//				}
//			}
//
//			if (!step_up)
//			{
//				XMVECTOR vDotn = XMVector3Dot(mVelocity, Normal[nearest_collided]);
//				mVelocity = XMVectorMultiplyAdd(Normal[nearest_collided], -vDotn, mVelocity);
//			}
//			else
//			{
//				vCollisionMove[nearest_boxindex].stepup = true;
//			}
//
//			//vCollisionMove.erase(vCollisionMove.begin() + nearest_boxindex);
//		}
//	}
//
//	mGrounded = grounded;
//	mCollided = collided;
//
//	if (grounded)
//	{
//		XMVECTOR Min = Center + mPosition - movingBox.Extents;
//		for (auto& it : vCollisionMove) if (it.stepup)
//		{
//			XMVECTOR Max = it.boundingbox.Center + it.boundingbox.Extents;
//			XMVECTOR Change = (Max - Min) * g_XMIdentityR1;
//			mPosition += Change;
//			Min += Change;
//		}
//	}
//	else
//	{
//		XMVECTOR Gravity = { 0, fElapsedTime * 40.0f, 0, 0 };
//		mAcceleration -= Gravity;
//	}
//
//	SetPosition(mPosition);
//}

uint32 Entity::Save(FILE * file)
{
	fwrite((const char*)&mWorld, sizeof(char), sizeof(mWorld), file);

	return sizeof(mWorld);
}

uint32 Entity::Load(FILE * file)
{
	fread((char*)&mWorld, sizeof(char), sizeof(mWorld), file);

	return sizeof(mWorld);
}