#pragma once
#include "Block.h"
#include "Lua.hpp"

unsigned int	getUnsignedFromTable(lua_State *lua, int index, const char * key);
int				getIntegerFromTable(lua_State *lua, int index, const char * key);
float			getNumberFromTable(lua_State *lua, int index, const char * key);

Vector3			getFloat3FromTable(lua_State *lua, int index);
VectorInt3		getInt3FromTable(lua_State *lua, int index);
Block			getBlockFromLua(lua_State *lua, int index);

void			setInt3ToTable(lua_State *lua, const VectorInt3& position);
void			setFloat3ToTable(lua_State *lua, const Vector3& position);
void			setBlockToLua(lua_State *lua, Block block);