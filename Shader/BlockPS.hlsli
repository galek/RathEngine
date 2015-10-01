#include "shader_include.hlsli"

Texture2DArray	g_TextureColor	: register(t0);
Texture2DArray	g_TextureBiome	: register(t1);
Texture3D		g_TextureLight	: register(t2);

struct PS_INPUT
{
	float4 Position			: SV_POSITION;
	float3 Diffuse			: COLOR0;
	float4 RayleighColor    : COLOR1;
	float4 MieColor			: COLOR2;
	float3 Texture			: TEXCOORD0;
	float3 LTexture			: TEXCOORD1;
	float4 WorldPosition	: TEXCOORD2;
#ifdef SHADOWMAPPING
	float4 TextureShadow	: TEXCOORD3;
#endif
	//uint uSampleIndex	: SV_SampleIndex;
};

float4 PS( PS_INPUT input ) : SV_TARGET
{
	float4 Color = g_TextureColor.Sample(g_samPoint, input.Texture);
	float4 Biome = g_TextureBiome.Sample(g_samPoint, input.Texture);
	Color.rgb = lerp(Color.rgb, Biome.rgb * input.Diffuse, Biome.a);

	float2 uv = input.Position.xy / g_vViewport.xy;
	float Shadow = g_shadowTexture.SampleLevel(g_samPoint, uv, 0).r;

#ifdef SHADOWMAPPING
	float4 vShadowMapTextureCoord = 0.0f;
	float4 vShadowMapTextureCoord_blend = 0.0f;

	float4 vVisualizeCascadeColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

	float fPercentLit = 0.0f;
	float fPercentLit_blend = 0.0f;

	float fUpTextDepthWeight = 0;
	float fRightTextDepthWeight = 0;
	float fUpTextDepthWeight_blend = 0;
	float fRightTextDepthWeight_blend = 0;

	int iBlurRowSize = g_iPCFBlurForLoopEnd - g_iPCFBlurForLoopStart;
	iBlurRowSize *= iBlurRowSize;
	float fBlurRowSize = (float)iBlurRowSize;

	int iCascadeFound = 0;
	int iNextCascadeIndex = 1;

	float fCurrentPixelDepth = input.WorldPosition.w;

	// The interval based selection technique compares the pixel's depth against the frustum's cascade divisions.

	// This for loop is not necessary when the frustum is uniformaly divided and interval based selection is used.
	// In this case fCurrentPixelDepth could be used as an array lookup into the correct frustum. 
	int iCurrentCascadeIndex;

	float4 vShadowMapTextureCoordViewSpace = input.TextureShadow;
		if (SELECT_CASCADE_BY_INTERVAL_FLAG)
		{
		iCurrentCascadeIndex = 0;
		if (CASCADE_COUNT_FLAG > 1)
		{
			float4 vCurrentPixelDepth = input.WorldPosition.w;
				float4 fComparison = (vCurrentPixelDepth > g_fCascadeFrustumsEyeSpaceDepthsFloat[0]);
				float4 fComparison2 = (vCurrentPixelDepth > g_fCascadeFrustumsEyeSpaceDepthsFloat[1]);
				float fIndex = dot(
				float4(CASCADE_COUNT_FLAG > 0,
				CASCADE_COUNT_FLAG > 1,
				CASCADE_COUNT_FLAG > 2,
				CASCADE_COUNT_FLAG > 3)
				, fComparison)
				+ dot(
				float4(
				CASCADE_COUNT_FLAG > 4,
				CASCADE_COUNT_FLAG > 5,
				CASCADE_COUNT_FLAG > 6,
				CASCADE_COUNT_FLAG > 7)
				, fComparison2);

			fIndex = min(fIndex, CASCADE_COUNT_FLAG - 1);
			iCurrentCascadeIndex = (int)fIndex;
		}
		}

	if (!SELECT_CASCADE_BY_INTERVAL_FLAG)
	{
		iCurrentCascadeIndex = 0;
		if (CASCADE_COUNT_FLAG == 1)
		{
			vShadowMapTextureCoord = vShadowMapTextureCoordViewSpace * g_vCascadeScale[0];
			vShadowMapTextureCoord += g_vCascadeOffset[0];
		}
		if (CASCADE_COUNT_FLAG > 1) {
			for (int iCascadeIndex = 0; iCascadeIndex < CASCADE_COUNT_FLAG && iCascadeFound == 0; ++iCascadeIndex)
			{
				vShadowMapTextureCoord = vShadowMapTextureCoordViewSpace * g_vCascadeScale[iCascadeIndex];
				vShadowMapTextureCoord += g_vCascadeOffset[iCascadeIndex];

				if (min(vShadowMapTextureCoord.x, vShadowMapTextureCoord.y) > g_fMinBorderPadding
					&& max(vShadowMapTextureCoord.x, vShadowMapTextureCoord.y) < g_fMaxBorderPadding)
				{
					iCurrentCascadeIndex = iCascadeIndex;
					iCascadeFound = 1;
				}
			}
		}
	}

	float4 color = 0;

		if (BLEND_BETWEEN_CASCADE_LAYERS_FLAG)
		{
		// Repeat text coord calculations for the next cascade. 
		// The next cascade index is used for blurring between maps.
		iNextCascadeIndex = min(CASCADE_COUNT_FLAG - 1, iCurrentCascadeIndex + 1);
		}

	float fBlendBetweenCascadesAmount = 1.0f;
	float fCurrentPixelsBlendBandLocation = 1.0f;

	if (SELECT_CASCADE_BY_INTERVAL_FLAG)
	{
		if (BLEND_BETWEEN_CASCADE_LAYERS_FLAG && CASCADE_COUNT_FLAG > 1)
		{
			CalculateBlendAmountForInterval(iCurrentCascadeIndex, fCurrentPixelDepth, fCurrentPixelsBlendBandLocation, fBlendBetweenCascadesAmount);
		}
	}
	else
	{

		if (BLEND_BETWEEN_CASCADE_LAYERS_FLAG)
		{
			CalculateBlendAmountForMap(vShadowMapTextureCoord, fCurrentPixelsBlendBandLocation, fBlendBetweenCascadesAmount);
		}
	}

	float3 vShadowMapTextureCoordDDX;
	float3 vShadowMapTextureCoordDDY;
	// The derivatives are used to find the slope of the current plane.
	// The derivative calculation has to be inside of the loop in order to prevent divergent flow control artifacts.
	if (USE_DERIVATIVES_FOR_DEPTH_OFFSET_FLAG)
	{
		vShadowMapTextureCoordDDX = ddx(vShadowMapTextureCoordViewSpace).xyz;
		vShadowMapTextureCoordDDY = ddy(vShadowMapTextureCoordViewSpace).xyz;

		vShadowMapTextureCoordDDX *= g_vCascadeScale[iCurrentCascadeIndex].xyz;
		vShadowMapTextureCoordDDY *= g_vCascadeScale[iCurrentCascadeIndex].xyz;
	}

	ComputeCoordinatesTransform(iCurrentCascadeIndex,
		float4(input.WorldPosition.xyz, 1.0f),
		vShadowMapTextureCoord,
		vShadowMapTextureCoordViewSpace);


	if (USE_DERIVATIVES_FOR_DEPTH_OFFSET_FLAG)
	{
		CalculateRightAndUpTexelDepthDeltas(vShadowMapTextureCoordDDX, vShadowMapTextureCoordDDY, fUpTextDepthWeight, fRightTextDepthWeight);
	}

	CalculatePCFPercentLit(vShadowMapTextureCoord, fRightTextDepthWeight,
		fUpTextDepthWeight, fBlurRowSize, fPercentLit);

	if (BLEND_BETWEEN_CASCADE_LAYERS_FLAG && CASCADE_COUNT_FLAG > 1)
	{
		if (fCurrentPixelsBlendBandLocation < g_fCascadeBlendArea)
		{  // the current pixel is within the blend band.

			// Repeat text coord calculations for the next cascade. 
			// The next cascade index is used for blurring between maps.
			if (!SELECT_CASCADE_BY_INTERVAL_FLAG)
			{
				vShadowMapTextureCoord_blend = vShadowMapTextureCoordViewSpace * g_vCascadeScale[iNextCascadeIndex];
				vShadowMapTextureCoord_blend += g_vCascadeOffset[iNextCascadeIndex];
			}

			ComputeCoordinatesTransform(iNextCascadeIndex, float4(input.WorldPosition.xyz, 1.0f), vShadowMapTextureCoord_blend, vShadowMapTextureCoordViewSpace);

			// We repeat the calcuation for the next cascade layer, when blending between maps.
			if (fCurrentPixelsBlendBandLocation < g_fCascadeBlendArea)
			{  // the current pixel is within the blend band.
				if (USE_DERIVATIVES_FOR_DEPTH_OFFSET_FLAG)
				{

					CalculateRightAndUpTexelDepthDeltas(vShadowMapTextureCoordDDX,
						vShadowMapTextureCoordDDY,
						fUpTextDepthWeight_blend,
						fRightTextDepthWeight_blend);
				}
				CalculatePCFPercentLit(vShadowMapTextureCoord_blend, fRightTextDepthWeight_blend, fUpTextDepthWeight_blend, fBlurRowSize, fPercentLit_blend);
				fPercentLit = lerp(fPercentLit_blend, fPercentLit, fBlendBetweenCascadesAmount);
				// Blend the two calculated shadows by the blend amount.
			}
		}
	}
	float4 finalColour = (float4(input.MieColor.rgb, 1.0f) * max(min(input.MieColor.a, fPercentLit), 0.25f));
#else
	float4 finalColour = (float4(input.MieColor.rgb, 1.0f) * max(min(input.MieColor.a, Shadow), 0.25f));
#endif

	float4 Light = g_TextureLight.Sample(g_samBilinear, input.LTexture);
	finalColour *= Color;
	finalColour *= Light.a;
	finalColour += input.RayleighColor;

	return finalColour;
}