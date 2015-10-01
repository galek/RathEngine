//
// Main.cpp -
//

#include "pch.h"
#include "BaseEngine.h"

using namespace DirectX;

namespace
{
	std::unique_ptr<Rath::BaseEngine> g_game;
};
// Entry point
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

#if defined(_PROFILE) | defined(_DEBUG)
	if (AllocConsole())
	{
		FILE *stream;
		freopen_s(&stream, "CONIN$", "r", stdin);
		freopen_s(&stream, "CONOUT$", "w", stdout);
		freopen_s(&stream, "CONOUT$", "w", stderr);
	}
#endif

    HRESULT hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);
    if (FAILED(hr))
        return 1;

	g_game.reset(new Rath::BaseEngine(L"Game", L"IDI_ICON"));

	g_game->Run();

    g_game.reset();

    CoUninitialize();

    return 0;
}
