#pragma once
#include "AntTweakBar.h"

class TweakBar
{
public:
	TwBar*				m_pBar;
public:
	void Create(ID3D11Device* pd3dDevice);
	void ResizedSwapChain(const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);

	void ReleasingSwapChain();
	void Release();

	void Render();

	void MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

