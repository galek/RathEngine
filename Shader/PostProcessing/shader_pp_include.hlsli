//--------------------------------------------------------------------------------------
// Samplers
//--------------------------------------------------------------------------------------
SamplerState	g_samPoint		: register(s0);
SamplerState	g_samLinear		: register(s1);
SamplerState	g_samAnisotropic: register(s2);
SamplerState	g_samBilinear	: register(s3);

//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------

Texture2D g_tex0 : register(t0);
Texture2D g_tex1 : register(t1);
Texture2D g_tex2 : register(t2);
Texture2D g_tex3 : register(t3);

Texture2D<int4> g_texInt0 : register(t0);
Texture2D<int4> g_texInt1 : register(t1);
Texture2D<int4> g_texInt2 : register(t2);
Texture2D<int4> g_texInt3 : register(t3);

Texture2D<uint4> g_texUint0 : register(t0);
Texture2D<uint4> g_texUint1 : register(t1);
Texture2D<uint4> g_texeUint2 : register(t2);
Texture2D<uint4> g_texUint3 : register(t3);

struct PS_INPUT
{
	float4 Position  : SV_POSITION;
	float2 TexCoord  : TEXCOORD0;
};

cbuffer PSConstants : register(b0)
{
	float2 g_InputSize0 : packoffset(c0.x);
	float2 g_InputSize1 : packoffset(c0.z);
	float2 g_InputSize2 : packoffset(c1.x);
	float2 g_InputSize3 : packoffset(c1.z);
	float2 g_OutputSize : packoffset(c2.x);
}

static const uint ReductionTGSize = 16;
static const uint CullTGSize = 128;
static const uint BatchTGSize = 256;
static const uint NumThreads = ReductionTGSize * ReductionTGSize;