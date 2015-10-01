#include "stdafx.h"
#include "LuaConsole.h"

LuaConsole::LuaConsole()
{
	mTerminate = false;
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	mThreadHandle = CreateThread(NULL, 0, run, this, CREATE_SUSPENDED, NULL);
	initLua();
}

LuaConsole::~LuaConsole()
{
	Kill();

	lua_close(mLua);
}

void LuaConsole::Kill()
{
	TerminateThread(mThreadHandle, 0);
}

void LuaConsole::Stop()
{
	SuspendThread(mThreadHandle);
}

void LuaConsole::Start()
{
	ResumeThread(mThreadHandle);
}


BOOL LuaConsole::isRunning()
{
	return !mTerminate;
}

void LuaConsole::AddFunction(lua_CFunction fn, const char *name)
{
	lua_pushcfunction(mLua, fn);
	lua_setglobal(mLua, name);
}

void LuaConsole::initLua()
{
	mLua = luaL_newstate();
	luaL_openlibs(mLua);
}

void LuaConsole::parseLine(char * pLine)
{
	if (luaL_loadstring(mLua, pLine))
	{
		std::string error(lua_tostring(mLua, -1));
		lua_pop(mLua, 1);

		SetConsoleTextAttribute(hConsole, 12);
		std::cout << error.c_str() << std::endl;
		SetConsoleTextAttribute(hConsole, 7);
		return;
	}
	else
	{
		// The statment compiled correctly, now run it.
		if (lua_pcall(mLua, 0, LUA_MULTRET, 0))
		{
			std::string error(lua_tostring(mLua, -1));

			SetConsoleTextAttribute(hConsole, 12);
			std::cout << error.c_str() << std::endl;
			SetConsoleTextAttribute(hConsole, 7);

			// The error message (if any) will be added to the output as part
			// of the stack reporting.
			lua_gc(mLua, LUA_GCCOLLECT, 0);     // Do a full garbage collection on errors.
		}
	}

	// Clear stack
	lua_settop(mLua, 0);
}

DWORD WINAPI LuaConsole::run(LPVOID pParam)
{
	LuaConsole* This = (LuaConsole*)pParam;

	while (This->isRunning())
	{
		char	input[100];
		std::cin.getline(input, 100);
		This->parseLine(input);
	}

	return 0;
}