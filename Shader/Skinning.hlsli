#include "shader_include.hlsli"

//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	float3 inPositionOS   : POSITION;
	float2 inTexCoord     : TEXCOORD0;
	float3 vInNormalOS    : NORMAL;
	float4 inWeights 	  : WEIGHTS;
	uint4  inBones 		  : BONES;

	//uint   uVertexID      : SV_VERTEXID;
};

struct VS_OUTPUT
{
	float4 vPosition : SV_POSITION;
	float3 vNormal   : NORMAL;
	float2 texCoord  : TEXCOORD0;   
};

// Input control point
struct VS_CONTROL_POINT_OUTPUT
{
	float3 vPosition : WORLDPOS;
	float3 vNormal   : NORMAL;
	float2 texCoord  : TEXCOORD0;
};

// Output control point
struct HS_CONTROL_POINT_OUTPUT
{
	float3 vPosition : WORLDPOS;
	float3 vNormal   : NORMAL;
	float2 texCoord  : TEXCOORD0;
};

// Output patch constant data.
struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[3]	: SV_TessFactor; // e.g. would be [4] for a quad domain
	float InsideTessFactor	: SV_InsideTessFactor; // e.g. would be Inside[2] for a quad domain
};

struct DS_OUTPUT
{
	float4 vPosition : SV_POSITION;
	float4 vColor    : COLOR0;
	float2 texCoord  : TEXCOORD0;
};

struct SKINNED_OUTPUT
{
	float4 vPosition;
	float3 vNormal;
};

SKINNED_OUTPUT SkinVert(VS_INPUT Input)
{
	SKINNED_OUTPUT Output = (SKINNED_OUTPUT)0;

	float3 Norm = Input.vInNormalOS;
	float4 Pos = float4(Input.inPositionOS.xyz, 1.0f);

	//Bone0
	uint iBone = Input.inBones.x;
	float fWeight = Input.inWeights.x;

	iBone *= 4;
	float4 row1 = g_bwmBuffer.Load(iBone);
	float4 row2 = g_bwmBuffer.Load(iBone + 1);
	float4 row3 = g_bwmBuffer.Load(iBone + 2);
	float4 row4 = g_bwmBuffer.Load(iBone + 3);
	matrix m = float4x4(row1, row2, row3, row4);

	Output.vPosition += fWeight * mul(Pos, m);
	Output.vNormal += fWeight * mul(Norm, (float3x3)m);

	//Bone1
	iBone = Input.inBones.y;
	fWeight = Input.inWeights.y;

	iBone *= 4;
	row1 = g_bwmBuffer.Load(iBone);
	row2 = g_bwmBuffer.Load(iBone + 1);
	row3 = g_bwmBuffer.Load(iBone + 2);
	row4 = g_bwmBuffer.Load(iBone + 3);
	m = float4x4(row1, row2, row3, row4);

	Output.vPosition += fWeight * mul(Pos, m);
	Output.vNormal += fWeight * mul(Norm, (float3x3)m);

	//Bone2
	iBone = Input.inBones.z;
	fWeight = Input.inWeights.z;

	iBone *= 4;
	row1 = g_bwmBuffer.Load(iBone);
	row2 = g_bwmBuffer.Load(iBone + 1);
	row3 = g_bwmBuffer.Load(iBone + 2);
	row4 = g_bwmBuffer.Load(iBone + 3);
	m = float4x4(row1, row2, row3, row4);

	Output.vPosition += fWeight * mul(Pos, m);
	Output.vNormal += fWeight * mul(Norm, (float3x3)m);

	//Bone3
	iBone = Input.inBones.w;
	fWeight = Input.inWeights.w;

	iBone *= 4;
	row1 = g_bwmBuffer.Load(iBone);
	row2 = g_bwmBuffer.Load(iBone + 1);
	row3 = g_bwmBuffer.Load(iBone + 2);
	row4 = g_bwmBuffer.Load(iBone + 3);
	m = float4x4(row1, row2, row3, row4);

	Output.vPosition += fWeight * mul(Pos, m);
	Output.vNormal += fWeight * mul(Norm, (float3x3)m);

	//Output.vPosition = Pos;
	//Output.vNormal = Norm;

	return Output;
}