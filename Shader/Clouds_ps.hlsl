#include "shader_include.hlsli"

/////////////
// GLOBALS //
/////////////
Texture2D cloudTexture : register(t0);
Texture2D perturbTexture : register(t1);


cbuffer cbClouds : register(b3)
{
	float g_translation;
	float g_scale;
	float g_brightness;
	float pad;
};


//////////////
// TYPEDEFS //
//////////////
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
[earlydepthstencil]
float4 main(PixelInputType input) : SV_TARGET
{
	float4 perturbValue;
	float4 cloudColor;

	// Translate the texture coordinate sampling location by the translation value.
	input.tex.x = input.tex.x + g_translation;

	// Sample the texture value from the perturb texture using the translated texture coordinates.
	perturbValue = perturbTexture.Sample(g_samLinearWrap, input.tex);

	// Multiply the perturb value by the perturb scale.
	perturbValue = perturbValue * g_scale;

	// Add the texture coordinates as well as the translation value to get the perturbed texture coordinate sampling location.
	perturbValue.xy = perturbValue.xy + input.tex.xy + g_translation;

	// Now sample the color from the cloud texture using the perturbed sampling coordinates.
	cloudColor = cloudTexture.Sample(g_samLinearWrap, perturbValue.xy);

	// Reduce the color cloud by the brightness value.
	cloudColor = cloudColor * g_brightness;

	return cloudColor;
}