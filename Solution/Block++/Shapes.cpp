#include "stdafx.h"
#include "Shapes.h"

inline void CalculateLight(Block * pBlocks, int32 iIndexX, int32 iIndexY, XMVECTOR * pLights)
{
	const int32 indices[] =
	{
		-iIndexX - iIndexY,
		-iIndexX,
		-iIndexX + iIndexY,
		iIndexX - iIndexY,
		iIndexX,
		iIndexX + iIndexY,
		iIndexX - iIndexY,
		iIndexX,
		iIndexX + iIndexY,
	};

	XMVECTOR	trueLights[9];
	bool		opaque[9];

	for (uint32 i = 0; i < 9; i++)
		opaque[i] = pBlocks[indices[i]].mOpaque;

	opaque[0] |= (opaque[1] & opaque[3]);
	opaque[2] |= (opaque[1] & opaque[5]);
	opaque[6] |= (opaque[3] & opaque[7]);
	opaque[8] |= (opaque[5] & opaque[7]);

	for (uint32 i = 0; i < 9; i++) if (!opaque[i])
		trueLights[i] = pBlocks[indices[i]].mLight.FColor();

	trueLights[0] += trueLights[1];
	trueLights[0] += trueLights[3];

	trueLights[2] += trueLights[1];
	trueLights[2] += trueLights[5];

	trueLights[6] += trueLights[3];
	trueLights[6] += trueLights[7];

	trueLights[8] += trueLights[5];
	trueLights[8] += trueLights[7];

	pLights[0] = (trueLights[0] + trueLights[4]) * 0.25f;
	pLights[1] = (trueLights[2] + trueLights[4]) * 0.25f;
	pLights[2] = (trueLights[6] + trueLights[4]) * 0.25f;
	pLights[3] = (trueLights[8] + trueLights[4]) * 0.25f;
};

size_t Cube(const Vector3& position, 
			const Vector3& color,
			const Vector3& center,
			const Vector3& extents,
			CHAR culling, 
			Block block, 
			std::vector<BLOCK_MESH> & mesh)
{
	const XMVECTORF32 NORMAL[6] =
	{
		{ -1.0f, -0.0f, 0.0f },
		{ 1.0f, -0.0f, 0.0f },
		{ -0.0f, -0.0f, -1.0f },
		{ -0.0f, -0.0f, 1.0f },
		{ -0.0f, -1.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
	};

	const XMVECTORF32 TANGENT[6] =
	{
		{ 0.0f, 0.0f, -1.0f },
		{ 0.0f, 0.0f, 1.0f },
		{ 1.0f, 0.0f, -0.0f },
		{ -1.0f, 0.0f, -0.0f },
		{ 0.0f, 0.0f, 1.0f },
		{ 0.0f, 0.0f, 1.0f },
	};

	const XMVECTORF32 BINORMAL[6] =
	{
		{ 0.0f, -1.0f, 0.0f },
		{ -0.0f, -1.0f, 0.0f },
		{ 0.0f, -1.0f, 0.0f },
		{ 0.0f, -1.0f, -0.0f },
		{ -1.0f, 0.0f, 0.0f },
		{ 1.0f, 0.0f, 0.0f },
	};

	const XMVECTORI32 TEXTURE[6] =
	{
		{ 2, 1, 3, 0 },
		{ 2, 1, 3, 0 },
		{ 0, 1, 3, 0 },
		{ 0, 1, 3, 0 },
		{ 0, 2, 3, 0 },
		{ 0, 2, 3, 0 },
	};

	UCHAR uv[6];
	block.GetTextureIdizes(uv);

	XMVECTOR vCenter = XMLoadFloat3(&center);
	XMVECTOR vExtents = XMLoadFloat3(&extents);
	XMVECTOR vPosition = XMLoadFloat3(&position) + vCenter;


	size_t size = 0;
	for (int i = 0; i < 6; i++) if (culling & (1 << i))
	{
		BLOCK_MESH m = {
		Vector3(XMVectorMultiplyAdd(vExtents, NORMAL[i], vPosition)),
		//Vector3((XMVECTOR)NORMAL[i]),
		Vector3(TANGENT[i] * vExtents),
		Vector3(BINORMAL[i] * vExtents),
		color,
		Vector3(_mm_permutevar_ps(XMVectorSetW(vCenter, (float)uv[i]), TEXTURE[i])),
		Vector2(_mm_permutevar_ps(vExtents, TEXTURE[i]))};
		mesh.emplace_back(m);
		size++;
	}
	return size;
}

size_t Cross(const Vector3& position,
			 const Vector3& color,
			 const Vector3& center,
			 const Vector3& extents,
			 CHAR culling,
			 Block block,
			 std::vector<BLOCK_MESH> & mesh)
{
	const XMVECTORF32 NORMAL[4] =
	{
		{ -0.7f, -0.0f, 0.7f },
		{ 0.7f, -0.0f, -0.7f },
		{ -0.7f, -0.0f, -0.7f },
		{ 0.7f, -0.0f, 0.7f },
	};

	const XMVECTORF32 TANGENT[4] =
	{
		{ -0.7f, 0.0f, -0.7f },
		{ 0.7f, 0.0f, 0.7f },
		{ 0.7f, 0.0f, -0.7f },
		{ -0.7f, 0.0f, 0.7f },
	};

	const XMVECTORF32 BINORMAL[4] =
	{
		{ 0.0f, -1.0f, -0.0f },
		{ -0.0f, -1.0f, 0.0f },
		{ 0.0f, -1.0f, 0.0f },
		{ -0.0f, -1.0f, -0.0f },
	};

	const XMVECTORI32 TEXTURE[4] =
	{
		{ 2, 1, 3, 0 },
		{ 2, 1, 3, 0 },
		{ 0, 1, 3, 0 },
		{ 0, 1, 3, 0 },
	};

	UCHAR uv[6];
	block.GetTextureIdizes(uv);

	XMVECTOR vCenter = XMLoadFloat3(&center);
	XMVECTOR vExtents = XMLoadFloat3(&extents);
	XMVECTOR vPosition = XMLoadFloat3(&position) + vCenter;

	for (int i = 0; i < 4; i++)
	{
		BLOCK_MESH m = {
			Vector3(vPosition),
			//Vector3((XMVECTOR)NORMAL[i]),
			Vector3(TANGENT[i] * vExtents),
			Vector3(BINORMAL[i] * vExtents),
			color,
			Vector3(_mm_permutevar_ps(XMVectorSetW(vCenter, (float)uv[i]), TEXTURE[i])),
			Vector2(_mm_permutevar_ps(vExtents, TEXTURE[i]))};
		mesh.emplace_back(m);
	}
	return 4;
}