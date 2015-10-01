#include "stdafx.h"
#include "BlockEngine.h"

constexpr size_t str2int(const char* str, size_t h = 0)
{
	return !str[h] ? 5381 : (str2int(str, h + 1) * 33) ^ str[h];
}

#if defined(_PROFILE) | defined(_DEBUG)
int BlockEngine::setlight(lua_State* lua)
{
	int paramCount = lua_gettop(lua);

	if (paramCount != 1)
	{
		return -1;
	}

	Vector3 pos = getFloat3FromTable(lua, 1);
	if (g_BlockEngine)
		g_BlockEngine->m_State.m_LightDirection = XMVector3Normalize(pos);

	return 0;
}

int BlockEngine::setDebugSetting(lua_State* lua)
{
	int paramCount = lua_gettop(lua);

	if (paramCount != 2)
		return luaL_error(lua, "Expected 2 parameter.");

	if (lua_isstring(lua, 1))
	{
		size_t len = 0;
		const char* cstr = lua_tolstring(lua, 1, &len);
		bool toogle = false;
		if (lua_isboolean(lua, 2))
			toogle = lua_toboolean(lua, 2) != 0;
		else if (lua_isinteger(lua, 2))
			toogle = lua_tointeger(lua, 2) != 0;
		else
			return luaL_error(lua, "Boolean type expected as 2nd parameter.");

		if (g_BlockEngine)
		{
			switch (str2int(cstr))
			{
			case str2int("showSM"):
				g_BlockEngine->m_State.m_displayShadowMaps = toogle;
				break;
			case str2int("showSF"):
				g_BlockEngine->m_State.m_displayShadowFrustum = toogle;
				break;
			case str2int("showSB"):
				g_BlockEngine->m_State.m_displayShadowBuffer = toogle;
				break;
			default:
				return luaL_error(lua, "Command not recognized.");
			}
		}
	}

	return 0;
}
#endif