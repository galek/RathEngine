struct VS_OUTPUT
{
	float4 vPosition : SV_POSITION;
	float2 texCoord  : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------
Texture2DMS<float>		g_DepthBufferMSAATexture	: register(t0);

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
float4 main( VS_OUTPUT Input ) : SV_TARGET
{
	float RD = g_DepthBufferMSAATexture.Load(int2( Input.vPosition.xy ),int( 0 ),int2( 0, 0 )).x;
	return float4( RD, RD, RD, 1 );
}