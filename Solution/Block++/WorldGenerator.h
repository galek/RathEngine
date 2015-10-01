#pragma once
#include "BiomeGenerator.h"

struct Block;
class Chunk;
class WorldGenerator
{
private:
	struct Density
	{
		float		density;
		XMVECTOR	rnd;
		inline void lerp(const Density& d1, const Density& d2, const float l)
		{
			density = d1.density * (1.0f - l) + d2.density * l;
			if (density >= 5.0f)
			{
				rnd = XMVectorLerp(d1.rnd, d2.rnd, l);
			}
		}

		inline void bilerp(const Density& d1, const Density& d2, const Density& d3, const Density& d4, const float l1, const float l2)
		{
			Density e, f;
			e.lerp(d1, d3, l1);
			f.lerp(d2, d4, l1);
			lerp(e, f, l2);
		}

		inline void trilerp(const Density* d, const float l1, const float l2, const float l3)
		{
			Density e, f;
			e.bilerp(d[0], d[1], d[2], d[3], l3, l1);
			f.bilerp(d[4], d[5], d[6], d[7], l3, l1);
			lerp(e, f, l2);
		}
	};
	void triLerpDensityMap(Density(*densityBuffer)[(CHUNK_WIDTH / 4) + 1][(CHUNK_WIDTH / 4) + 1], Density(*density)[CHUNK_WIDTH][CHUNK_WIDTH]) const;

protected:
	UINT64	mSeed;
	UINT	mOctaves;

	noise::module::Perlin	 mTerrainNoise;
	noise::module::Perlin	 mOceanNoise;
	noise::module::Perlin	 mRiverNoise;
	noise::module::Perlin	 mMountainNoise;
	noise::module::Perlin	 mHillNoise;

	float  calcBaseTerrain(const XMVECTOR& position) const;
	float  calcOceanTerrain(const XMVECTOR& position) const;
	float  calcRiverTerrain(const XMVECTOR& position) const;
	float  calcMountainDensity(const XMVECTOR& position) const;
	float  calcHillDensity(const XMVECTOR& position) const;

	Density calcDensity(const VectorInt3& position) const;

	void	GenerateOuterLayer(int y, Biome biome, int& firstBlockHeight, Block &block, Density& density) const;
	void	GenerateInnerLayer(int y, Biome biome, int& firstBlockHeight, Block &block, Density& density) const;

	int		ChunkHeightEstimate(VectorInt3 position) const;
	void	EvaluateCaves(Chunk* pChunk) const;
	WorldGenerator();

	static const WorldGenerator g_WorldGenerator;

	void Generate(Chunk* pChunk) const;
	void GenerateForest(Chunk* pChunk) const;
	void GenerateStructures(Chunk* pChunk) const;
public:
	static void GenerateChunk(Chunk* pChunk);
	static void PopulateChunk(Chunk* pChunk);
};