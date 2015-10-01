#include "shader_include.hlsli"

#define numSamples 2

Texture3D		g_TextureLight	: register(t0);

struct VS_INPUT
{
	float3 Position			: POSITION;
	float3 Normal			: NORMAL;
	float3 Tangent			: TANGENT;
	float3 Binormal			: BINORMAL;
	float3 Diffuse			: COLOR;
	float3 Texture			: TEXCOORD0;
	float2 TDimension		: TEXCOORD1;
};

struct VS_OUTPUT
{
	float4 Position			: ANCHOR;
	float3 Normal			: NORMAL;
	float3 Tangent			: TANGENT;
	float3 Binormal			: BINORMAL;
	float3 Diffuse			: COLOR0;
	float4 RayleighColor    : COLOR1;
	float4 MieColor			: COLOR2;
	float3 Texture			: TEXCOORD0;
	float2 TDimension		: TEXCOORD1;
	float3 LTexture			: TEXCOORD2;
};

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;

	float4 vPositionWS = mul(float4(input.Position, 1.f), g_mWorld);
	float3 vNormalWS = input.Normal;// normalize(mul(input.Normal, (float3x3)g_mWorld));
	float3 vTangentWS = input.Tangent * 1.001f;// mul(input.Tangent, (float3x3)g_mWorld);
	float3 vBinormalWS = input.Binormal * 1.001f;// mul(input.Binormal, (float3x3)g_mWorld);

	// Get the ray from the camera to the vertex, and it's length (far point)
	float3 v3Pos = (vPositionWS.xyz - g_vEye.xyz) / 2048.f;
	v3Pos.y = g_CameraPos.y;

	float3 v3Ray = v3Pos - g_CameraPos.xyz;
	float fFar = length(v3Ray);
	v3Ray /= fFar;

	// Calculate the ray's starting position, then calculate its scattering offset
	float3 v3Start = g_CameraPos.xyz;
	float fDepth = g_fDepth;
	float fCameraAngle = dot(-v3Ray, v3Pos) / length(v3Pos);
	float fLightAngle = dot(g_LightDirection.xyz, v3Pos) / length(v3Pos);
	float fCameraScale = scale(fCameraAngle);
	float fLightScale = scale(fLightAngle);
	float fCameraOffset = fDepth*fCameraScale;
	float fTemp = (fLightScale + fCameraScale);

	// Initialize the scattering loop variables
	float fSampleLength = fFar / numSamples;
	float fScaledLength = fSampleLength * g_fScale;
	float3 v3SampleRay = v3Ray * fSampleLength;
	float3 v3SamplePoint = v3Start + v3SampleRay * 0.5;

	// Now loop through the sample rays
	float3 v3FrontColor = 0;
	float3 v3Attenuate;
	[unroll]
	for (int i = 0; i < numSamples; i++)
	{
		float fHeight = length(v3SamplePoint);
		float fDepth = exp(g_fScaleOverScaleDepth * (g_fInnerRadius - fHeight));
		float fScatter = fDepth * fTemp - fCameraOffset;
		v3Attenuate = exp(-fScatter * g_InvAttenuate.rgb);
		v3FrontColor += v3Attenuate * (fDepth * fScaledLength);
		v3SamplePoint += v3SampleRay;
	}

	// Finally, scale the Mie and Rayleigh colors
	output.Texture = input.Texture;	
	output.TDimension = input.TDimension;
	output.LTexture = input.Position + float3(1.0f, 1.0f, 1.0f) + vNormalWS * 0.5f;
	output.RayleighColor = float4(v3FrontColor * (g_InvWavelength.rgb * g_KrESun + g_KmESun), 1.f);
	output.RayleighColor *= g_TextureLight.SampleLevel(g_samPoint, output.LTexture.xzy / float3(34.0f, 34.0f, 258.0f), 0).a;
	output.MieColor = float4(v3Attenuate, saturate(dot(vNormalWS, g_LightDirection.xyz) + 0.3f));
	output.Diffuse = input.Diffuse;

	output.Position = vPositionWS;
	output.Normal = vNormalWS;
	output.Tangent = vTangentWS;
	output.Binormal = vBinormalWS;


	return output;
}

//http://www.gamedev.net/topic/461747-atmospheric-scattering-sean-oneill---gpu-gems2/page-2