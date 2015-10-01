#include "stdafx.h"
#include "TweakBar.h"

#include "Block.h"

static const TwStructMember TW_XMINT3[] = {
		{ "x", TW_TYPE_INT32, offsetof(XMINT3, x), "" },
		{ "y", TW_TYPE_INT32, offsetof(XMINT3, y), "" },
		{ "z", TW_TYPE_INT32, offsetof(XMINT3, z), "" }
};

static const TwStructMember TW_BLOCK[] = {
		{ "Type",		TW_TYPE_UINT16, offsetof(Block, mBlocktype), "" },
		{ "Light",		TW_TYPE_UINT16, offsetof(Block, mLight),	" hexa=true " },
		{ "Metadata",	TW_TYPE_UINT8,  offsetof(Block, mMetadata), " hexa=true " },
		{ "Status",		TW_TYPE_UINT8,  offsetof(Block, mStatus),	" hexa=true " },
		{ "Userdata",	TW_TYPE_UINT16, offsetof(Block, mUserdata), " hexa=true " },
};

void TweakBar::Create(ID3D11Device* pd3dDevice)
{
	TwInit(TW_DIRECT3D11, pd3dDevice);

	TwType TW_TYPE_XMINT3 = TwDefineStruct("TW_TYPE_XMINT3", TW_XMINT3, 3, sizeof(XMINT3), nullptr, nullptr);
	TwType TW_TYPE_BLOCK = TwDefineStruct("TW_TYPE_BLOCK", TW_BLOCK, 5, sizeof(Block), nullptr, nullptr);

	//m_pBar = TwNewBar("DebugTweakBar");

	//TwAddSeparator(m_pBar, "Timing", nullptr);
	//TwAddVarRO(m_pBar, "FPS", TW_TYPE_FLOAT, &RATHEngine::GetEngine()->m_State.m_FPS, " label='FPS: ' ");
	//TwAddVarRO(m_pBar, "Runtime", TW_TYPE_DOUBLE, &RATHEngine::GetEngine()->m_State.m_Time, " label='Runtime: ' ");

	//TwAddSeparator(m_pBar, "Light", nullptr);
	//TwAddVarRW(m_pBar, "Lightdirection", TW_TYPE_DIR3F, &pScene->mLightDirection, " label='Light direction: ' opened=true axisz=-z showval=false");

	//TwAddSeparator(m_pBar, "Blockview", nullptr);
	//TwAddVarRO(m_pBar, "Block", TW_TYPE_BLOCK, &pScene->mViewedBlock, " label='Block: ' ");
	//TwAddVarRO(m_pBar, "Blockpos", TW_TYPE_XMINT3, &pScene->mBlockCoord, " label='Block position: ' ");

	//TwDefine(" DebugTweakBar iconified=true ");
}

void TweakBar::ResizedSwapChain(const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{	
	TwWindowSize(pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height);
}

void TweakBar::ReleasingSwapChain()
{
	TwWindowSize(0, 0);
}

void TweakBar::Release()
{
	TwTerminate();
}

void TweakBar::Render()
{
	TwDraw();
}

void TweakBar::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	TwEventWin(hWnd, uMsg, wParam, lParam);
}