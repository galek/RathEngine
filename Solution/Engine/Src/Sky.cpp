#include "pch.h"
#include "Sky.h"
#include "Clouds.h"
#include "AssetLibrary.h"

namespace Rath
{
	void Sky::GLOBAL_SKY_CB_STRUCT::CalculateLight(const XMMATRIX&  mViewProjection, const XMVECTOR& vLightDirection, const XMVECTOR& vViewDirection)
	{
		static const float fRayMult = 0.002f;
		static const float fMieMult = 0.0015f;

		v3LightDir = vLightDirection;
		XMVECTOR Light = XMVector4Transform(vLightDirection, mViewProjection);
		XMVECTOR Dot = XMVectorClamp(XMVector3Dot(vLightDirection, vViewDirection) * 10.0f, g_XMZero, g_XMOne);
		Light /= XMVectorGetW(Light);
		v2LightScreenPosition = XMVectorSet(((XMVectorGetX(Light) + 1.f) / 2.f), (1.f - (XMVectorGetY(Light) + 1.f) / 2.f), XMVectorGetZ(Light), XMVectorGetX(Dot));

		// Scattering parameters
		KrESun = fRayMult * 40.f;
		KmESun = fMieMult * 40.f;
		Kr4PI = fRayMult * XM_PI * 4.f;
		Km4PI = fMieMult * XM_PI * 4.f;
		v3InvWavelength = XMVectorSet(1.f / powf(0.650f, 4.f), 1.f / powf(0.570f, 4.f), 1.f / powf(0.475f, 4.f), 1.f);
		v3InvAttenuate = XMVectorMultiplyAdd(v3InvWavelength, XMVectorReplicate(Kr4PI), XMVectorReplicate(Km4PI));

		// Phase function
		g = 0.991f;
		g2 = g * g;

		// Size parameters
		fCameraHeight = 10.01f;
		fInnerRadius = 10.000f;
		fOuterRadius = 10.25f;
		fScale = 1.f / (fOuterRadius - fInnerRadius) * 1.2f;
		fScaleDepth = 0.25f;
		fScaleOverScaleDepth = fScale / fScaleDepth;
		fSkydomeRadius = 1.0f;
		fDepth = exp((fInnerRadius - fCameraHeight) / fScaleDepth);
		v3CameraPos = XMVectorSet(0.f, fCameraHeight, 0.f, 1.f);
	}

	Sky::Sky(Camera* pCamera, DirectX::SimpleMath::Vector3* pLightDirection) :
		m_pCamera(pCamera),
		m_pLightDirection(pLightDirection)
	{
		mSkyTechnique = AssetLibrary::GetTechnique("SkyShader");

		Assert_(m_pCamera != nullptr);
		Assert_(m_pLightDirection != nullptr);
		Assert_(mSkyTechnique != nullptr);

		mClouds = new Clouds();
	}

	Sky::~Sky()
	{
		delete mClouds;
	}

	void Sky::generateSkyDome(ID3D11Device* device)
	{
		std::vector<XMFLOAT3> vbl;
		std::vector<UINT>	  ibl;

		unsigned int mNumRings = 20;
		unsigned int mNumRingsLow = 11;
		unsigned int mNumSegments = 20;

		vbl.reserve((mNumRings + 1)*(mNumSegments + 1));
		ibl.reserve(mNumRings*(mNumSegments + 1) * 6);

		float fDeltaRingAngle = (XM_PI / mNumRings);
		float fDeltaSegAngle = (XM_2PI / mNumSegments);
		unsigned int offset = 0;

		// Generate the group of rings for the sphere
		for (unsigned int ring = 0; ring <= mNumRingsLow; ring++) {
			float r0 = sinf(ring * fDeltaRingAngle);
			float y0 = cosf(ring * fDeltaRingAngle);

			// Generate the group of segments for the current ring
			for (unsigned int seg = 0; seg <= mNumSegments; seg++) {
				float x0 = r0 * sinf(seg * fDeltaSegAngle);
				float z0 = r0 * cosf(seg * fDeltaSegAngle);

				vbl.push_back(XMFLOAT3(x0, y0, z0));
				// Add one vertex to the strip which makes up the sphere
				/*	addPoint(buffer, Vector3(x0, y0, z0),
				Vector3(x0, y0, z0).normalisedCopy(),
				Vector2(float)seg / (Real)mNumSegments, (float)ring / (Real)mNumRings));*/

				if (ring != mNumRings) {
					// each vertex (except the last) has six indices pointing to it
					ibl.push_back(offset + mNumSegments + 1);
					ibl.push_back(offset + mNumSegments);
					ibl.push_back(offset);

					ibl.push_back(offset + mNumSegments + 1);
					ibl.push_back(offset);
					ibl.push_back(offset + 1);

					offset++;
				}
			}; // end for seg
		} // end for ring

		mMesh = new Mesh(device, sizeof(MINIMUM_MESH),
			sizeof(MINIMUM_MESH) * (UINT)vbl.size(), &vbl[0],
			sizeof(UINT) * (UINT)ibl.size(), &ibl[0],
			0);
	}

	void Sky::CreateDevice(ID3D11Device* device)
	{
		generateSkyDome(device);

		mConstantBuffer.Initialize(device);

		mClouds->CreateDevice(device);
	}

	void Sky::Update(float fElapsedTime)
	{
		//XMVECTOR lookAt = pState->mCamera.GetPosition();
		//XMVECTOR eyePt = XMVectorMultiplyAdd(pState->mLightDirection, XMVectorReplicate(100.0f), lookAt);
		//pState->mLight.SetViewParams(eyePt, lookAt);
		//pState->mLight.FrameMove(fElapsedTime);

		XMMATRIX ViewMatrix = m_pCamera->GetCameraView();
		XMMATRIX ProjMatrix = m_pCamera->GetCameraProjection();
		XMMATRIX ViewProjMatrix = ViewMatrix * ProjMatrix;
		XMVECTOR LightDirection = *m_pLightDirection;
		XMVECTOR ViewDirection = m_pCamera->GetDirection();
		mConstantBuffer.Data.CalculateLight(ViewProjMatrix, LightDirection, ViewDirection);

		mClouds->Update(fElapsedTime);
	}

	void Sky::Render(ID3D11DeviceContext* context)
	{
		mConstantBuffer.ApplyChanges(context);
		mConstantBuffer.SetVS(context, 2);
		mConstantBuffer.SetHS(context, 2);
		mConstantBuffer.SetDS(context, 2);
		mConstantBuffer.SetPS(context, 2);

		mSkyTechnique->Apply(context);
		mMesh->Render(context);

		context->HSSetShader(nullptr, nullptr, 0);
		context->DSSetShader(nullptr, nullptr, 0);
		context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		mClouds->Render(context);
	}
}