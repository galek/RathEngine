struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex  : TEXCOORD0;
};

static const float  MIDDLE_GRAY = 0.72f;
static const float  LUM_WHITE = 1.5f;
static const float4 LUM_VECTOR = float4(.299, .587, .114, 0);
static const float  BRIGHT_THRESHOLD = 0.7f;

SamplerState	g_samPoint		: register(s0);
SamplerState	g_samLinear		: register(s1);

Texture2D s0 : register(t0);
Texture2D s1 : register(t1);
Texture2D s2 : register(t2);
Texture2D s3 : register(t3);

Texture2D<float4>		tex		: register(t0);
StructuredBuffer<float> lum		: register(t1);
Texture2D<float4>		bloom	: register(t2);

cbuffer cbPS : register(b0)
{
	float4    g_param;
};

cbuffer cb0 : register(b0)
{
	float4 g_avSampleOffsets[15];
	float4 g_avSampleWeights[15];
}