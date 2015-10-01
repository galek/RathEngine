#include "pch.h"
#include "Camera.h"

namespace Rath
{
	Camera::Camera()
	{
		// Setup the view matrix
		SetViewParams(g_XMZero, g_XMIdentityR2);

		// Setup the projection matrix
		SetProjParams(XM_PI / 4, 1.0f, 0.1f, 200.0f);
	}

	void XM_CALLCONV Camera::SetPosition(_In_ XMVECTOR position)
	{
		Node::SetPosition(position);
		//m_mView = XMMatrixInverse(nullptr, GetWorld());
	}

	void XM_CALLCONV Camera::SetWorld(_In_ XMMATRIX world)
	{
		Node::SetWorld(world);
		//m_mView = XMMatrixInverse(nullptr, GetWorld());
	}

	//--------------------------------------------------------------------------------------
	// Client can call this to change the position and direction of camera
	//--------------------------------------------------------------------------------------
	_Use_decl_annotations_
		void XM_CALLCONV Camera::SetViewParams(FXMVECTOR vEyePt, FXMVECTOR vLookatPt, FXMVECTOR vWorldUp)
	{
		// Calc the view matrix
		XMMATRIX view = XMMatrixLookAtLH(vEyePt, vLookatPt, vWorldUp);
		//m_mView = view;

		XMMATRIX mInvView = XMMatrixInverse(nullptr, view);
		SetWorld(mInvView);

		//// The axis basis vectors and camera position are stored inside the 
		//// position matrix in the 4 rows of the camera's world matrix.
		//// To figure out the yaw/pitch of the camera, we just need the Z basis vector
		//XMFLOAT3 zBasis;
		//XMStoreFloat3(&zBasis, mInvView.r[2]);

		//m_fCameraYawAngle = atan2f(zBasis.x, zBasis.z);
		//float fLen = sqrtf(zBasis.z * zBasis.z + zBasis.x * zBasis.x);
		//m_fCameraPitchAngle = -atan2f(zBasis.y, fLen);
	}

	_Use_decl_annotations_
		void XM_CALLCONV Camera::SetViewParams(_In_ FXMVECTOR vDirection)
	{
		XMMATRIX view = XMMatrixIdentity();
		view.r[2] = XMVectorSetW(-XMVector3Normalize(vDirection), 0.0f);
		view.r[3] = XMVectorSetW(XMVector3Normalize(vDirection), 1.0f);
		//m_mView = view;
	}
	_Use_decl_annotations_
		void XM_CALLCONV Camera::SetCameraView(_In_ XMMATRIX mView)
	{
		//m_mView = mView;
		XMMATRIX mInvView = XMMatrixInverse(nullptr, mView);
		SetWorld(mInvView);
	}
	_Use_decl_annotations_
		void XM_CALLCONV Camera::SetCameraProjection(_In_ XMMATRIX mProjection)
	{
		m_mProj = mProjection;
	}

	//--------------------------------------------------------------------------------------
	// Calculates the projection matrix based on input params
	//--------------------------------------------------------------------------------------
	_Use_decl_annotations_
		void Camera::SetProjParams(float fFOV, float fAspect, float fNearPlane, float fFarPlane)
	{
		// Set attributes for the projection matrix
		m_fFOV = fFOV;
		m_fAspect = fAspect;
		m_fNearPlane = fNearPlane;
		m_fFarPlane = fFarPlane;

		m_mProj = XMMatrixPerspectiveFovLH(fFOV, fAspect, fNearPlane, fFarPlane);
	}

	XMMATRIX XM_CALLCONV Camera::GetCameraView() const
	{
		return XMMatrixInverse(nullptr, GetWorld());
	}

	XMMATRIX XM_CALLCONV Camera::GetCameraProjection() const
	{
		return m_mProj;
	}

	XMMATRIX XM_CALLCONV Camera::GetCameraViewProjection() const
	{
		return XMMatrixMultiply(GetCameraView(), m_mProj);
	}

	XMVECTOR XM_CALLCONV Camera::GetLookAtPoint() const
	{
		XMMATRIX InvView = GetWorld();// XMMatrixInverse(nullptr, m_mView);
		XMVECTOR Direction = XMVector3Normalize(InvView.r[3]);

		return GetPosition() + Direction;
	}

	XMFRUSTUM XM_CALLCONV Camera::GetCameraFrustum() const
	{
		return XMFRUSTUM(GetCameraViewProjection());
	}

	//=================================================================================================
	// OrthographicCamera
	//=================================================================================================

	OrthographicCamera::OrthographicCamera(float minX, float minY, float maxX,
		float maxY, float nearClip, float farClip) : Camera(),
		m_fxMin(minX),
		m_fyMin(minY),
		m_fxMax(maxX),
		m_fyMax(maxY)
	{
		Assert_(maxX > minX && maxY > minY);

		m_fNearPlane = nearClip;
		m_fFarPlane = farClip;

		CreateProjection();
	}

	void OrthographicCamera::SetProjParams(_In_ float minX, _In_ float minY, _In_ float maxX, _In_ float maxY, _In_ float fNearPlane, _In_ float fFarPlane)
	{
		Assert_(maxX > minX && maxY > minY);

		m_fxMin = minX;
		m_fyMin = minY;
		m_fxMax = maxX;
		m_fyMax = maxY;	
		m_fNearPlane = fNearPlane;
		m_fFarPlane = fFarPlane;

		m_mProj = XMMatrixOrthographicOffCenterLH(m_fxMin, m_fxMax, m_fyMin, m_fyMax, m_fNearPlane, m_fFarPlane);
	}

	void OrthographicCamera::CreateProjection()
	{
		m_mProj = XMMatrixOrthographicOffCenterLH(m_fxMin, m_fxMax, m_fyMin, m_fyMax, m_fNearPlane, m_fFarPlane);
		//viewProjection = m_mProj * m_mProj;
	}

	void OrthographicCamera::SetMinX(float minX)
	{
		m_fxMin = minX;
		CreateProjection();
	}

	void OrthographicCamera::SetMinY(float minY)
	{
		m_fyMin = minY;
		CreateProjection();
	}

	void OrthographicCamera::SetMaxX(float maxX)
	{
		m_fxMax = maxX;
		CreateProjection();
	}

	void OrthographicCamera::SetMaxY(float maxY)
	{
		m_fyMax = maxY;
		CreateProjection();
	}

	//=================================================================================================
	// OptimizedCamera
	//=================================================================================================

	void OptimizedCamera::Optimize(const DirectX::SimpleMath::Vector2 & reducedDepth)
	{
		m_fOptNearPlane = m_fFarPlane * reducedDepth.x + m_fNearPlane;
		m_fOptFarPlane = m_fFarPlane * reducedDepth.y;
		m_mOptProj = XMMatrixPerspectiveFovLH(m_fFOV, m_fAspect, m_fOptNearPlane, m_fOptFarPlane);
	}

	XMMATRIX XM_CALLCONV OptimizedCamera::GetOptCameraProjection() const
	{
		return m_mOptProj;
	}

	XMMATRIX XM_CALLCONV OptimizedCamera::GetOptCameraViewProjection() const
	{
		return XMMatrixMultiply(GetCameraView(), m_mOptProj);
	}

	XMFRUSTUM XM_CALLCONV OptimizedCamera::GetOptCameraFrustum() const
	{
		return XMFRUSTUM(GetOptCameraViewProjection());
	}
}