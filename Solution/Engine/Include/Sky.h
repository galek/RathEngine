#pragma once
#include "Mesh.h"
#include "Technique.h"
#include "GraphicTypes.h"
#include "Camera.h"

namespace Rath
{
	class Clouds;
	class Sky
	{
	protected:
		struct GLOBAL_SKY_CB_STRUCT
		{
			DirectX::SimpleMath::Vector4 v3LightDir;		// Light direction
			DirectX::SimpleMath::Vector4 v2LightScreenPosition;
			DirectX::SimpleMath::Vector4 v3CameraPos;		// Camera's current position
			DirectX::SimpleMath::Vector4 v3InvWavelength;	// 1 / pow(wavelength, 4) for RGB channels
			DirectX::SimpleMath::Vector4 v3InvAttenuate;

			float  fCameraHeight;
			float  fInnerRadius;
			float  fOuterRadius;

			// Scattering parameters
			float  KrESun;		// Kr * ESun
			float  KmESun;		// Km * ESun
			float  Kr4PI;			// Kr * 4 * PI
			float  Km4PI;			// Km * 4 * PI

			// Phase function
			float  g;
			float  g2;

			float  fScale;			// 1 / (outerRadius - innerRadius) = 4 here
			float  fScaleDepth;		// Where the average atmosphere density is found
			float  fScaleOverScaleDepth;	// scale / scaleDepth
			float  fDepth;
			float  fSkydomeRadius;	// Skydome radius (allows us to normalize skydome distances etc)

			void CalculateLight(const XMMATRIX&  mViewProjection, const XMVECTOR& vLightDirection, const XMVECTOR& vViewDirection);
		};
		ConstantBuffer<GLOBAL_SKY_CB_STRUCT>  mConstantBuffer;
	protected:
		Clouds*			mClouds;

		MeshPtr			mMesh;
		TechniquePtr	mSkyTechnique;

		const Camera*						m_pCamera;
		const DirectX::SimpleMath::Vector3* m_pLightDirection;

		void generateSkyDome(ID3D11Device* device);
	public:
		Sky(Camera* pCamera, DirectX::SimpleMath::Vector3* pLightDirection);
		~Sky();

		void CreateDevice(ID3D11Device* device);
		void Update(float fElapsedTime);
		void Render(ID3D11DeviceContext* context);
	};
}