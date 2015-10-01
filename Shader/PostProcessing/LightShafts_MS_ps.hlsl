#include "../shader_include.hlsli"

Texture2DMS<float4> g_BackBufferTexture : register(t0);
Texture2D<float4>	g_LightTexture : register(t1);

struct VS_OUTPUT
{
	float4 vPosition : SV_POSITION;
	float2 texCoord  : TEXCOORD0;
};

static const uint NUM_SAMPLES = 64;
static const float Weight = 0.4f;
static const float Exposure = 0.025f;
static const float Density = 0.6f;
static const float Decay = 0.975f;
float4 main(VS_OUTPUT Input) : SV_TARGET
{
	float2 texCoord = Input.texCoord;
	// Calculate vector from pixel to light source in screen space.  
	float2 deltaTexCoord = (texCoord - g_LightScreenPosition.xy);
	float len = min(length(deltaTexCoord), 0.75f);
	deltaTexCoord = normalize(deltaTexCoord) * len;
	// Divide by number of samples and scale by control factor.  
	deltaTexCoord *= 1.0f / NUM_SAMPLES * Density;
	// Store initial sample.  
	float3 color = (float3)g_LightTexture.Sample(g_samPoint, Input.texCoord);
	// Set up illumination decay factor.  
	float illuminationDecay = 1.0f;
	// Evaluate summation from Equation 3 NUM_SAMPLES iterations.  
	for (uint i = 0; i < NUM_SAMPLES; i++)
	{
		// Step sample location along ray.  
		texCoord -= deltaTexCoord;
		// Retrieve sample at new location.  
		float3 sample = (float3)g_LightTexture.Sample(g_samPoint, Input.texCoord);
		// Apply sample attenuation scale/decay factors.  
		sample *= illuminationDecay * Weight;
		// Accumulate combined color.  
		color += sample;
		// Update exponential decay factor.  
		illuminationDecay *= Decay;
	}
	// Output final color with a further scale control factor.  
	float3 backBuffer = (float3)g_BackBufferTexture.Load(int2(Input.vPosition.xy), 0, int2(0, 0));
	return float4(backBuffer + color * Exposure * g_LightScreenPosition.w, 1.0f);
}