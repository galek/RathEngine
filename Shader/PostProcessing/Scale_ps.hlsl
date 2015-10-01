#include "shader_lum_include.hlsli"

// Uses hw bilinear filtering for upscaling or downscaling
float4 main(in PS_INPUT input) : SV_Target
{
    return g_tex0.Sample(g_samLinear, input.TexCoord);
}