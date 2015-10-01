#include "shader_pp_include.hlsli"

//=================================================================================================
// Constants
//=================================================================================================
cbuffer LumConstants : register(b1)
{
	float g_BloomThreshold;
	float g_BloomMagnitude;
	float g_BloomBlurSigma;
	float g_Tau;
	float g_TimeDelta;
    float g_KeyValue;
	float g_MaxAdept;
	float g_MinAdept;
};

// Approximates luminance from an RGB value
float CalcLuminance(float3 color)
{
    return max(dot(color, float3(0.299f, 0.587f, 0.114f)), 0.0001f);
}

// Retrieves the log-average luminance from the texture
float GetAvgLuminance(Texture2D lumTex)
{
    return lumTex.Load(uint3(0, 0, 0)).x;
}

// Applies the filmic curve from John Hable's presentation
float3 ToneMapFilmicALU(float3 color)
{
    color = max(0, color - 0.004f);
    color = (color * (6.2f * color + 0.5f)) / (color * (6.2f * color + 1.7f)+ 0.06f);

    // result has 1/2.2 baked in
    return pow(color, 2.2f);
}

// Determines the color based on exposure settings
float3 CalcExposedColor(float3 color, float avgLuminance, float threshold, out float exposure)
{
    // Use geometric mean
    avgLuminance = max(avgLuminance, 0.001f);
    float linearExposure = (g_KeyValue / avgLuminance);
    exposure = log2(max(linearExposure, 0.0001f));
    exposure -= threshold;
    return exp2(exposure) * color;
}

// Applies exposure and tone mapping to the specific color, and applies
// the threshold to the exposure value.
float3 ToneMap(float3 color, float avgLuminance, float threshold, out float exposure)
{
    float pixelLuminance = CalcLuminance(color);
    color = CalcExposedColor(color, avgLuminance, threshold, exposure);
    color = ToneMapFilmicALU(color);
    return color;
}

// Calculates the gaussian blur weight for a given distance and sigmas
float CalcGaussianWeight(int sampleDist, float sigma)
{
    float g = 1.0f / sqrt(2.0f * 3.14159 * sigma * sigma);
    return (g * exp(-(sampleDist * sampleDist) / (2 * sigma * sigma)));
}

// Performs a gaussian blur in one direction
float4 Blur(in PS_INPUT input, float2 texScale, float sigma)
{
	float4 color = 0;
	for (int i = -6; i < 6; i++)
	{
		float weight = CalcGaussianWeight(i, sigma);
		float2 texCoord = input.TexCoord;
		texCoord += (i / g_InputSize0) * texScale;
		float4 sample = g_tex0.Sample(g_samPoint, texCoord);
		color += sample * weight;
	}

	return color;
}
