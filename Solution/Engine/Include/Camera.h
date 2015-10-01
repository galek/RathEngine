//--------------------------------------------------------------------------------------
// File: Camera.h
//
// Helper functions for Direct3D programming.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// http://go.microsoft.com/fwlink/?LinkId=320437
//--------------------------------------------------------------------------------------
#pragma once

#include "Node.h"
namespace Rath
{
	class Camera : public Node
	{
	protected:
		//Matrix		m_mView;						// View matrix 
		DirectX::SimpleMath::Matrix	m_mProj;						// Projection matrix

		float						m_fFOV;                         // Field of view
		float						m_fAspect;                      // Aspect ratio
		float						m_fNearPlane;                   // Near plane
		float						m_fFarPlane;                    // Far plane
	public:
		Camera();

		void virtual XM_CALLCONV SetPosition(_In_ XMVECTOR position);
		void virtual XM_CALLCONV SetWorld(_In_ XMMATRIX world);

		// Functions to change camera matrices
		void XM_CALLCONV SetViewParams(_In_ FXMVECTOR vEyePt, _In_ FXMVECTOR vLookatPt, _In_opt_ FXMVECTOR vWorldUp = g_XMIdentityR1);
		void XM_CALLCONV SetViewParams(_In_ FXMVECTOR vDirection);
		void			 SetProjParams(_In_ float fFOV, _In_ float fAspect, _In_ float fNearPlane, _In_ float fFarPlane);
		void XM_CALLCONV SetCameraView(_In_ XMMATRIX mView);
		void XM_CALLCONV SetCameraProjection(_In_ XMMATRIX mProjection);
		XMMATRIX     XM_CALLCONV GetCameraView() const;
		XMMATRIX     XM_CALLCONV GetCameraProjection() const;
		XMMATRIX     XM_CALLCONV GetCameraViewProjection() const;
		XMVECTOR	 XM_CALLCONV GetLookAtPoint() const;
		XMFRUSTUM    XM_CALLCONV GetCameraFrustum() const;
		float					 GetNearClip() const { return m_fNearPlane; };
		float					 GetFarClip() const { return m_fFarPlane; };
		float virtual			 GetOptNearClip() const { return m_fNearPlane; };
		float virtual			 GetOptFarClip() const { return m_fFarPlane; };
	};

	class OrthographicCamera : public Camera	
	{
	protected:
		float m_fxMin;
		float m_fxMax;
		float m_fyMin;
		float m_fyMax;

		virtual void CreateProjection();
	public:
		OrthographicCamera(float minX, float minY, float maxX, float maxY, float nearClip, float farClip);

		void  SetProjParams(_In_ float minX, _In_ float minY, _In_ float maxX, _In_ float maxY, _In_ float fNearPlane, _In_ float fFarPlane);

		float MinX() const { return m_fxMin; };
		float MinY() const { return m_fyMin; };
		float MaxX() const { return m_fxMax; };
		float MaxY() const { return m_fyMax; };

		void SetMinX(float minX);
		void SetMinY(float minY);
		void SetMaxX(float maxX);
		void SetMaxY(float maxY);
	};

	class OptimizedCamera : public Camera
	{
	protected:
		DirectX::SimpleMath::Matrix	m_mOptProj;						// Projection matrix
		float						m_fOptNearPlane;                // Near plane
		float						m_fOptFarPlane;                 // Far plane
	public:
		void Optimize(const DirectX::SimpleMath::Vector2 & reducedDepth);

		XMMATRIX    XM_CALLCONV GetOptCameraProjection() const;
		XMMATRIX    XM_CALLCONV GetOptCameraViewProjection() const;
		XMFRUSTUM	XM_CALLCONV GetOptCameraFrustum() const;
		float		GetOptNearClip() const { return m_fOptNearPlane; };
		float		GetOptFarClip() const { return m_fOptFarPlane; };
	};
}