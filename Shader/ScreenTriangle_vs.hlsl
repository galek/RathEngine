struct VS_OUTPUT
{
	float4 vPosition : SV_POSITION;
	float2 texCoord  : TEXCOORD0;
};

VS_OUTPUT main( uint id : SV_VertexID )
{
	VS_OUTPUT Output = (VS_OUTPUT)0;
	Output.texCoord  = float2((id << 1) & 2, id & 2);
	Output.vPosition = float4(Output.texCoord * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
	return Output;
}