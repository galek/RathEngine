#include "shader_include.hlsli"

#define numSamples 8

struct VS_INPUT
{
	float3 Position : POSITION;
};

struct VS_OUTPUT
{
	float3 Position : WORLDPOS;
};

VS_OUTPUT main(VS_INPUT Input)
{
	VS_OUTPUT Output;

	Output.Position = Input.Position;

	return Output;
}