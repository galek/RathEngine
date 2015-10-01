#pragma once
#include "Node.h"

#include "GamePad.h"
#include "Keyboard.h"
#include "Mouse.h"

namespace Rath
{
	class NodeController
	{
	private:
		void UpdateMouseDelta();
		void UpdateVelocity(_In_ float fElapsedTime);
	protected:
		void GetInput();

		GamePad::ButtonStateTracker		m_GamePadTracker;
		Mouse::ButtonStateTracker		m_MouseTracker;
		Keyboard::KeyboardStateTracker	m_KeyboardTracker;

		bool							m_bUseGamepad;
		bool							m_bUseMouse;
		bool							m_bUseKeyboard;

		DirectX::SimpleMath::Vector3		m_vGamePadLeftThumb;
		DirectX::SimpleMath::Vector3		m_vGamePadRightThumb;

		DirectX::SimpleMath::Vector3		m_vKeyboardDirection;				// Direction vector of keyboard input

		int				m_nMouseWheelDelta;                 // Amount of middle wheel scroll (+/-) 
		DirectX::SimpleMath::Vector2		m_vMouseDelta;						// Mouse relative delta smoothed over a few frames

		float			m_fFramesToSmoothMouseData;			// Number of frames to smooth mouse data over

		//RECT			m_rcDrag;							// Rectangle within which a drag can be initiated.
		DirectX::SimpleMath::Vector3		m_vVelocity;						// Velocity of camera
		DirectX::SimpleMath::Vector3		m_vVelocityDrag;					// Velocity drag force
		float			m_fDragTimer;						// Countdown timer to apply drag
		float			m_fTotalDragTimeToZero;				// Time it takes for velocity to go from full to 0
		DirectX::SimpleMath::Vector2		m_vRotVelocity;						// Velocity of camera
		float			m_fRotationScaler;					// Scaler for rotation
		float			m_fMoveScaler;						// Scaler for movement

		bool m_bMovementDrag;                   // If true, then camera movement will slow to a stop otherwise movement is instant
		bool m_bInvertPitch;                    // Invert the pitch axis
		bool m_bEnablePositionMovement;         // If true, then the user can translate the camera/model 
		bool m_bEnableYAxisMovement;            // If true, then camera can move in the y-axis
		bool m_bResetCursorAfterMove;           // If true, the class will reset the cursor position so that the cursor always has space to move 

		Node*	m_pNode;

		NodeController(Node* pNode);
	public:
		//~NodeController();

		virtual void FrameMove(_In_ float fElapsedTime);

		// Functions to change behavior
		void SetInvertPitch(_In_ bool bInvertPitch) { m_bInvertPitch = bInvertPitch; }
		void SetDrag(_In_ bool bMovementDrag, _In_ float fTotalDragTimeToZero = 0.25f)
		{
			m_bMovementDrag = bMovementDrag;
			m_fTotalDragTimeToZero = fTotalDragTimeToZero;
		}
		void SetEnableYAxisMovement(_In_ bool bEnableYAxisMovement) { m_bEnableYAxisMovement = bEnableYAxisMovement; }
		void SetEnablePositionMovement(_In_ bool bEnablePositionMovement) { m_bEnablePositionMovement = bEnablePositionMovement; }
		void SetScalers(_In_ float fRotationScaler = 0.01f, _In_ float fMoveScaler = 5.0f)
		{
			m_fRotationScaler = fRotationScaler;
			m_fMoveScaler = fMoveScaler;
		}
		void SetNumberOfFramesToSmoothMouseData(_In_ int nFrames) { if (nFrames > 0) m_fFramesToSmoothMouseData = (float)nFrames; }
		void SetResetCursorAfterMove(_In_ bool bResetCursorAfterMove) { m_bResetCursorAfterMove = bResetCursorAfterMove; }
	};

	class NodeControllerFPS : public NodeController
	{
	protected:
		DirectX::XMVECTOR	m_vMovement;
		int					m_nActiveButtonMask;            // Mask to determine which button to enable for rotation
		bool				m_bRotateWithoutButtonDown;

		float				m_fYawAngle;              // Yaw angle of camera
		float				m_fPitchAngle;            // Pitch angle of camera
	public:
		NodeControllerFPS(Node* pNode);

		virtual void FrameMove(_In_ float fElapsedTime) override;
	};

	class NodeControllerArcBall : public NodeController
	{
	protected:
		DirectX::XMVECTOR	m_vMovement;
		int					m_nActiveButtonMask;            // Mask to determine which button to enable for rotation
		bool				m_bRotateWithoutButtonDown;
	public:
	};
}