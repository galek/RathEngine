struct PS_INPUT
{
	float4 Position     : SV_POSITION; // vertex position 
};

float4 main(PS_INPUT In) : SV_TARGET
{ 
	return float4(1.0f, 0.0f, 1.0f, 1.0f);
};