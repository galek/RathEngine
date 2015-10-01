//--------------------------------------------------------------------------------------
// File: shader_include.hlsl
//
// Include file for common shader definitions and functions.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Defines
//--------------------------------------------------------------------------------------                  
#define ADD_SPECULAR 0

//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------
Texture2D			g_baseTexture	: register(t0);   // Base color texture
Texture2D			g_nmhTexture	: register(t1);   // Normal map and height map texture pair
Texture2D			g_shadowTexture : register(t5);
Texture2DMS<float>	g_shadowTextureMS : register(t5);
//--------------------------------------------------------------------------------------
// Buffer
//--------------------------------------------------------------------------------------
Buffer<float4> 	g_bwmBuffer 	: register (t0);   // Bone matrix buffer
//--------------------------------------------------------------------------------------
// Samplers
//--------------------------------------------------------------------------------------
SamplerState	g_samPoint		: register(s0);
SamplerState	g_samLinear		: register(s1);
SamplerState	g_samAnisotropic: register(s2);
SamplerState	g_samBilinear	: register(s3);
SamplerState	g_samLinearWrap	: register(s4);

//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------
shared cbuffer cbGlobal : register(b0)
{
	matrix	g_mView;                             // View matrix
	matrix	g_mProjection;                       // Projection matrix
	matrix	g_mViewProjection;                   // VP matrix
	matrix	g_mInvView;                          // Inverse of view matrix
	matrix	g_mCenterViewProjection;			 // Centered VP matrix
	
	float4 	g_vEye;					    		 // Camera's location	
	float4  g_ViewDirection; 
	float4  g_LightDiffuse;                  	 // Light's diffuse color
	float4 	g_LightAmbient;                  	 // Light's ambient color	

	float4  g_vViewport;
	float4  g_vFrustumPlaneEquation[4];			 // View frustum planes ( x=left, y=right, z=top, w=bottom )
	float4  g_vCenterFrustumPlaneEquation[4];	 // View frustum planes ( x=left, y=right, z=top, w=bottom )
	float4  g_vTessellationFactor;               // Edge, inside, minimum tessellation factor and 1/desired triangle size
};

shared cbuffer cbMaterial : register(b1)
{
	matrix 	g_mWorld;                           // World matrix
	
	float4 	g_MaterialAmbientColor;          	// Material's ambient color
	float4 	g_MaterialDiffuseColor;          	// Material's diffuse color
	float4 	g_MaterialSpecularColor;         	// Material's specular color
	float4  g_fSpecularExponent;             	// Material's specular exponent
};

shared cbuffer cbSky : register(b2)
{
	float4 g_LightDirection;		// Light direction
	float4 g_LightScreenPosition;
	float4 g_CameraPos;				// Camera's current position
	float4 g_InvWavelength;			// 1 / pow(wavelength, 4) for RGB channels
	float4 g_InvAttenuate;

	float  g_fCameraHeight;
	float  g_fInnerRadius;
	float  g_fOuterRadius;

	// Scattering parameters
	float  g_KrESun;		// Kr * ESun
	float  g_KmESun;		// Km * ESun
	float  g_Kr4PI;			// Kr * 4 * PI
	float  g_Km4PI;			// Km * 4 * PI

	// Phase function
	float  g_g;
	float  g_g2;

	float  g_fScale;			// 1 / (outerRadius - innerRadius) = 4 here
	float  g_fScaleDepth;		// Where the average atmosphere density is found
	float  g_fScaleOverScaleDepth;	// scale / scaleDepth
	float  g_fDepth;
	float  g_fSkydomeRadius;	// Skydome radius (allows us to normalize skydome distances etc)
};

shared cbuffer cbShadow : register(b3)
{
	matrix	g_mShadow;
	float4	g_vCascadeOffset[8];
	float4	g_vCascadeScale[8];
	float4  g_fCascadeFrustumsEyeSpaceDepthsFloat[2];  // The values along Z that seperate the cascades.
	float4  g_fCascadeFrustumsEyeSpaceDepthsFloat4[8];  // the values along Z that separte the cascades.  

	int     g_nCascadeLevels;
	int     g_iPCFBlurForLoopStart; // For loop begin value. For a 5x5 Kernal this would be -2.
	int     g_iPCFBlurForLoopEnd; // For loop end value. For a 5x5 kernel this would be 3.

	// For Map based selection scheme, this keeps the pixels inside of the the valid range.
	// When there is no boarder, these values are 0 and 1 respectivley.
	float           g_fMinBorderPadding;
	float           g_fMaxBorderPadding;
	float           g_fShadowBiasFromGUI;  // A shadow map offset to deal with self shadow artifacts.  
	//These artifacts are aggravated by PCF.
	float           g_fShadowPartitionSize;

	float           g_fCascadeBlendArea; // Amount to overlap when blending between cascades.
	float           g_fTexelSize;
	float           g_fNativeTexelSizeInX;

	int		padding[2];
};

float scale(float cos)
{
	float x = 1.0 - cos;
	return g_fScaleDepth * exp(-0.00287f + x*(0.459f + x*(3.83f + x*(-6.80f + x*5.25f))));
};

//--------------------------------------------------------------------------------------
// Returns the dot product between the viewing vector and the patch edge
//--------------------------------------------------------------------------------------
float GetEdgeDotProduct(
	float3 f3EdgeNormal0,   // Normalized normal of the first control point of the given patch edge 
	float3 f3EdgeNormal1,   // Normalized normal of the second control point of the given patch edge 
	float3 f3ViewVector     // Normalized viewing vector
	)
{
	float3 f3EdgeNormal = normalize((f3EdgeNormal0 + f3EdgeNormal1) * 0.5f);

	float fEdgeDotProduct = dot(f3EdgeNormal, f3ViewVector);

	return fEdgeDotProduct;
}


//--------------------------------------------------------------------------------------
// Returns the screen space position from the given world space patch control point
//--------------------------------------------------------------------------------------
float2 GetScreenSpacePosition(
	float3 f3Position,              // World space position of patch control point
	float4x4 f4x4ViewProjection,    // View x Projection matrix
	float fScreenWidth,             // Screen width
	float fScreenHeight             // Screen height
	)
{
	float4 f4ProjectedPosition = mul(float4(f3Position, 1.0f), f4x4ViewProjection);

	float2 f2ScreenPosition = f4ProjectedPosition.xy / f4ProjectedPosition.ww;

	f2ScreenPosition = (f2ScreenPosition + 1.0f) * 0.5f * float2(fScreenWidth, -fScreenHeight);

	return f2ScreenPosition;
}


//--------------------------------------------------------------------------------------
// Returns the distance of a given point from a given plane
//--------------------------------------------------------------------------------------
float DistanceFromPlane(
	float3 f3Position,      // World space position of the patch control point
	float4 f4PlaneEquation  // Plane equation of a frustum plane
	)
{
	float fDistance = dot(float4(f3Position, 1.0f), f4PlaneEquation);

	return fDistance;
}


//--------------------------------------------------------------------------------------
// Returns a distance adaptive tessellation scale factor (0.0f -> 1.0f) 
//--------------------------------------------------------------------------------------
float GetDistanceAdaptiveScaleFactor(
	float3 f3Eye,           // Position of the camera/eye
	float3 f3EdgePosition0, // Position of the first control point of the given patch edge
	float3 f3EdgePosition1, // Position of the second control point of the given patch edge
	float fMinDistance,     // Minimum distance that maximum tessellation factors should be applied at
	float fRange            // Range beyond the minimum distance where tessellation will scale down to the minimum scaling factor    
	)
{
	float3 f3MidPoint = (f3EdgePosition0 + f3EdgePosition1) * 0.5f;

	float fDistance = distance(f3MidPoint, f3Eye) - fMinDistance;

	float fScale = 1.0f - saturate(fDistance / fRange);

	return fScale;
}

float GetDistanceAdaptiveScaleFactor(
	float3 f3Eye,           // Position of the camera/eye
	float3 f3EdgePosition, // Position of the first control point of the given patch edge
	float fMinDistance,     // Minimum distance that maximum tessellation factors should be applied at
	float fRange            // Range beyond the minimum distance where tessellation will scale down to the minimum scaling factor    
	)
{
	float fDistance = distance(f3EdgePosition, f3Eye) - fMinDistance;

	float fScale = 1.0f - saturate(fDistance / fRange);

	return fScale;
}


//--------------------------------------------------------------------------------------
// Returns the orientation adaptive tessellation factor (0.0f -> 1.0f)
//--------------------------------------------------------------------------------------
float GetOrientationAdaptiveScaleFactor(
	float fEdgeDotProduct,      // Dot product of edge normal with view vector
	float fSilhouetteEpsilon    // Epsilon to determine the range of values considered to be silhoutte
	)
{
	float fScale = 1.0f - abs(fEdgeDotProduct);

	fScale = saturate((fScale - fSilhouetteEpsilon) / (1.0f - fSilhouetteEpsilon));

	return fScale;
}


//--------------------------------------------------------------------------------------
// Returns the screen resolution adaptive tessellation scale factor (0.0f -> 1.0f)
//--------------------------------------------------------------------------------------
float GetScreenResolutionAdaptiveScaleFactor(
	float fCurrentWidth,    // Current render window width 
	float fCurrentHeight,   // Current render window height 
	float fMaxWidth,        // Width considered to be max
	float fMaxHeight        // Height considered to be max
	)
{
	float fMaxArea = fMaxWidth * fMaxHeight;

	float fCurrentArea = fCurrentWidth * fCurrentHeight;

	float fScale = saturate(fCurrentArea / fMaxArea);

	return fScale;
}


//--------------------------------------------------------------------------------------
// Returns the screen space adaptive tessellation scale factor (0.0f -> 1.0f)
//--------------------------------------------------------------------------------------
float GetScreenSpaceAdaptiveScaleFactor(
	float2 f2EdgeScreenPosition0,   // Screen coordinate of the first patch edge control point
	float2 f2EdgeScreenPosition1,   // Screen coordinate of the second patch edge control point    
	float fMaxEdgeTessFactor,       // Maximum edge tessellation factor                            
	float fTargetEdgePrimitiveSize  // Desired primitive edge size in pixels
	)
{
	float fEdgeScreenLength = distance(f2EdgeScreenPosition0, f2EdgeScreenPosition1);

	float fTargetTessFactor = fEdgeScreenLength / fTargetEdgePrimitiveSize;

	fTargetTessFactor /= fMaxEdgeTessFactor;

	float fScale = saturate(fTargetTessFactor);

	return fScale;
}


//--------------------------------------------------------------------------------------
// Returns back face culling test result (true / false)
//--------------------------------------------------------------------------------------
bool BackFaceCull(
	float fEdgeDotProduct0, // Dot product of edge 0 normal with view vector
	float fEdgeDotProduct1, // Dot product of edge 1 normal with view vector
	float fEdgeDotProduct2, // Dot product of edge 2 normal with view vector
	float fBackFaceEpsilon  // Epsilon to determine cut off value for what is considered back facing
	)
{
	float3 f3BackFaceCull;

	f3BackFaceCull.x = (fEdgeDotProduct0 > -fBackFaceEpsilon) ? (0.0f) : (1.0f);
	f3BackFaceCull.y = (fEdgeDotProduct1 > -fBackFaceEpsilon) ? (0.0f) : (1.0f);
	f3BackFaceCull.z = (fEdgeDotProduct2 > -fBackFaceEpsilon) ? (0.0f) : (1.0f);

	return all(f3BackFaceCull);
}


//--------------------------------------------------------------------------------------
// Returns view frustum Culling test result (true / false)
//--------------------------------------------------------------------------------------
bool ViewFrustumCull(
	float3 f3EdgePosition0,         // World space position of patch control point 0
	float3 f3EdgePosition1,         // World space position of patch control point 1
	float3 f3EdgePosition2,         // World space position of patch control point 2
	float4 f4ViewFrustumPlanes[4],  // 4 plane equations (left, right, top, bottom)
	float fCullEpsilon              // Epsilon to determine the distance outside the view frustum is still considered inside
	)
{
	float4 f4PlaneTest;
	float fPlaneTest;

	// Left clip plane
	f4PlaneTest.x = ((DistanceFromPlane(f3EdgePosition0, f4ViewFrustumPlanes[0]) > -fCullEpsilon) ? 1.0f : 0.0f) +
		((DistanceFromPlane(f3EdgePosition1, f4ViewFrustumPlanes[0]) > -fCullEpsilon) ? 1.0f : 0.0f) +
		((DistanceFromPlane(f3EdgePosition2, f4ViewFrustumPlanes[0]) > -fCullEpsilon) ? 1.0f : 0.0f);
	// Right clip plane
	f4PlaneTest.y = ((DistanceFromPlane(f3EdgePosition0, f4ViewFrustumPlanes[1]) > -fCullEpsilon) ? 1.0f : 0.0f) +
		((DistanceFromPlane(f3EdgePosition1, f4ViewFrustumPlanes[1]) > -fCullEpsilon) ? 1.0f : 0.0f) +
		((DistanceFromPlane(f3EdgePosition2, f4ViewFrustumPlanes[1]) > -fCullEpsilon) ? 1.0f : 0.0f);
	// Top clip plane
	f4PlaneTest.z = ((DistanceFromPlane(f3EdgePosition0, f4ViewFrustumPlanes[2]) > -fCullEpsilon) ? 1.0f : 0.0f) +
		((DistanceFromPlane(f3EdgePosition1, f4ViewFrustumPlanes[2]) > -fCullEpsilon) ? 1.0f : 0.0f) +
		((DistanceFromPlane(f3EdgePosition2, f4ViewFrustumPlanes[2]) > -fCullEpsilon) ? 1.0f : 0.0f);
	// Bottom clip plane
	f4PlaneTest.w = ((DistanceFromPlane(f3EdgePosition0, f4ViewFrustumPlanes[3]) > -fCullEpsilon) ? 1.0f : 0.0f) +
		((DistanceFromPlane(f3EdgePosition1, f4ViewFrustumPlanes[3]) > -fCullEpsilon) ? 1.0f : 0.0f) +
		((DistanceFromPlane(f3EdgePosition2, f4ViewFrustumPlanes[3]) > -fCullEpsilon) ? 1.0f : 0.0f);

	// Triangle has to pass all 4 plane tests to be visible
	return !all(f4PlaneTest);
}

bool ViewFrustumCull(
	float3 f3EdgePosition0,         // World space position of patch control point 0
	float4 f4ViewFrustumPlanes[4],  // 4 plane equations (left, right, top, bottom)
	float fCullEpsilon              // Epsilon to determine the distance outside the view frustum is still considered inside
	)
{
	float4 f4PlaneTest;
	float fPlaneTest;

	// Left clip plane
	f4PlaneTest.x = ((DistanceFromPlane(f3EdgePosition0, f4ViewFrustumPlanes[0]) > -fCullEpsilon) ? 1.0f : 0.0f);
	// Right clip plane
	f4PlaneTest.y = ((DistanceFromPlane(f3EdgePosition0, f4ViewFrustumPlanes[1]) > -fCullEpsilon) ? 1.0f : 0.0f);
	// Top clip plane
	f4PlaneTest.z = ((DistanceFromPlane(f3EdgePosition0, f4ViewFrustumPlanes[2]) > -fCullEpsilon) ? 1.0f : 0.0f);
	// Bottom clip plane
	f4PlaneTest.w = ((DistanceFromPlane(f3EdgePosition0, f4ViewFrustumPlanes[3]) > -fCullEpsilon) ? 1.0f : 0.0f);

	// Triangle has to pass all 4 plane tests to be visible
	return !all(f4PlaneTest);
}