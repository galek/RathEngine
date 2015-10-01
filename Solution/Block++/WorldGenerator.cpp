#include "stdafx.h"
#include "WorldGenerator.h"
#include "Chunk.h"

const WorldGenerator WorldGenerator::g_WorldGenerator;

WorldGenerator::WorldGenerator() :
mSeed(34234234),
mOctaves(8)
{
	mTerrainNoise.SetSeed((int)mSeed);
	mOceanNoise.SetSeed((int)mSeed + 1);
	mRiverNoise.SetSeed((int)mSeed + 2);
	mMountainNoise.SetSeed((int)mSeed + 3);
	mHillNoise.SetSeed((int)mSeed + 4);
}

float WorldGenerator::calcBaseTerrain(const XMVECTOR& position) const
{
	Vector3 pos = XMVectorMultiply(position, XMVectorSet(0.002f, 0.002f, 0.002f, 0.0f));
	return (float)mTerrainNoise.GetValue(pos.x, pos.y, pos.z);
}

float WorldGenerator::calcOceanTerrain(const XMVECTOR& position) const
{
	Vector3 pos = XMVectorMultiply(position, XMVectorSet(0.0009f, 0.0009f, 0.0009f, 0.0f));
	return clamp((float)mOceanNoise.GetValue(pos.x, pos.y, pos.z) * 8.0f, 0.0f, 1.0f);
}

float WorldGenerator::calcRiverTerrain(const XMVECTOR& position) const
{
	Vector3 pos = XMVectorMultiply(position, XMVectorSet(0.0008f, 0.0008f, 0.0008f, 0.0f));
	return clamp((sqrt(abs((float)mRiverNoise.GetValue(pos.x, pos.y, pos.z))) - 0.1f) * 7.0f, 0.0f, 1.0f);
}

float WorldGenerator::calcMountainDensity(const XMVECTOR& position) const
{
	Vector3 pos = XMVectorMultiply(position, XMVectorSet(0.002f, 0.001f, 0.002f, 0.0f));
	float result = (float)mMountainNoise.GetValue(pos.x, pos.y, pos.z);
	result *= abs(result);
	return result > 0.0f ? result : 0.0f;
}

float WorldGenerator::calcHillDensity(const XMVECTOR& position) const
{
	Vector3 pos = XMVectorMultiply(position, XMVectorSet(0.008f, 0.006f, 0.008f, 0.0f));
	return clamp((float)mHillNoise.GetValue(pos.x, pos.y, pos.z), 0.0f, 1.0f);
}

WorldGenerator::Density WorldGenerator::calcDensity(const VectorInt3& position) const
{
	XMVECTOR pos = position.Float();
	const float base = calcBaseTerrain(pos);
	const float ocean = calcOceanTerrain(pos);
	const float river = calcRiverTerrain(pos);
	const float hill = calcHillDensity(pos);
	const float basescale = 10.0f;
	const float baseheight = 128.0f + basescale;
	const float hillscale = 20.0f;
	//const float densityMountains = calcMountainDensity(pos);
	//const float densityHills = calcHillDensity(pos);

	float plateauArea = 26.0f;
	float flatten = clamp((200.0f - XMVectorGetY(pos)) / plateauArea, 0.0f, 1.0f);

	Density result;
	//result.density = -XMVectorGetY(pos) + (16.0f + ((32.0f + ((16.0f + base * basescale) * 
	//	clamp(river + 0.25f, 0.0f, 1.0f))) * 
	//	clamp(ocean + 0.25f, 0.0f, 1.0f))/* + densityHills * 24.0 + densityMountains * 1024.0*/) * 
	//	flatten;

	result.density = baseheight + (base * basescale) + (hill * hillscale) - (float)position.y;

	return result;
}

void WorldGenerator::triLerpDensityMap(Density(*densityBuffer)[(CHUNK_WIDTH / 4) + 1][(CHUNK_WIDTH / 4) + 1], Density(*density)[CHUNK_WIDTH][CHUNK_WIDTH]) const
{
	for (size_t y = 0; y < (CHUNK_HEIGHT / 4); y++)
	{
		for (size_t x = 0; x < (CHUNK_WIDTH / 4); x++)
		{
			for (size_t z = 0; z < (CHUNK_WIDTH / 4); z++)
			{
				Density Corners[] =
				{
					densityBuffer[y][z][x],
					densityBuffer[y][z][x + 1],
					densityBuffer[y][z + 1][x],
					densityBuffer[y][z + 1][x + 1],
					densityBuffer[y + 1][z][x],
					densityBuffer[y + 1][z][x + 1],
					densityBuffer[y + 1][z + 1][x],
					densityBuffer[y + 1][z + 1][x + 1],
				};

				for (size_t iy = 0; iy <= 4; iy++) if (iy + y * 4 < CHUNK_HEIGHT)
				{
					for (size_t ix = 0; ix <= 4; ix++) if (ix + x * 4 < CHUNK_WIDTH)
					{
						for (size_t iz = 0; iz <= 4; iz++) if (iz + z * 4 < CHUNK_WIDTH)
						{
							density[iy + y * 4][iz + z * 4][ix + x * 4].trilerp(Corners, (float)(ix) / 4.0f, (float)(iy) / 4.0f, (float)(iz) / 4.0f);
						}
					}
				}
			}
		}
	}

	free(densityBuffer);
}

void WorldGenerator::GenerateOuterLayer(int y, Biome biome, int& firstBlockHeight, Block &block, Density& density) const
{
	int depth = (firstBlockHeight - y);

	if (biome == Biome::Desert)
	{
		block = Block::Sand;
	}
	else
	{
		if (depth == 0)// && y >= 100)
			block = Block::Grass;
		else
			block = Block::Dirt;
	}

	return;
}

void WorldGenerator::GenerateInnerLayer(int y, Biome biome, int& firstBlockHeight, Block &block, Density& density) const
{
	block = Block::Stone;
}

int WorldGenerator::ChunkHeightEstimate(VectorInt3 position) const
{
	position.x += 8; position.z += 8; position.y += 40;

	while (calcDensity(position).density >= 5.0)
	{
		position.y += 16;
	}

	return position.y - 16;
}

float DistancePointToChunk(const VectorInt3& position)
{
	XMINT3 v = position, q;

	if (v.x < 0) v.x = 0; else if (v.x > CHUNK_WIDTH_INDEX) v.x = CHUNK_WIDTH_INDEX;
	q.x = position.x - v.x;

	if (v.y < 0) v.y = 0; else if (v.y > CHUNK_HEIGHT_INDEX) v.y = CHUNK_HEIGHT_INDEX;
	q.y = position.y - v.y;

	if (v.z < 0) v.z = 0; else if (v.z > CHUNK_WIDTH_INDEX) v.z = CHUNK_WIDTH_INDEX;
	q.z = position.z - v.z;

	return sqrt(float(q.x * q.x + q.y * q.y + q.z * q.z));
}

void WormDrill(const XMVECTOR& Direction, VectorInt3& position, int distance, Chunk* pChunk)
{
	VectorInt3 Position = position;

	XMVECTOR diff = XMVectorGreater(Direction, g_XMZero);
	XMVECTOR step = XMVectorSelect(XMVectorSelect(g_XMNegativeOne, g_XMOne, diff), g_XMZero, XMVectorNearEqual(Direction, g_XMZero, g_XMEpsilon));
	XMVECTOR cellBoundary = XMVectorSelect(g_XMZero, g_XMOne, diff);
	XMVECTOR tMax = cellBoundary / Direction;
	XMVECTOR tDelta = step / Direction;

	XMINT3 Step;	XMStoreSInt3(&Step, step);
	XMFLOAT3 Max;	XMStoreFloat3(&Max, tMax);
	XMFLOAT3 Delta;	XMStoreFloat3(&Delta, tDelta);

	while ((distance--) > 0)
	{
		for (int x = Position.x - 1; x <= Position.x + 1; x++) if ((x & CHUNK_WIDTH_INVMASK) == 0)
			for (int z = Position.z - 1; z <= Position.z + 1; z++) if ((z & CHUNK_WIDTH_INVMASK) == 0)
				for (int y = Position.y - 1; y <= Position.y + 1; y++) if ((y < pChunk->mLoadedHeight) && (y >= 0))
					pChunk->mBlocks[y][z][x] = Block::Air;

		// Do the next step.
		if (Max.x < Max.y && Max.x < Max.z)
		{
			Position.x += Step.x;
			Max.x += Delta.x;
		}
		else if (Max.y < Max.z)
		{
			Position.y += Step.y;
			Max.y += Delta.y;
			Position.y = max(Position.y, 6);
		}
		else
		{
			Position.z += Step.z;
			Max.z += Delta.z;
		}
	}

	position = Position;
	return;
}

void WorldGenerator::EvaluateCaves(Chunk* pChunk) const
{
	noise::module::Perlin				perlinWorm;
	std::default_random_engine			generator;
	XMINT3								position = pChunk->mPosition;
	perlinWorm.SetOctaveCount(3);

	const int range = CHUNK_WIDTH * 4;
	for (int x = -range; x <= range; x += CHUNK_WIDTH)
	{
		for (int z = -range; z <= range; z += CHUNK_WIDTH)
		{
			unsigned long seed = ((unsigned long(mSeed) * 73856093) ^ (unsigned long(x + position.x) * 19349663) ^ (unsigned long(z + position.z) * 83492791));
			generator.seed(seed);
			std::uniform_int_distribution<uint32>	distA(0, 2);
			if (distA(generator) == 0)
			{
				perlinWorm.SetSeed((int)seed);
				std::uniform_int_distribution<uint32>	distXZ(0, CHUNK_WIDTH_INDEX);
				std::uniform_int_distribution<uint32>	distY(10, ChunkHeightEstimate(VectorInt3(x + position.x, 0, z + position.z)));
				VectorInt3 startpoint = VectorInt3(distXZ(generator) + x, distY(generator), distXZ(generator) + z);
				for (float i = 0; i < 64.0; i++)
				{
					float pitch = float(perlinWorm.GetValue(i / 50.0, 0.1, 0.1)) * XM_PIDIV4;
					float yaw = float(perlinWorm.GetValue(i / 50.0, 1.1, 1.1)) * XM_2PI;

					XMVECTOR rotation = XMQuaternionRotationRollPitchYaw(pitch, yaw, 0.0f);
					XMVECTOR direction = XMVector3Normalize(XMVector3Rotate(g_XMIdentityR2, rotation));

					WormDrill(direction, startpoint, 8, pChunk);
				}
			}
		}
	}
}

void WorldGenerator::Generate(Chunk* pChunk) const
{
	union
	{
		// Access: Y(256) - Z(16) - X(1)
		Density (*density)[CHUNK_WIDTH][CHUNK_WIDTH];
		Density  *densityDirect;
	};
	densityDirect = (Density*)malloc(CHUNK_SQRWIDTH * CHUNK_HEIGHT * sizeof(Density));
	auto densityBuffer = (Density(*)[(CHUNK_WIDTH / 4) + 1][(CHUNK_WIDTH / 4) + 1]) malloc(((CHUNK_HEIGHT / 4) + 1) * ((CHUNK_WIDTH / 4) + 1) * ((CHUNK_WIDTH / 4) + 1) * sizeof(Density));

	XMINT3 position = pChunk->mPosition;
	for (int y = 0; y <= (CHUNK_HEIGHT / 4); y++)
	{
		for (int x = 0; x <= (CHUNK_WIDTH / 4); x++)
		{
			for (int z = 0; z <= (CHUNK_WIDTH / 4); z++)
			{
				densityBuffer[y][z][x] = calcDensity(VectorInt3(x * 4 + position.x, y * 4, z * 4 + position.z));
			}
		}
	}
	triLerpDensityMap(densityBuffer, density);

	int height = CHUNK_HEIGHT_INDEX;
	for (height; height > 64; height-- )
	{
		for (size_t z = 0; z < CHUNK_WIDTH; z++)
		{
			for (size_t x = 0; x < CHUNK_WIDTH; x++)
			{
				if (density[height][z][x].density >= 0.0)
					goto FinishHieght;
			}
		}
	}
FinishHieght:
	height = 191;
	pChunk->Resize(WORD(height + 1));
	for (int z = 0; z < CHUNK_WIDTH; z++)
	{
		for (int x = 0; x < CHUNK_WIDTH; x++)
		{
			int firstBlockHeight = -1;

			Biome biome = BiomeGenerator::getBiome(Vector3(float(position.x + x), 0, float(position.z + z)));

			for (int y = height; y >= 0; y--)
			{
				if (density[y][z][x].density > 0.0)
				{
					if (firstBlockHeight == -1)
						firstBlockHeight = y;

					if (density[y][z][x].density < 4.0001)
						GenerateOuterLayer(y, biome, firstBlockHeight, pChunk->mBlocks[y][z][x], density[y][z][x]);
					else
						GenerateInnerLayer(y, biome, firstBlockHeight, pChunk->mBlocks[y][z][x], density[y][z][x]);
				}

				/*if (firstBlockHeight > 0 && pChunk->mHeight[z][x] == 0)
					pChunk->mHeight[z][x] = (UCHAR)firstBlockHeight;*/
			}
		}
	}

	free(densityDirect);

	EvaluateCaves(pChunk);
}

void WorldGenerator::GenerateForest(Chunk* pChunk) const
{
	VectorInt3 position = pChunk->mPosition;
	std::default_random_engine	generator;
	unsigned long seed = ((unsigned long(mSeed) * 73856093) ^ (unsigned long(position.x) * 19349663) ^ (unsigned long(position.z) * 83492791));
	generator.seed(seed);
	std::uniform_int_distribution<uint32>	distT(0, 20);
	std::uniform_int_distribution<uint32>	distG(0, 5);
	std::uniform_int_distribution<uint32>	distXY(0, CHUNK_WIDTH_INDEX);

	while (distT(generator) > 8) // Trees 
	{
		UINT x = distXY(generator);
		UINT z = distXY(generator);
		UINT y = pChunk->mHeight[z + 1][x + 1];
		Block & groundBlock = pChunk->mBlocks[y][z][x];

		if ((y <= 50) || (groundBlock.mBlocktype != Blocktype::Grass)) continue;

		VectorInt3 Position = VectorInt3(x, y, z) + position;
		VectorInt3 sides[] = {
			VectorInt3(1, 1, 0),
			VectorInt3(-1, 1, 0),
			VectorInt3(0, 1, 1),
			VectorInt3(0, 1, -1),
		};

		BOOL cont = TRUE;
		for (UINT i = 0; (i < 4) && cont; i++) {
			cont = !pChunk->GetBlock(Position + sides[i]).mOpaque;
		}
		if (cont == FALSE) continue;

		groundBlock = Block::Dirt;
		UINT height = ((rand() % 4) + 5);
		for (UINT h = 1; h <= height; h++) {
			if (h != height)
				pChunk->SetBlock(Position + VectorInt3(0, h, 0), Block::Wood);
			if (h > 2) {
				int r = (height - h) * 2 + 1;
				for (int i = -3; i < 4; i++) {
					int a = i * i;
					for (int j = -3; j < 4; j++) {
						int b = j * j;
						Block leaf = Block::Leaf;
						leaf.mMetaLo = max(0, 5 - (abs(i) + abs(j)) - max(0, h - height - 1));
						if ((a + b) <= (r - 1)) {
							pChunk->SetBlock(Position + VectorInt3(i, h, j), leaf);
						}
						else if ((a + b) <= r) {
							if ((rand() % 2) == 0) {
								pChunk->SetBlock(Position + VectorInt3(i, h, j), leaf);
							}
						}
					}
				}

			}
		}
	}

	while (distG(generator) > 0) // Grass
	{
		UINT x = distXY(generator);
		UINT z = distXY(generator);
		UINT y = pChunk->mHeight[z + 1][x + 1];
		Block & groundBlock = pChunk->mBlocks[y][z][x];

		if (groundBlock.mBlocktype == Blocktype::Grass)
		{
			Block & selectedBlock = pChunk->mBlocks[y + 1][z][x];
			if (selectedBlock.mBlocktype == Blocktype::Air)
			{
				Light l = selectedBlock.mLight;
				l -= 1;
				selectedBlock = Block::TallGrass;
				selectedBlock.mLight = l;
			}
		}
	}
}

void WorldGenerator::GenerateStructures(Chunk* pChunk) const
{
	VectorInt3 position = pChunk->mPosition;
	std::default_random_engine	generator;
	unsigned long seed = ((unsigned long(mSeed) * 73856093) ^ (unsigned long(position.x) * 19349663) ^ (unsigned long(position.z) * 83492791));
	generator.seed(seed);
	std::uniform_int_distribution<uint32>	distA(0, 16);

	if (distA(generator) == 0) 
	{
		std::uniform_int_distribution<uint32>	distXY(0, CHUNK_WIDTH_INDEX);
		std::uniform_int_distribution<uint32>	distY(5, 75);

		UINT x = distXY(generator);
		UINT z = distXY(generator);
		UINT y = distY(generator);

		const VectorInt3 corners[] = {
			VectorInt3(0, 0, 0),
			VectorInt3(9, 5, 13),
		};
		VectorInt3 Position = VectorInt3(x, y, z) + position;

		//if ((pChunk->GetBlock(corners[0] + Position).isLiquid()) || (pChunk->GetBlock(corners[7] + Position).isLiquid()))
		//	return;

		VectorInt3 itPos(corners[0]);
		for (itPos.x = corners[0].x; itPos.x <= corners[1].x; itPos.x++)
			for (itPos.y = corners[0].y; itPos.y <= corners[1].y; itPos.y++)
				for (itPos.z = corners[0].z; itPos.z <= corners[1].z; itPos.z++)
				{
					Block block = pChunk->GetBlock(itPos + Position);
					if (block.mBlocktype == Blocktype::Air)
						continue;

					if ((itPos.x == corners[0].x) ||
						(itPos.x == corners[1].x) ||
						(itPos.y == corners[0].y) ||
						(itPos.y == corners[1].y) ||
						(itPos.z == corners[0].z) ||
						(itPos.z == corners[1].z))
						pChunk->SetBlock(itPos + Position, Block::Cobble);
					else
						pChunk->SetBlock(itPos + Position, Block::Air);
				}
		pChunk->SetBlock(VectorInt3(4, 1, 7) + Position, Block::Lightstone);
	}

	//for (int x = 0; x < 32; x++)
	//	for (int z = 0; z < 32; z++)
	//		pChunk->SetBlock(VectorInt3(x, 160, z) + position, Block::Cobble);
}

void WorldGenerator::GenerateChunk(Chunk* pChunk)
{
	g_WorldGenerator.Generate(pChunk);

	pChunk->PlaceSkyLight();
}

void WorldGenerator::PopulateChunk(Chunk* pChunk)
{
	g_WorldGenerator.GenerateStructures(pChunk);
	g_WorldGenerator.GenerateForest(pChunk);

	pChunk->PropagateSkyLight();
}