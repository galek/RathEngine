#include "BlockGS.hlsli"

[maxvertexcount(4)]
void main(point GS_INPUT input[1], inout TriangleStream< GS_OUTPUT > stream)
{
	GS(input, stream);
}