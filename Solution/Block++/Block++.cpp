// Block++.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"

#include "BlockEngine.h"
//--------------------------------------------------------------------------------------
// Initialize everything and go into a render loop
//--------------------------------------------------------------------------------------
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	// Enable run-time memory check for debug builds.
#if defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	auto print = [&](const XMMATRIX & mat, uint32 i)
	{
		std::cout << std::fixed 
				  << std::setprecision(1) 
				  << "{ " 
				  << mat.r[i].m128_f32[0] 
				  << "f, " 
				  << mat.r[i].m128_f32[1] 
				  << "f, "  
				  << mat.r[i].m128_f32[2] 
				  << "f }," 
				  << std::endl;
	};

	BlockEngine* pEngine = new BlockEngine(hInstance);

	pEngine->Run();

	delete pEngine;

	//XMMATRIX yp = XMMatrixIdentity() * XMMatrixRotationY(XM_PIDIV4);
	//XMMATRIX ym = XMMatrixRotationZ(XM_PI) * XMMatrixRotationY(XM_PIDIV4);

	//XMMATRIX xp = XMMatrixRotationZ(-XM_PIDIV2) * XMMatrixRotationY(XM_PIDIV4);
	//XMMATRIX xm = XMMatrixRotationZ(-XM_PIDIV2) * XMMatrixRotationY(XM_PI) * XMMatrixRotationY(XM_PIDIV4);

	//XMMATRIX zp = XMMatrixRotationZ(-XM_PIDIV2) * XMMatrixRotationY(-XM_PIDIV2) * XMMatrixRotationY(XM_PIDIV4);
	//XMMATRIX zm = XMMatrixRotationZ(-XM_PIDIV2) * XMMatrixRotationY(XM_PIDIV2) * XMMatrixRotationY(XM_PIDIV4);

	//print(xm, 1);
	//print(xp, 1);
	//print(zm, 1);
	//print(zp, 1);
	//print(ym, 1);
	//print(yp, 1);
	//std::cout << std::endl;
	//print(xm, 2);
	//print(xp, 2);
	//print(zm, 2);
	//print(zp, 2);
	//print(ym, 2);
	//print(yp, 2);
	//std::cout << std::endl;
	//print(xm, 0);
	//print(xp, 0);
	//print(zm, 0);
	//print(zp, 0);
	//print(ym, 0);
	//print(yp, 0);

	//DEVMODE dm;
	//// initialize the DEVMODE structure
	//ZeroMemory(&dm, sizeof(dm));
	//dm.dmSize = sizeof(dm);
	////if (0 != EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm))
	////{
	////	// inspect the DEVMODE structure to obtain details
	////	// about the display settings such as
	////	//  - Orientation
	////	//  - Width and Height
	////	//  - Frequency
	////	//  - etc.

	////	std::cout << dm.dmPelsWidth << "x" << dm.dmPelsHeight << "-" << dm.dmDisplayFrequency << std::endl;
	////}

	//for (int iModeNum = 0; EnumDisplaySettings(NULL, iModeNum, &dm) != 0; iModeNum++) 
	//{
	//	std::cout << dm.dmPelsWidth << "x" << dm.dmPelsHeight << "-" << dm.dmDisplayFrequency << std::endl;
	//}

	//do
	//{
	//	std::cout << '\n' << "Press a key to continue...";
	//} while (std::cin.get() != '\n');

	return 0;
}

