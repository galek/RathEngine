#include "shader_lum_include.hlsli"

// Vertical gaussian blur
float4 main(in PS_INPUT input) : SV_Target
{
    return Blur(input, float2(0, 1), g_BloomBlurSigma);
}