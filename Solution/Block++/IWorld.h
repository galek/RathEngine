#pragma once

#include "Block.h"

class IWorld
{
public:
	Block virtual GetBlock(const VectorInt3& position) = 0;
	void virtual  SetBlock(const VectorInt3& position, const Block& block, Blockupdate flag = Blockupdate::None) = 0;

	template <size_t X, size_t Y, size_t Z>
	void virtual  GetBlocks(const VectorInt3& position, Block(*blocks)[Z][X]) = 0;

	void virtual  SetLight(const VectorInt3& position, const Light oldlight) = 0;
	void virtual  RemoveLight(const VectorInt3& position, const Light newlight) = 0;
}