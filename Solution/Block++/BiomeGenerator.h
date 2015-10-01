#pragma once
#include <noise.h>
#include <random>

enum class Biome : uint8
{
	Plains,
	Forrest,
	Mountains,
	Desert,
	Swamp,
	Taiga,
	Jungle,
	BiomeMax,
};

const PWCHAR BiomeToStr(Biome biome);

class BiomeGenerator
{
protected:
	uint64					mSeed;
	noise::module::Perlin	mHumidityNoise;
	noise::module::Perlin	mTemperatureNoise;

	static const BiomeGenerator g_BiomeGenerator;

	BiomeGenerator();
public:
	static float	getHumidity(const Vector3& position);
	static float	getTemperatur(const Vector3& position);
	static Biome	getBiome(const Vector3& position);
	static Vector3	getBiomeColor(const Vector3& position);
};

