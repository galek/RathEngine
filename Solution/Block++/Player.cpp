#include "stdafx.h"
#include "Player.h"

Player::Player(Rath::Node* head) :
	m_fFramesToSmoothMouseData(2.0f),
	m_fRotationScaler(0.01f),
	m_fMoveScaler(15.0f),
	m_bUseGamepad(false),
	m_bUseMouse(true),
	m_bUseKeyboard(true),
	m_pHead(head)
{
	m_pHead->SetParent(this);

	mWorld = DirectX::SimpleMath::Matrix(
		-0.459742427f, 0.000000000f, 0.888052285f, 0.000000000f,
		0.000000000f, 1.00000000f, 0.000000000f, 0.000000000f,
		-0.888052285f, 0.000000000f, -0.459742427f, 0.000000000f,
		46.7789955f, 141.000000f, 55.7633896f, 1.00000000f);

	setupFiltering(CollisionType::ePlayer, CollisionType::eEntityItem | CollisionType::eEnemy);
}


Player::~Player()
{
}

inline void GetRotation(const DirectX::SimpleMath::Matrix& world, float& Yaw, float& Pitch, float& Roll)
{
	if (world._11 == 1.0f)
	{
		Yaw = atan2f(world._31, world._43);
		Pitch = 0;
		Roll = 0;
	}
	else if (world._11 == -1.0f)
	{
		Yaw = atan2f(world._31, world._43);
		Pitch = 0;
		Roll = 0;
	}
	else
	{
		Yaw = atan2(-world._13, world._11);
		Pitch = asin(world._12);
		Roll = atan2(-world._32, world._22);
	}
}

void XM_CALLCONV Player::SetWorld(_In_ XMMATRIX world)
{
	Rath::Node::SetWorld(world);

	float Yaw, Pitch, Roll;

	GetRotation(Matrix(world), Yaw, Pitch, Roll);

	m_fPitchAngle = Pitch;
	m_fYawAngle = Yaw;
}

void Player::GetInput()
{
	m_vKeyboardDirection = DirectX::SimpleMath::Vector3();
	m_vGamePadLeftThumb = DirectX::SimpleMath::Vector3();
	m_vGamePadRightThumb = DirectX::SimpleMath::Vector3();

	if (m_bUseKeyboard)
	{
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
		if (m_KeyboardTracker.IsKeyPressed(Keyboard::Space) && mGrounded)
			mVelocity += DirectX::SimpleMath::Vector3(0.0f, 50.0f, 0.0f);
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

	// If rotating the camera 
	if (m_MouseTracker.leftButton == Mouse::ButtonStateTracker::HELD
		|| m_vGamePadRightThumb.x != 0
		|| m_vGamePadRightThumb.z != 0)
	{
		// Update the pitch & yaw angle based on mouse movement
		float fYawDelta = m_vRotVelocity.x;
		float fPitchDelta = m_vRotVelocity.y;

		m_fPitchAngle += fPitchDelta;
		m_fYawAngle += fYawDelta;

		// Limit pitch to straight up or straight down
		m_fPitchAngle = max(-XM_PI / 2.0f, m_fPitchAngle);
		m_fPitchAngle = min(+XM_PI / 2.0f, m_fPitchAngle);
	}
}

void Player::UpdateAcceleration(float fElapsedTime)
{
	XMVECTOR vGamePadRightThumb = g_XMZero;
	XMVECTOR vGamePadLeftThumb = g_XMZero;
	XMVECTOR vMouseDelta = g_XMZero;
	XMVECTOR vKeyboardDirection = XMVectorSet(m_vKeyboardDirection.x, 0.f, m_vKeyboardDirection.z, 0.f);

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
	XMVECTOR vAccel = XMVector3Normalize(vKeyboardDirection + vGamePadLeftThumb) * m_fMoveScaler;
	vAccel = XMVector4Transform(vAccel, GetWorld());

	m_vRotVelocity = vRotVelocity;
	mAcceleration += vAccel * fElapsedTime;
}

void Player::FrameMove(float fElapsedTime)
{
	GetInput();

	XMMATRIX mBodyCameraRot = XMMatrixRotationRollPitchYaw(0, m_fYawAngle, 0);
	XMMATRIX mHeadCameraRot = XMMatrixRotationRollPitchYaw(m_fPitchAngle, 0, 0);

	mHeadCameraRot.r[3] = XMVectorSet(0.f, 0.6f, 0.f, 1.f);

	mWorld = mBodyCameraRot * XMMatrixTranslationFromVector(mWorld.Translation());
	m_pHead->SetRelativeWorld(mHeadCameraRot);

	UpdateAcceleration(fElapsedTime);

	ControllerNode::FrameMove(fElapsedTime);
}