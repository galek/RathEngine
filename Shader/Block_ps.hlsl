#include "shader_include.hlsli"

Texture2DArray<float4>	g_TextureColor	: register(t0);
Texture2DArray<float4>	g_TextureBiome	: register(t1);
Texture3D<float4>		g_TextureLight	: register(t2);

struct PS_INPUT
{
	float4 Position						 : SV_POSITION;
	nointerpolation float3 Diffuse		 : COLOR0;
	nointerpolation float4 RayleighColor : COLOR1;
	nointerpolation float4 MieColor		 : COLOR2;
	float3 Texture		 : TEXCOORD0;
	float3 LTexture						 : TEXCOORD1;
};

[earlydepthstencil]
float4 main(PS_INPUT input) : SV_TARGET
{
	float4 Color = g_TextureColor.Sample(g_samPoint, input.Texture);
	if (Color.a < 0.1f)
		discard;
	float4 Biome = g_TextureBiome.Sample(g_samPoint, input.Texture);
	Color.rgb = lerp(Color.rgb, Biome.rgb * input.Diffuse, Biome.a);
	//Color.rgb = float3(1, 1, 1);

	float2 uv = input.Position.xy / g_vViewport.xy;
	float4 Light = g_TextureLight.Sample(g_samLinear, input.LTexture);
	float Shadow = g_shadowTexture.SampleLevel(g_samPoint, uv, 0).r * Light.a;
	float4 finalColour = (float4(input.MieColor.rgb, 1.0f) * max(min(input.MieColor.a, Shadow), 0.2f));

	finalColour *= Color;
	finalColour = finalColour * Light.a + float4(finalColour.rgb * Light.rgb, 0.0f);

	if (Light.a > 0.01f)
		finalColour += input.RayleighColor;

	return finalColour;
}