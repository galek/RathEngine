#include "shader_lum_include.hlsli"

// Horizontal gaussian blur
float4 main(in PS_INPUT input) : SV_Target
{
    return Blur(input, float2(1, 0), g_BloomBlurSigma);
}