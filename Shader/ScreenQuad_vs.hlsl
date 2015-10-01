
//--------------------------------------------------------------------------------------
// shader input/output structure
//--------------------------------------------------------------------------------------
struct VS_SCREEN_QUAD_INPUT
{
	float3 f3Position : POSITION;
	float2 f2TexCoord : TEXCOORD0;
};

struct PS_SCREEN_QUAD_INPUT
{
	float4 f4Position : SV_POSITION;
	float2 f2TexCoord : TEXCOORD0;
};

PS_SCREEN_QUAD_INPUT main(VS_SCREEN_QUAD_INPUT IN)
{
	PS_SCREEN_QUAD_INPUT O;

	O.f4Position.x = IN.f3Position.x;
	O.f4Position.y = IN.f3Position.y;
	O.f4Position.z = IN.f3Position.z;
	O.f4Position.w = 1.0f;

	O.f2TexCoord = IN.f2TexCoord;

	return O;
}