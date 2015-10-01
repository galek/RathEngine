#pragma once
#include "LuaUtils.h"

class LuaConsole
{
protected:
	volatile BOOL				mTerminate;
	static DWORD WINAPI			run(LPVOID pParam);
	HANDLE						mThreadHandle;
	HANDLE						hConsole;

	lua_State*					mLua;
	void initLua();
public:
	LuaConsole();
	~LuaConsole();

	void Kill();
	void Stop();
	void Start();

	void AddFunction(lua_CFunction fn, const char *name);

	void parseLine(char * pLine);

	BOOL isRunning();
};
