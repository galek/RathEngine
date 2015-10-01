
cbuffer cbMSAA : register(b1)
{
	float	g_MSAA_Detect_fDepthEpsilonPercent;
	int		g_MSAA_Detect_fSampleCount;
	int		padding[2];
}

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
	float fDepth0 = g_DepthBufferMSAATexture.Load(int2( Input.vPosition.xy ), 0, int2( 0, 0 ) );
	float fEpsilon = fDepth0 * g_MSAA_Detect_fDepthEpsilonPercent;
	float fComplexSamples = 0.0f;

	[loop] for( int i = 1; i < g_MSAA_Detect_fSampleCount; i++ )
	{
		float fDepthi = g_DepthBufferMSAATexture.Load(int2( Input.vPosition.xy ), i, int2( 0, 0 ) );

		if( abs( fDepth0 - fDepthi ) > fEpsilon )
		{
			fComplexSamples += 1.0f;
		}
	}

	if( fComplexSamples == 0.0f  )
	{
		discard;
	}

	return float4( 0, 0, 0, 0 );
}