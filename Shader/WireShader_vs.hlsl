#include "shader_include.hlsli"

struct VS_INPUT
{
	float4 Position     : POSITION;  // vertex position 
};

struct PS_INPUT
{
	float4 Position     : SV_POSITION; // vertex position 
};

PS_INPUT main(VS_INPUT input)
{
	PS_INPUT Output = (PS_INPUT)0;

	Output.Position = mul(input.Position, g_mViewProjection);
	return Output;
};