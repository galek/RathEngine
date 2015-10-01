#pragma once
#include "Block.h"

//size_t Cube(size_t x, size_t y, size_t z, const XMFLOAT3& position, const XMFLOAT3& color, XMFLOAT3 min, XMFLOAT3 max, CHAR culling, Block(*blocks)[CHUNK_WIDTH + 2][CHUNK_WIDTH + 2], BLOCK_MESH* vertex);
size_t Cube(const Vector3& position, const Vector3& color, const Vector3& center, const Vector3& extents, CHAR culling, Block block, std::vector<BLOCK_MESH> & mesh);
size_t Cross(const Vector3& position, const Vector3& color, const Vector3& center, const Vector3& extents, CHAR culling, Block block, std::vector<BLOCK_MESH> & mesh);