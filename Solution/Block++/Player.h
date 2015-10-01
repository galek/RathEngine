#pragma once
#include "ControllerNode.h"

#include "GamePad.h"
#include "Keyboard.h"
#include "Mouse.h"

class Player : public Rath::ControllerNode
{
protected:
	Rath::Node*						m_pHead;

	GamePad::ButtonStateTracker		m_GamePadTracker;
	Mouse::ButtonStateTracker		m_MouseTracker;
	Keyboard::KeyboardStateTracker	m_KeyboardTracker;

	bool							m_bUseGamepad;
	bool							m_bUseMouse;
	bool							m_bUseKeyboard;

	DirectX::SimpleMath::Vector3	m_vGamePadLeftThumb;
	DirectX::SimpleMath::Vector3	m_vGamePadRightThumb;
	DirectX::SimpleMath::Vector3	m_vKeyboardDirection;
	DirectX::SimpleMath::Vector2	m_vMouseDelta;
	DirectX::SimpleMath::Vector2	m_vRotVelocity;

	float							m_fFramesToSmoothMouseData;
	float							m_fRotationScaler;
	float							m_fMoveScaler;

	float							m_fYawAngle;
	float							m_fPitchAngle;

	void GetInput();
	void UpdateAcceleration(float fElapsedTime);
public:
	Player(Rath::Node* head);
	~Player();

	void XM_CALLCONV SetWorld(_In_ XMMATRIX world) override;
	void FrameMove(float fElapsedTime) override;
};

