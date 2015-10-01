cbuffer cbPS : register(b1)
{
	float4 ProjectionAB : packoffset(c0.x);
	float4 DOFDepths	: packoffset(c1.x);
};

Texture2D<float> DepthTexture : register(t0);

// Converts z-buffer depth to linear view-space depth
float LinearDepth(in float zBufferDepth)
{
	//if (zBufferDepth == 1.0f)
	//	zBufferDepth = 0.9f;

	return ProjectionAB.y / (zBufferDepth - ProjectionAB.x);
}

// Computes the depth of field blur factor
float BlurFactor(in float depth) 
{
	float f0 = 1.0f - saturate((depth - DOFDepths.x) / max(DOFDepths.y - DOFDepths.x, 0.01f));
	float f1 = saturate((depth - DOFDepths.z) / max(DOFDepths.w - DOFDepths.z, 0.01f));
	float blur = saturate(f0 + f1);

	return blur;
}

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD0;
};

float2 main(in VS_OUTPUT input) : SV_TARGET
{
	float depth = LinearDepth(DepthTexture.Load(int3(input.Position.xy, 0)));
	float blur = BlurFactor(depth);

	return float2(depth, blur);
}