#include "pch.h"
#include "NodeController.h"

namespace Rath
{
	NodeController::NodeController(Node* pNode) :
		m_nMouseWheelDelta(0),
		m_fFramesToSmoothMouseData(2.0f),
		m_fDragTimer(0.0f),
		m_fTotalDragTimeToZero(0.25),
		m_fRotationScaler(0.01f),
		m_fMoveScaler(15.0f),
		m_bMovementDrag(false),
		m_bInvertPitch(false),
		m_bEnablePositionMovement(true),
		m_bEnableYAxisMovement(true),
		m_bResetCursorAfterMove(false),
		m_bUseGamepad(false),
		m_bUseMouse(true),
		m_bUseKeyboard(false),
		m_pNode(pNode)
	{
		//SetRect(&m_rcDrag, LONG_MIN, LONG_MIN, LONG_MAX, LONG_MAX);
		m_vVelocity = DirectX::SimpleMath::Vector3();
		m_vVelocityDrag = DirectX::SimpleMath::Vector3();
		m_vRotVelocity = DirectX::SimpleMath::Vector2();
		m_vMouseDelta = DirectX::SimpleMath::Vector2();

		//GetCursorPos(&m_ptLastMousePosition);
	}

	//--------------------------------------------------------------------------------------
	// Figure out the velocity based on keyboard input & drag if any
	//--------------------------------------------------------------------------------------
	void NodeController::UpdateVelocity(_In_ float fElapsedTime)
	{
		XMVECTOR vGamePadRightThumb = g_XMZero;
		XMVECTOR vGamePadLeftThumb = g_XMZero;
		XMVECTOR vMouseDelta = g_XMZero;
		XMVECTOR vKeyboardDirection = m_vKeyboardDirection;

		if (m_bUseGamepad)
		{
			vGamePadRightThumb = XMVectorSet(m_vGamePadRightThumb.x, -m_vGamePadRightThumb.z, 0, 0);
			vGamePadRightThumb *= XMVectorAbs(vGamePadRightThumb);
			vGamePadLeftThumb = XMVectorSet(m_vGamePadLeftThumb.x, 0, m_vGamePadLeftThumb.z, 0);
		}

		if (m_bUseMouse)
		{
			vMouseDelta = m_vMouseDelta;
		}

		XMVECTOR vRotVelocity = vMouseDelta * m_fRotationScaler + vGamePadRightThumb * 0.01f;
		m_vRotVelocity = vRotVelocity;

		XMVECTOR vAccel = vKeyboardDirection + vGamePadLeftThumb;

		// Normalize vector so if moving 2 dirs (left & forward), 
		// the camera doesn't move faster than if moving in 1 dir
		vAccel = XMVector3Normalize(vAccel);

		// Scale the acceleration vector
		vAccel *= m_fMoveScaler;

		if (m_bMovementDrag)
		{
			// Is there any acceleration this frame?
			if (XMVectorGetX(XMVector3LengthSq(vAccel)) > 0)
			{
				// If so, then this means the user has pressed a movement key
				// so change the velocity immediately to acceleration 
				// upon keyboard input.  This isn't normal physics
				// but it will give a quick response to keyboard input
				XMStoreFloat3(&m_vVelocity, vAccel);

				m_fDragTimer = m_fTotalDragTimeToZero;

				XMStoreFloat3(&m_vVelocityDrag, vAccel / m_fDragTimer);
			}
			else
			{
				// If no key being pressed, then slowly decrease velocity to 0
				if (m_fDragTimer > 0)
				{
					// Drag until timer is <= 0
					XMVECTOR vVelocity = XMLoadFloat3(&m_vVelocity);
					XMVECTOR vVelocityDrag = XMLoadFloat3(&m_vVelocityDrag);

					vVelocity -= vVelocityDrag * fElapsedTime;

					XMStoreFloat3(&m_vVelocity, vVelocity);

					m_fDragTimer -= fElapsedTime;
				}
				else
				{
					// Zero velocity
					m_vVelocity = DirectX::SimpleMath::Vector3();
				}
			}
		}
		else
		{
			// No drag, so immediately change the velocity
			m_vVelocity = vAccel;// *fElapsedTime;
		}
	}

	//--------------------------------------------------------------------------------------
	// Figure out the velocity based on keyboard input & drag if any
	//--------------------------------------------------------------------------------------
	_Use_decl_annotations_
		void NodeController::GetInput()
	{
		m_vKeyboardDirection = DirectX::SimpleMath::Vector3();
		m_vGamePadLeftThumb = DirectX::SimpleMath::Vector3();
		m_vGamePadRightThumb = DirectX::SimpleMath::Vector3();

		if (m_bUseKeyboard)
		{
			// Update acceleration vector based on keyboard state
			//if (IsKeyDown(m_aKeys[CAM_MOVE_FORWARD]))
			//	m_vKeyboardDirection.z += 1.0f;
			//if (IsKeyDown(m_aKeys[CAM_MOVE_BACKWARD]))
			//	m_vKeyboardDirection.z -= 1.0f;
			//if (m_bEnableYAxisMovement)
			//{
			//	if (IsKeyDown(m_aKeys[CAM_MOVE_UP]))
			//		m_vKeyboardDirection.y += 1.0f;
			//	if (IsKeyDown(m_aKeys[CAM_MOVE_DOWN]))
			//		m_vKeyboardDirection.y -= 1.0f;
			//}
			//if (IsKeyDown(m_aKeys[CAM_STRAFE_RIGHT]))
			//	m_vKeyboardDirection.x += 1.0f;
			//if (IsKeyDown(m_aKeys[CAM_STRAFE_LEFT]))
			//	m_vKeyboardDirection.x -= 1.0f;

			auto keyboard = Keyboard::Get().GetState();
			m_KeyboardTracker.Update(keyboard);

			if (m_KeyboardTracker.IsKeyHold(Keyboard::W))
				m_vKeyboardDirection.z += 1.0f;
			if (m_KeyboardTracker.IsKeyHold(Keyboard::A))
				m_vKeyboardDirection.x -= 1.0f;
			if (m_KeyboardTracker.IsKeyHold(Keyboard::S))
				m_vKeyboardDirection.z -= 1.0f;
			if (m_KeyboardTracker.IsKeyHold(Keyboard::D))
				m_vKeyboardDirection.x += 1.0f;
		}

		if (m_bUseMouse)
		{
			auto mouse = Mouse::Get().GetState();
			m_MouseTracker.Update(mouse);

			// Smooth the relative mouse data over a few frames so it isn't 
			// jerky when moving slowly at low frame rates.
			float fPercentOfNew = 1.0f / m_fFramesToSmoothMouseData;
			float fPercentOfOld = 1.0f - fPercentOfNew;
			m_vMouseDelta.x = m_vMouseDelta.x * fPercentOfOld + m_MouseTracker.xMovement * fPercentOfNew;
			m_vMouseDelta.y = m_vMouseDelta.y * fPercentOfOld + m_MouseTracker.yMovement * fPercentOfNew;

			m_vRotVelocity.x = m_vMouseDelta.x * m_fRotationScaler;
			m_vRotVelocity.y = m_vMouseDelta.y * m_fRotationScaler;
		}

		if (m_bUseGamepad)
		{
			auto gamepad = GamePad::Get().GetState(0);
			m_GamePadTracker.Update(gamepad);

			m_vGamePadLeftThumb.x = gamepad.thumbSticks.leftX;
			m_vGamePadLeftThumb.y = 0.0f;
			m_vGamePadLeftThumb.z = gamepad.thumbSticks.leftY;

			m_vGamePadRightThumb.x = gamepad.thumbSticks.rightX;
			m_vGamePadRightThumb.y = 0.0f;
			m_vGamePadRightThumb.z = gamepad.thumbSticks.rightY;
		}

		//if (bGetGamepadInput)
		//{
		//	m_vGamePadLeftThumb = DirectX::SimpleMath::Vector3();
		//	m_vGamePadRightThumb = DirectX::SimpleMath::Vector3();

		//	// Get controller state
		//	for (DWORD iUserIndex = 0; iUserIndex < MAX_CONTROLLERS; iUserIndex++)
		//	{
		//		GetGamepadState(iUserIndex, &m_GamePad[iUserIndex], true, true);

		//		// Mark time if the controller is in a non-zero state
		//		if (m_GamePad[iUserIndex].wButtons ||
		//			m_GamePad[iUserIndex].sThumbLX || m_GamePad[iUserIndex].sThumbLX ||
		//			m_GamePad[iUserIndex].sThumbRX || m_GamePad[iUserIndex].sThumbRY ||
		//			m_GamePad[iUserIndex].bLeftTrigger || m_GamePad[iUserIndex].bRightTrigger)
		//		{
		//			m_GamePadLastActive[iUserIndex] = DXUTGetTime();
		//		}
		//	}

		//	// Find out which controller was non-zero last
		//	int iMostRecentlyActive = -1;
		//	double fMostRecentlyActiveTime = 0.0f;
		//	for (DWORD iUserIndex = 0; iUserIndex < MAX_CONTROLLERS; iUserIndex++)
		//	{
		//		if (m_GamePadLastActive[iUserIndex] > fMostRecentlyActiveTime)
		//		{
		//			fMostRecentlyActiveTime = m_GamePadLastActive[iUserIndex];
		//			iMostRecentlyActive = iUserIndex;
		//		}
		//	}

		//	// Use the most recent non-zero controller if its connected
		//	if (iMostRecentlyActive >= 0 && m_GamePad[iMostRecentlyActive].bConnected)
		//	{
		//		if (bGetKeyboardInput)
		//		{
		//			m_vGamePadLeftThumb.x = m_GamePad[iMostRecentlyActive].fThumbLX;
		//			m_vGamePadLeftThumb.y = 0.0f;
		//			m_vGamePadLeftThumb.z = m_GamePad[iMostRecentlyActive].fThumbLY;
		//		}

		//		m_vGamePadRightThumb.x = m_GamePad[iMostRecentlyActive].fThumbRX;
		//		m_vGamePadRightThumb.y = 0.0f;
		//		m_vGamePadRightThumb.z = m_GamePad[iMostRecentlyActive].fThumbRY;
		//	}
		//}
	}

	//--------------------------------------------------------------------------------------
	// Figure out the mouse delta based on mouse movement
	//--------------------------------------------------------------------------------------
	void NodeController::UpdateMouseDelta()
	{
		// Get current position of mouse
		//POINT ptCurMousePos;
		//GetCursorPos(&ptCurMousePos);

		//// Calc how far it's moved since last frame
		//POINT ptCurMouseDelta;

		//ptCurMouseDelta.x = ptCurMousePos.x - m_ptLastMousePosition.x;
		//ptCurMouseDelta.y = ptCurMousePos.y - m_ptLastMousePosition.y;

		// Record current position for next time
		//m_ptLastMousePosition = ptCurMousePos;

		//if (m_bResetCursorAfterMove && DXUTIsActive())
		//{
		//	// Set position of camera to center of desktop, 
		//	// so it always has room to move.  This is very useful
		//	// if the cursor is hidden.  If this isn't done and cursor is hidden, 
		//	// then invisible cursor will hit the edge of the screen 
		//	// and the user can't tell what happened
		//	POINT ptCenter;

		//	// Get the center of the current monitor
		//	MONITORINFO mi;
		//	DXUTGetActiveMonitorInfo(mi);
		//	ptCenter.x = (mi.rcMonitor.left + mi.rcMonitor.right) / 2;
		//	ptCenter.y = (mi.rcMonitor.top + mi.rcMonitor.bottom) / 2;
		//	SetCursorPos(ptCenter.x, ptCenter.y);
		//	m_ptLastMousePosition = ptCenter;
		//}

		// Smooth the relative mouse data over a few frames so it isn't 
		// jerky when moving slowly at low frame rates.
		//float fPercentOfNew = 1.0f / m_fFramesToSmoothMouseData;
		//float fPercentOfOld = 1.0f - fPercentOfNew;
		//m_vMouseDelta.x = m_vMouseDelta.x * fPercentOfOld + ptCurMouseDelta.x * fPercentOfNew;
		//m_vMouseDelta.y = m_vMouseDelta.y * fPercentOfOld + ptCurMouseDelta.y * fPercentOfNew;

		//m_vRotVelocity.x = m_vMouseDelta.x * m_fRotationScaler;
		//m_vRotVelocity.y = m_vMouseDelta.y * m_fRotationScaler;
	}

	//--------------------------------------------------------------------------------------
	// Call this from your message proc so this class can handle window messages
	//--------------------------------------------------------------------------------------
	//_Use_decl_annotations_
	//	LRESULT NodeController::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	//{
	//	UNREFERENCED_PARAMETER(hWnd);
	//	UNREFERENCED_PARAMETER(lParam);

	//	switch (uMsg)
	//	{
	//	case WM_KEYDOWN:
	//	{
	//		// Map this key to a D3DUtil_CameraKeys enum and update the
	//		// state of m_aKeys[] by adding the KEY_WAS_DOWN_MASK|KEY_IS_DOWN_MASK mask
	//		// only if the key is not down
	//		D3DUtil_CameraKeys mappedKey = MapKey((UINT)wParam);
	//		if (mappedKey != CAM_UNKNOWN)
	//		{
	//			_Analysis_assume_(mappedKey < CAM_MAX_KEYS);
	//			if (FALSE == IsKeyDown(m_aKeys[mappedKey]))
	//			{
	//				m_aKeys[mappedKey] = KEY_WAS_DOWN_MASK | KEY_IS_DOWN_MASK;
	//				++m_cKeysDown;
	//			}
	//		}
	//		break;
	//	}

	//	case WM_KEYUP:
	//	{
	//		// Map this key to a D3DUtil_CameraKeys enum and update the
	//		// state of m_aKeys[] by removing the KEY_IS_DOWN_MASK mask.
	//		D3DUtil_CameraKeys mappedKey = MapKey((UINT)wParam);
	//		if (mappedKey != CAM_UNKNOWN && (DWORD)mappedKey < 8)
	//		{
	//			m_aKeys[mappedKey] &= ~KEY_IS_DOWN_MASK;
	//			--m_cKeysDown;
	//		}
	//		break;
	//	}

	//	case WM_RBUTTONDOWN:
	//	case WM_MBUTTONDOWN:
	//	case WM_LBUTTONDOWN:
	//	case WM_RBUTTONDBLCLK:
	//	case WM_MBUTTONDBLCLK:
	//	case WM_LBUTTONDBLCLK:
	//	{
	//		// Compute the drag rectangle in screen coord.
	//		POINT ptCursor =
	//		{
	//			(short)LOWORD(lParam), (short)HIWORD(lParam)
	//		};

	//		// Update member var state
	//		if ((uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONDBLCLK) && PtInRect(&m_rcDrag, ptCursor))
	//		{
	//			m_bMouseLButtonDown = true; m_nCurrentButtonMask |= MOUSE_LEFT_BUTTON;
	//		}
	//		if ((uMsg == WM_MBUTTONDOWN || uMsg == WM_MBUTTONDBLCLK) && PtInRect(&m_rcDrag, ptCursor))
	//		{
	//			m_bMouseMButtonDown = true; m_nCurrentButtonMask |= MOUSE_MIDDLE_BUTTON;
	//		}
	//		if ((uMsg == WM_RBUTTONDOWN || uMsg == WM_RBUTTONDBLCLK) && PtInRect(&m_rcDrag, ptCursor))
	//		{
	//			m_bMouseRButtonDown = true; m_nCurrentButtonMask |= MOUSE_RIGHT_BUTTON;
	//		}

	//		// Capture the mouse, so if the mouse button is 
	//		// released outside the window, we'll get the WM_LBUTTONUP message
	//		SetCapture(hWnd);
	//		GetCursorPos(&m_ptLastMousePosition);
	//		return TRUE;
	//	}

	//	case WM_RBUTTONUP:
	//	case WM_MBUTTONUP:
	//	case WM_LBUTTONUP:
	//	{
	//		// Update member var state
	//		if (uMsg == WM_LBUTTONUP)
	//		{
	//			m_bMouseLButtonDown = false; m_nCurrentButtonMask &= ~MOUSE_LEFT_BUTTON;
	//		}
	//		if (uMsg == WM_MBUTTONUP)
	//		{
	//			m_bMouseMButtonDown = false; m_nCurrentButtonMask &= ~MOUSE_MIDDLE_BUTTON;
	//		}
	//		if (uMsg == WM_RBUTTONUP)
	//		{
	//			m_bMouseRButtonDown = false; m_nCurrentButtonMask &= ~MOUSE_RIGHT_BUTTON;
	//		}

	//		// Release the capture if no mouse buttons down
	//		if (!m_bMouseLButtonDown &&
	//			!m_bMouseRButtonDown &&
	//			!m_bMouseMButtonDown)
	//		{
	//			ReleaseCapture();
	//		}
	//		break;
	//	}

	//	case WM_CAPTURECHANGED:
	//	{
	//		if ((HWND)lParam != hWnd)
	//		{
	//			if ((m_nCurrentButtonMask & MOUSE_LEFT_BUTTON) ||
	//				(m_nCurrentButtonMask & MOUSE_MIDDLE_BUTTON) ||
	//				(m_nCurrentButtonMask & MOUSE_RIGHT_BUTTON))
	//			{
	//				m_bMouseLButtonDown = false;
	//				m_bMouseMButtonDown = false;
	//				m_bMouseRButtonDown = false;
	//				m_nCurrentButtonMask &= ~MOUSE_LEFT_BUTTON;
	//				m_nCurrentButtonMask &= ~MOUSE_MIDDLE_BUTTON;
	//				m_nCurrentButtonMask &= ~MOUSE_RIGHT_BUTTON;
	//				ReleaseCapture();
	//			}
	//		}
	//		break;
	//	}

	//	case WM_MOUSEWHEEL:
	//		// Update member var state
	//		m_nMouseWheelDelta += (short)HIWORD(wParam);
	//		break;
	//	}

	//	return FALSE;
	//}

	void NodeController::FrameMove(_In_ float fElapsedTime)
	{
		// Get keyboard/mouse/gamepad input
		GetInput();

		UpdateVelocity(fElapsedTime);
	}

	NodeControllerFPS::NodeControllerFPS(Node* pNode) :
		NodeController(pNode),
		m_vMovement(g_XMZero),
		m_bRotateWithoutButtonDown(false),
		m_nActiveButtonMask(0x07)
	{
		XMMATRIX InvView = m_pNode->GetWorld();
		XMFLOAT3 zBasis;
		XMStoreFloat3(&zBasis, InvView.r[2]);

		m_fYawAngle = atan2f(zBasis.x, zBasis.z);
		float fLen = sqrtf(zBasis.z * zBasis.z + zBasis.x * zBasis.x);
		m_fPitchAngle = -atan2f(zBasis.y, fLen);
	};

	void NodeControllerFPS::FrameMove(_In_ float fElapsedTime)
	{
		NodeController::FrameMove(fElapsedTime);

		// Simple euler method to calculate position delta
		XMVECTOR vVelocity = XMLoadFloat3(&m_vVelocity);
		XMVECTOR vPosDelta = vVelocity * fElapsedTime;

		// If rotating the camera 
		if ((m_nActiveButtonMask && (m_MouseTracker.leftButton == Mouse::ButtonStateTracker::HELD))// & m_nCurrentButtonMask)
			|| m_bRotateWithoutButtonDown
			|| m_vGamePadRightThumb.x != 0
			|| m_vGamePadRightThumb.z != 0)
		{
			// Update the pitch & yaw angle based on mouse movement
			float fYawDelta = m_vRotVelocity.x;
			float fPitchDelta = m_vRotVelocity.y;

			// Invert pitch if requested
			if (m_bInvertPitch)
				fPitchDelta = -fPitchDelta;

			m_fPitchAngle += fPitchDelta;
			m_fYawAngle += fYawDelta;

			// Limit pitch to straight up or straight down
			m_fPitchAngle = max(-XM_PI / 2.0f, m_fPitchAngle);
			m_fPitchAngle = min(+XM_PI / 2.0f, m_fPitchAngle);
		}

		// Make a rotation matrix based on the camera's yaw & pitch
		XMMATRIX mCameraRot = XMMatrixRotationRollPitchYaw(m_fPitchAngle, m_fYawAngle, 0);

		// Transform vectors based on camera's rotation matrix
		XMVECTOR vWorldUp = XMVector3TransformCoord(g_XMIdentityR1, mCameraRot);
		XMVECTOR vWorldAhead = XMVector3TransformCoord(g_XMIdentityR2, mCameraRot);

		// Transform the position delta by the camera's rotation 
		if (!m_bEnableYAxisMovement)
		{
			// If restricting Y movement, do not include pitch
			// when transforming position delta vector.		
			mCameraRot = XMMatrixRotationRollPitchYaw(0.0f, m_fYawAngle, 0.0f);
		}
		XMVECTOR vPosDeltaWorld = XMVector3TransformCoord(vPosDelta, mCameraRot);
		//XMVECTOR vVelocityWorld = XMVector3TransformCoord(vVelocity, mCameraRot);

		//m_pNode->Accelerate(vVelocityWorld);

		// Move the eye position 
		XMVECTOR vEye = m_pNode->GetPosition();
		vEye += vPosDeltaWorld;
		vEye += m_vMovement;
		m_vMovement = XMVectorSet(0, 0, 0, 0);
		//	m_pNode->SetPosition(vEye);

		// Update the lookAt position based on the eye position
		XMVECTOR vLookAt = vEye + vWorldAhead;
		// Update the view matrix
		XMMATRIX view = XMMatrixLookAtLH(vEye, vLookAt, vWorldUp);
		XMMATRIX mInvView = XMMatrixInverse(nullptr, view);

		XMMATRIX mWorld = mCameraRot * XMMatrixTranslationFromVector(vEye);
		m_pNode->SetWorld(mWorld);

		// The axis basis vectors and camera position are stored inside the 
		// position matrix in the 4 rows of the camera's world matrix.
		// To figure out the yaw/pitch of the camera, we just need the Z basis vector
		XMFLOAT3 zBasis;
		XMStoreFloat3(&zBasis, mInvView.r[2]);

		m_fYawAngle = atan2f(zBasis.x, zBasis.z);
		float fLen = sqrtf(zBasis.z * zBasis.z + zBasis.x * zBasis.x);
		m_fPitchAngle = -atan2f(zBasis.y, fLen);
	}
}