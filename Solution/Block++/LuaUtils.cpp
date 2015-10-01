#include "stdafx.h"
#include "LuaUtils.h"

unsigned int getUnsignedFromTable(lua_State *lua, int index, const char * key)
{
	lua_getfield(lua, index, key);
	unsigned int rtn = (unsigned int)lua_tointeger(lua, -1);
	lua_pop(lua, 1);
	return rtn;
}

int getIntegerFromTable(lua_State *lua, int index, const char * key)
{
	lua_getfield(lua, index, key);
	int rtn = (int)lua_tointeger(lua, -1);
	lua_pop(lua, 1);
	return rtn;
}

float getNumberFromTable(lua_State *lua, int index, const char * key)
{
	lua_getfield(lua, index, key);
	float rtn = (float)lua_tonumber(lua, -1);
	lua_pop(lua, 1);
	return rtn;
}

Vector3 getFloat3FromTable(lua_State *lua, int index)
{
	lua_rawgeti(lua, index, 1);
	float x = (float)lua_tonumber(lua, -1);
	lua_pop(lua, 1);

	lua_rawgeti(lua, index, 2);
	float y = (float)lua_tonumber(lua, -1);
	lua_pop(lua, 1);

	lua_rawgeti(lua, index, 3);
	float z = (float)lua_tonumber(lua, -1);
	lua_pop(lua, 1);

	return Vector3(x, y, z);
}

VectorInt3 getInt3FromTable(lua_State *lua, int index)
{
	lua_rawgeti(lua, index, 1);
	int x = (int)lua_tointeger(lua, -1);
	lua_pop(lua, 1);

	lua_rawgeti(lua, index, 2);
	int y = (int)lua_tointeger(lua, -1);
	lua_pop(lua, 1);

	lua_rawgeti(lua, index, 3);
	int z = (int)lua_tointeger(lua, -1);
	lua_pop(lua, 1);

	return VectorInt3(x, y, z);
}

Block getBlockFromLua(lua_State *lua, int index)
{
	Block block = Block::Air;

	if (lua_type(lua, index) == LUA_TTABLE)
	{
		Blocktype type = (Blocktype)getUnsignedFromTable(lua, index, "id");
		Blockstatus status = (Blockstatus)getUnsignedFromTable(lua, index, "status");
		uint8 metadata = (uint8)getUnsignedFromTable(lua, index, "metadata");
		uint16 userdata = (uint16)getUnsignedFromTable(lua, index, "userdata");
		Block buffer = { type, Light(), status, metadata, userdata };
		block = buffer;
	}
	else
	{
		uint16 type = (uint16)lua_tointeger(lua, index);
	}

	return block;
}


void setInt3ToTable(lua_State *lua, const VectorInt3& position)
{
	lua_createtable(lua, 3, 0);

	lua_pushinteger(lua, position.x);
	lua_rawseti(lua, -2, 1);

	lua_pushinteger(lua, position.y);
	lua_rawseti(lua, -2, 2);

	lua_pushinteger(lua, position.z);
	lua_rawseti(lua, -2, 3);
}

void setFloat3ToTable(lua_State *lua, const Vector3& position)
{
	lua_createtable(lua, 3, 0);

	lua_pushnumber(lua, position.x);
	lua_rawseti(lua, -2, 1);

	lua_pushnumber(lua, position.y);
	lua_rawseti(lua, -2, 2);

	lua_pushnumber(lua, position.z);
	lua_rawseti(lua, -2, 3);
}

void setBlockToLua(lua_State *lua, Block block)
{
	lua_newtable(lua);
	lua_pushinteger(lua, (uint16)block.mBlocktype);
	lua_setfield(lua, -2, "id");

	lua_pushinteger(lua, (uint8)block.mStatus);
	lua_setfield(lua, -2, "status");

	lua_pushinteger(lua, block.mMetadata);
	lua_setfield(lua, -2, "metadata");

	lua_pushinteger(lua, block.mUserdata);
	lua_setfield(lua, -2, "userdata");
}