#include "shader_include.hlsli"

struct DS_OUTPUT
{
	float4 Position			: SV_POSITION;
	float3 Direction		: TEXCOORD0;
	float4 RayleighColor    : TEXCOORD1;
	float4 MieColor			: TEXCOORD2;
};

// Output control point
struct HS_CONTROL_POINT_OUTPUT
{
	float3 Position : WORLDPOS; 
};

// Output patch constant data.
struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[3]			: SV_TessFactor; // e.g. would be [4] for a quad domain
	float InsideTessFactor			: SV_InsideTessFactor; // e.g. would be Inside[2] for a quad domain
};

#define NUM_SAMPLES 8
#define NUM_CONTROL_POINTS 3

[domain("tri")]
DS_OUTPUT main(
	HS_CONSTANT_DATA_OUTPUT input,
	float3 domain : SV_DomainLocation,
	const OutputPatch<HS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> patch)
{
	DS_OUTPUT Output;

	Output.Position = float4(normalize(patch[0].Position*domain.x + patch[1].Position*domain.y + patch[2].Position*domain.z), 1.0f);

	// Get the ray from the camera to the vertex, and it's length (far point)
	float3 v3Pos = Output.Position.xyz;
	v3Pos.y += g_fInnerRadius;

	float3 v3Ray = v3Pos - g_CameraPos.xyz;
	float fFar = length(v3Ray);
	v3Ray /= fFar;

	// Calculate the ray's starting position, then calculate its scattering offset
	float3 v3Start = g_CameraPos.xyz;
	float fHeight = length(v3Start);
	float fDepth = g_fDepth;
	float fStartAngle = dot(v3Ray, v3Start) / fHeight;
	float fStartOffset = fDepth * scale(fStartAngle);

	// Init loop variables
	float fSampleLength = fFar / NUM_SAMPLES;
	float fScaledLength = fSampleLength * g_fScale;
	float3 v3SampleRay = v3Ray * fSampleLength;
	float3 v3SamplePoint = v3Start + v3SampleRay * 0.5f;

	// Loop the ray
	float3 color = 0.f;
	[unroll]
	for (int i = 0; i < NUM_SAMPLES; i++)
	{
		float fHeight = length(v3SamplePoint);
		float fDepth = exp(g_fScaleOverScaleDepth * (g_fInnerRadius - fHeight));

		float fLightAngle = dot(g_LightDirection.xyz, v3SamplePoint) / fHeight;
		float fCameraAngle = dot(v3Ray, v3SamplePoint) / fHeight;

		float fScatter = (fStartOffset + fDepth*(scale(fLightAngle) - scale(fCameraAngle)));
		float3 v3Attenuate = exp(-fScatter * g_InvAttenuate.rgb);

		// Accumulate color
		v3Attenuate *= (fDepth * fScaledLength);
		color += v3Attenuate;

		// Next sample point
		v3SamplePoint += v3SampleRay;
	}

	Output.Position = mul(Output.Position, g_mCenterViewProjection);
	Output.Position.z = Output.Position.w;
	// Finally, scale the Mie and Rayleigh colors
	Output.RayleighColor = float4(color * (g_InvWavelength.rgb * g_KrESun), 1.f);
	//Output.RayleighColor.a = length(Output.RayleighColor.rgb);
	Output.MieColor = float4(color * g_KmESun, 1.f);
	//Output.MieColor.a = length(Output.MieColor.rgb);
	Output.Direction = g_CameraPos.xyz - v3Pos;

	return Output;
}
