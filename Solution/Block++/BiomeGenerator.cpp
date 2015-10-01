#include "stdafx.h"
#include "BiomeGenerator.h"

const PWCHAR BiomeToStr(Biome biome)
{
	const PWCHAR strings[] =
	{
		L"PLAINS",
		L"FORREST",
		L"MOUNTAINS",
		L"DESERT",
		L"SWAMP",
		L"TAIGA",
		L"JUNGLE",
		L"VOID",
		L"NONE",
	};

	return strings[(uint8)biome];
}

const BiomeGenerator BiomeGenerator::g_BiomeGenerator;

BiomeGenerator::BiomeGenerator() :
mSeed(34234234)
{
	mHumidityNoise.SetSeed((int)mSeed);
	mTemperatureNoise.SetSeed((int)mSeed + 1);
}


float BiomeGenerator::getHumidity(const Vector3& position)
{
	return (float)g_BiomeGenerator.mHumidityNoise.GetValue(position.x / 2048.f, 0, position.z / 2048.f);
}

float BiomeGenerator::getTemperatur(const Vector3& position)
{
	return (float)g_BiomeGenerator.mTemperatureNoise.GetValue(position.x / 2048.f, 0, position.z / 2048.f);
}

Biome BiomeGenerator::getBiome(const Vector3& position)
{
	float temperature = getTemperatur(position);
	float humidity = getHumidity(position);

	Biome result = Biome::Plains;
	if ((temperature - humidity) > 0.85f)
		result = Biome::Desert;
	else if ((temperature + humidity) > 1.15f)
		result = Biome::Jungle;
	else if (temperature < -0.65)
		result = Biome::Taiga;
	else if ((humidity > 0.1) && (humidity <= 0.7))
		result = Biome::Forrest;
	else if (humidity > 0.65)
		result = Biome::Swamp;
	else if (temperature < -0.1)
		result = Biome::Mountains;
	return result;
}

Vector3 BiomeGenerator::getBiomeColor(const Vector3& position)
{
	float t = getTemperatur(position);
	float h = getHumidity(position);

	float d = clamp(((t - h) - 0.80f) * 5.0f, 0.0f, 1.0f);
	float j = clamp(((t + h) - 1.05f) * 5.0f, 0.0f, 1.0f);

	float s = clamp((h - 0.65f) * 5.0f, 0.0f, 1.0f - j);
	float i = clamp((t + 0.60f) * -5.0f, 0.0f, 1.0f);
	float n = clamp(1.0f - (j + d + s + i), 0.0f, 1.0f);

	return (Vector3(0.1f, 1.0f, 0.1f) * j) + 
		   (Vector3(0.8f, 0.7f, 0.2f) * d) + 
		   (Vector3(0.2f, 0.5f, 0.2f) * (s + i)) + 
		   (Vector3(0.2f, 01.0f, 0.3f) * n);
};