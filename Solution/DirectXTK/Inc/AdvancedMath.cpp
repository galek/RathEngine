#include "pch.h"
#include "AdvancedMath.h"

namespace DirectX
{
	//-----------------------------------------------------------------------------
	// Transform an axis aligned box by an angle preserving transform.
	//-----------------------------------------------------------------------------
	void XM_CALLCONV XMBOUNDINGBOX::Transform(XMBOUNDINGBOX& Out, FXMMATRIX M) const
	{
		// Compute and transform the corners and find new min/max bounds.
		XMVECTOR Corner = XMVectorMultiplyAdd(Extents, g_BoxOffset[0], Center);
		Corner = XMVector3Transform(Corner, M);

		XMVECTOR Min, Max;
		Min = Max = Corner;

		for (size_t i = 1; i < 8; ++i)
		{
			Corner = XMVectorMultiplyAdd(Extents, g_BoxOffset[i], Center);
			Corner = XMVector3Transform(Corner, M);

			Min = XMVectorMin(Min, Corner);
			Max = XMVectorMax(Max, Corner);
		}

		// Store center and extents.
		Out.Center = (Min + Max) * 0.5f;
		Out.Extents = (Max - Min) * 0.5f;
	}

	//-----------------------------------------------------------------------------
	// Get the corner points of the box
	//-----------------------------------------------------------------------------
	void XMBOUNDINGBOX::GetCorners(XMVECTOR* Corners) const
	{
		assert(Corners != nullptr);

		for (size_t i = 0; i < 8; ++i)
		{
			Corners[i] = XMVectorMultiplyAdd(Extents, g_BoxOffset[i], Center);
		}
	}

	//-----------------------------------------------------------------------------
	// Axis-aligned box vs. axis-aligned box test
	//-----------------------------------------------------------------------------
	bool XMBOUNDINGBOX::Intersects(const XMBOUNDINGBOX& box) const
	{
		XMVECTOR CenterB = box.Center;
		XMVECTOR ExtentsB = box.Extents;

		XMVECTOR MinA = Center - Extents;
		XMVECTOR MaxA = Center + Extents;

		XMVECTOR MinB = CenterB - ExtentsB;
		XMVECTOR MaxB = CenterB + ExtentsB;

		// for each i in (x, y, z) if a_min(i) > b_max(i) or b_min(i) > a_max(i) then return false
		XMVECTOR Disjoint = XMVectorOrInt(XMVectorGreater(MinA, MaxB), XMVectorGreater(MinB, MaxA));

		return !DirectX::Internal::XMVector3AnyTrue(Disjoint);
	}

	bool XMBOUNDINGBOX::Intersects(const BoundingBox& box) const
	{
		XMVECTOR CenterB = XMLoadFloat3(&box.Center);
		XMVECTOR ExtentsB = XMLoadFloat3(&box.Extents);

		XMVECTOR MinA = Center - Extents;
		XMVECTOR MaxA = Center + Extents;

		XMVECTOR MinB = CenterB - ExtentsB;
		XMVECTOR MaxB = CenterB + ExtentsB;

		// for each i in (x, y, z) if a_min(i) > b_max(i) or b_min(i) > a_max(i) then return false
		XMVECTOR Disjoint = XMVectorOrInt(XMVectorGreater(MinA, MaxB), XMVectorGreater(MinB, MaxA));

		return !DirectX::Internal::XMVector3AnyTrue(Disjoint);
	}

	ContainmentType XMBOUNDINGBOX::Contains(const XMBOUNDINGBOX& box) const
	{
		XMVECTOR MinA = Center - Extents;
		XMVECTOR MaxA = Center + Extents;

		XMVECTOR MinB = box.Center - box.Extents;
		XMVECTOR MaxB = box.Center + box.Extents;

		// for each i in (x, y, z) if a_min(i) > b_max(i) or b_min(i) > a_max(i) then return false
		XMVECTOR Disjoint = XMVectorOrInt(XMVectorGreater(MinA, MaxB), XMVectorGreater(MinB, MaxA));

		if (DirectX::Internal::XMVector3AnyTrue(Disjoint))
			return DISJOINT;

		// for each i in (x, y, z) if a_min(i) <= b_min(i) and b_max(i) <= a_max(i) then A contains B
		XMVECTOR Inside = XMVectorAndInt(XMVectorLessOrEqual(MinA, MinB), XMVectorLessOrEqual(MaxB, MaxA));

		return DirectX::Internal::XMVector3AllTrue(Inside) ? CONTAINS : INTERSECTS;
	}


	XMVECTOR XM_CALLCONV XMBOUNDINGBOX::GetMinCorner()
	{
		return Center - Extents;
	}

	XMVECTOR XM_CALLCONV XMBOUNDINGBOX::GetMaxCorner()
	{
		return Center + Extents;
	}


	//-----------------------------------------------------------------------------
	// Create axis-aligned box that contains two other bounding boxes
	//-----------------------------------------------------------------------------
	void XMBOUNDINGBOX::CreateMerged(XMBOUNDINGBOX& Out, const XMBOUNDINGBOX& b1, const XMBOUNDINGBOX& b2)
	{
		XMVECTOR Min = XMVectorSubtract(b1.Center, b1.Extents);
		Min = XMVectorMin(Min, XMVectorSubtract(b2.Center, b2.Extents));

		XMVECTOR Max = XMVectorAdd(b1.Center, b1.Extents);
		Max = XMVectorMax(Max, XMVectorAdd(b2.Center, b2.Extents));

		assert(XMVector3LessOrEqual(Min, Max));

		Out.Center = (Min + Max) * 0.5f;
		Out.Extents = (Max - Min) * 0.5f;
	}

	XMFRUSTUM::XMFRUSTUM(const XMMATRIX& matWorldViewProj)
	{
		XMMATRIX vec = XMMatrixTranspose(matWorldViewProj);

		/* Left Plane */
		mPlanes[0] = vec.r[3] + vec.r[0];

		/* Right Plane */
		mPlanes[1] = vec.r[3] - vec.r[0];

		/* Top Plane */
		mPlanes[2] = vec.r[3] - vec.r[1];

		/* Bottom Plane */
		mPlanes[3] = vec.r[3] + vec.r[1];

		/* Near Plane */
		mPlanes[4] = vec.r[2];

		/* Far Plane */
		mPlanes[5] = vec.r[3] - vec.r[2];

		/* Normalize Planes */
		for (size_t i = 0; i < 6; i++)
			mPlanes[i] = XMPlaneNormalize(mPlanes[i]);
	};


	bool XMSPHERE::Intersects(const XMBOUNDINGBOX& box) const
	{
		XMVECTOR MinB = box.Center - box.Extents;
		XMVECTOR MaxB = box.Center + box.Extents;
		XMVECTOR closestPoint = XMVectorMin(XMVectorMax(Center, MinB), MaxB);
		XMVECTOR distance = XMVector3LengthSq(closestPoint - Center);

		return XMVector3EqualInt(XMVectorLessOrEqual(distance, RadiusSq), XMVectorTrueInt());
	}

	ContainmentType XMSPHERE::Contains(const XMBOUNDINGBOX& box) const
	{
		if (!Intersects(box))
			return DISJOINT;

		XMVECTOR InsideAll = XMVectorTrueInt();
		XMVECTOR offset = box.Center - Center;

		for (size_t i = 0; i < BoundingBox::CORNER_COUNT; ++i)
		{
			XMVECTOR C = XMVectorMultiplyAdd(box.Extents, g_BoxOffset[i], offset);
			XMVECTOR d = XMVector3LengthSq(C);
			InsideAll = XMVectorAndInt(InsideAll, XMVectorLessOrEqual(d, RadiusSq));
		}

		return (XMVector3EqualInt(InsideAll, XMVectorTrueInt())) ? CONTAINS : INTERSECTS;
	}

	XMFRUSTUM::XMFRUSTUM() : XMFRUSTUM(XMMatrixIdentity())
	{
	}

	bool XMFRUSTUM::Intersects(const BoundingBox& box) const
	{
		// Load the box
		XMVECTOR vCenter = XMLoadFloat3(&box.Center);
		XMVECTOR vExtents = XMLoadFloat3(&box.Extents);
		XMVECTOR Corners[8];
		//static XMVECTOR g_BoxOffset[8] =
		//{
		//	{ -1.0f, -1.0f, 1.0f, 0.0f },
		//	{ 1.0f, -1.0f, 1.0f, 0.0f },
		//	{ 1.0f, 1.0f, 1.0f, 0.0f },
		//	{ -1.0f, 1.0f, 1.0f, 0.0f },
		//	{ -1.0f, -1.0f, -1.0f, 0.0f },
		//	{ 1.0f, -1.0f, -1.0f, 0.0f },
		//	{ 1.0f, 1.0f, -1.0f, 0.0f },
		//	{ -1.0f, 1.0f, -1.0f, 0.0f },
		//};

		for (size_t i = 0; i < 8; ++i)
		{
			Corners[i] = XMVectorMultiplyAdd(vExtents, g_BoxOffset[i], vCenter);
		}

		// for each plane…
		for (size_t i = 0; i < 6; ++i)
		{
			bool inside = false;
			for (size_t j = 0; j < 8; ++j)
			{
				if (XMVector4Greater(XMPlaneDotCoord(mPlanes[i], Corners[j]), g_XMZero))
				{
					inside = true;
					break;
				}
			}
			if (!inside)
			{
				return false;
			}
		}
		return true;
	};

	bool XMFRUSTUM::Intersects(const XMBOUNDINGBOX& box) const
	{
		// Load the box
		XMVECTOR Corners[8];

		for (size_t i = 0; i < 8; i++)
		{
			Corners[i] = XMVectorMultiplyAdd(box.Extents, g_BoxOffset[i], box.Center);
		}

		// for each plane…
		for (size_t i = 0; i < 6; i++)
		{
			bool inside = false;
			for (size_t j = 0; j < 8; j++)
			{
				if (XMVector4Greater(XMPlaneDotCoord(mPlanes[i], Corners[j]), g_XMZero))
				{
					inside = true;
					break;
				}
			}
			if (!inside)
			{
				return false;
			}
		}
		return true;
	};
}