#define GridSize_ 450
#define ApronSize_ 10
#define MaxSampleRadius_ 10
#define TGSize_ 470

//=================================================================================================
// Structures
//=================================================================================================
struct CoCSample
{
	float Depth;
	float Blur;
};

//=================================================================================================
// Resources
//=================================================================================================

// Inputs
Texture2D<float2> DepthBlurTexture : register(t0);

// Samplers
SamplerState PointSampler : register(s0);

// Outputs
RWTexture2D<float2> OutputTexture : register(u0);

// Shared memory
groupshared CoCSample Samples[TGSize_];

//=================================================================================================
// Performs the horizontal CoC spread
//=================================================================================================
[numthreads(TGSize_, 1, 1)]
void main(uint3 GroupID : SV_GroupID, uint3 DispatchThreadID : SV_DispatchThreadID,
	uint3 GroupThreadID : SV_GroupThreadID, uint GroupIndex : SV_GroupIndex)
{
	// These positions are relative to the "grid", AKA the horizontal group of pixels
	// that this thread group is writing to
	const int gridStartX = GroupID.x * GridSize_;
	const int gridX = GroupThreadID.x - ApronSize_;

	// These positions are relative to the pixel coordinates
	const int sampleX = gridStartX + gridX;
	const int sampleY = GroupID.y;

	int2 textureSize;
	DepthBlurTexture.GetDimensions(textureSize.x, textureSize.y);

	const int2 samplePos = int2(sampleX, sampleY);

	// Sample the textures
	float2 sampleCoord = saturate((samplePos + 0.5f) / float2(textureSize));
	float2 depthBlur = DepthBlurTexture.SampleLevel(PointSampler, sampleCoord, 0.0f).xy;
	float depth = depthBlur.x;
	float blur = depthBlur.y;
	float cocSize = blur * MaxSampleRadius_;

	// Store in shared memory
	Samples[GroupThreadID.x].Depth = depth;
	Samples[GroupThreadID.x].Blur = blur;

	GroupMemoryBarrierWithGroupSync();

	// Don't continue for threads in the apron, and threads outside the render target size
	if (gridX >= 0 && gridX < GridSize_ && sampleX < textureSize.x)
	{
		float outputBlur = 0.0f;
		float totalContribution = 0.0f;

		// Gather sample taps inside the radius
		for (int x = -MaxSampleRadius_; x <= MaxSampleRadius_; ++x)
		{
			// Grab the sample from shared memory
			int groupTapX = GroupThreadID.x + x;
			CoCSample tap = Samples[groupTapX];

			// Only accept samples if they're from the foreground, and have a higher blur amount
			float depthWeight = tap.Depth <= depth;
			float blurWeight = saturate(tap.Blur - blur);
			float tapWeight = depthWeight * blurWeight;

			// If it's the center tap, set the weight to 1 so that we don't reject it
			float centerWeight = x == 0 ? 1.0 : 0.0f;
			tapWeight = saturate(tapWeight + centerWeight);

			outputBlur += tap.Blur * tapWeight;
			totalContribution += tapWeight;
		}

		// Write out the result
		OutputTexture[samplePos] = float2(depth, outputBlur / totalContribution);
	}
}