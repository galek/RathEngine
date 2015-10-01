#include "stdafx.h"
#include "Chunk.h"
#include "Region.h"
#include "BiomeGenerator.h"

#define IN_CHUNK(p) (((((p.x ^ mPosition.x) | (p.z ^ mPosition.z)) & CHUNK_WIDTH_INVMASK) | ((p.y ^ mPosition.y) & CHUNK_HEIGHT_INVMASK)) == 0)

Chunk::Chunk(Region* parent, const VectorInt3& position) :
mParent(parent),
mPosition(position),
mLoadedHeight(0),
mVisibleHeight(0),
mLightHeight(CHUNK_HEIGHT),
mBlocksDirect(nullptr),
mHeight(nullptr)
{
}

Chunk::~Chunk()
{
	if (mBlocksDirect) free(mBlocksDirect);
	if (mHeight) free(mHeight);
}

Block Chunk::GetBlock(const VectorInt3& position) const
{
	if (IN_CHUNK(position))
	{
		size_t x = (position.x & CHUNK_WIDTH_MASK);
		size_t y = (position.y & CHUNK_HEIGHT_MASK);
		size_t z = (position.z & CHUNK_WIDTH_MASK);

		if (y >= mLoadedHeight)
			return Block::Sky;
		else
			return mBlocks[y][z][x];
	}
	else
	{
		return mParent->GetBlock(position);
	}
}

template <size_t X, size_t Y, size_t Z>
void Chunk::GetBlocks(const VectorInt3& position, Block(*blocks)[Z][X])
{
	bool low = ((((position.x ^ mPosition.x) | (position.z ^ mPosition.z)) & CHUNK_WIDTH_INVMASK) == 0);
	bool high = (((((position.x + X) ^ mPosition.x) | ((position.z + Z) ^ mPosition.z)) & CHUNK_WIDTH_INVMASK) == 0);

	if (low && high)
	{
		int x = (position.x & CHUNK_WIDTH_MASK);
		int y = position.y;
		int z = (position.z & CHUNK_WIDTH_MASK);

		for (int i = 0; i < Y; i++)
			if (y + i >= mLoadedHeight)
				for (int j = 0; j < Z; j++)
					memcpy(&blocks[Y][Z][0], &mBlocks[y + i][z + j][x], X * sizeof(Block));
			else if (y + i < 0)
				std::fill(&blocks[i][0][0], &blocks[i + 1][0][0], VOID_BLOCK);
			else
				std::fill(&blocks[i][0][0], &blocks[i + 1][0][0], SKY_BLOCK);
	}
	else
	{
		XMINT3 pos;
		for (pos.y = 0; pos.y < Y; pos.y++)
			for (pos.z = 0; pos.z < Z; pos.z++)
				for (pos.x = 0; pos.x < X; pos.x)
					blocks[Y][Z][X] = GetBlock(position + pos);
	}
}

void  Chunk::SetBlock(const VectorInt3& position, const Block& block, Blockupdate flag)
{
	if (IN_CHUNK(position))
	{
		size_t x = (position.x & CHUNK_WIDTH_MASK);
		size_t y = (position.y & CHUNK_HEIGHT_MASK);
		size_t z = (position.z & CHUNK_WIDTH_MASK);

		//Resize(WORD(y + 1));
		Block oldBlock = mBlocks[y][z][x];

		mBlocks[y][z][x] = block;

		if (oldBlock.mLight.rgbs != block.mLight.rgbs)
		{
			if (oldBlock.mLight.rgbs > 0)
				RemoveLight(position, oldBlock.mLight);
			if (block.mLight.rgbs > 0) 
				SetLight(position, block.mLight);
		}
		else if ((oldBlock.mOpaque != block.mOpaque) || (oldBlock.mLDV != block.mLDV))
		{
			std::shared_ptr<Chunk> This = shared_from_this();

			std::queue<LightNode> lightBfsQueue;
			uint32 index = (uint32)(position.y+1 & CHUNK_HEIGHT_MASK) * CHUNK_SQRWIDTH +
				(uint32)(position.z & CHUNK_WIDTH_MASK) * CHUNK_WIDTH +
				(uint32)(position.x & CHUNK_WIDTH_MASK);
			lightBfsQueue.emplace(index, This);

			SetLight(lightBfsQueue);
		}
	}
	else
	{
		mParent->SetBlock(position, block, flag);
	}
}

void Chunk::Resize(WORD height, bool FillWithSky)
{
	if (height > mLoadedHeight)
	{
		height = ((height + 15) / 16) * 16;

		size_t oldsize = mLoadedHeight * CHUNK_SQRWIDTH * sizeof(Block);
		size_t newsize = height * CHUNK_SQRWIDTH * sizeof(Block);

		Block* Mem = (Block*)malloc(newsize);
		if (mBlocksDirect)
		{
			memcpy(Mem, mBlocksDirect, oldsize);
			free(mBlocksDirect);

			if (FillWithSky)
				std::fill(Mem + mLoadedHeight * CHUNK_SQRWIDTH, Mem + height * CHUNK_SQRWIDTH, Block::Sky);
			else
				memset(Mem + mLoadedHeight * CHUNK_SQRWIDTH, 0, newsize - oldsize);
		}
		else
		{
			memset(Mem, 0, newsize);
		}
		mBlocksDirect = Mem;
		mLoadedHeight = height;
	}
}

void Chunk::GenerateShape()
{
	auto index = [](uint32 x, uint32 y, uint32 z)
	{
		return y * CHUNK_SQRWIDTH + z * CHUNK_WIDTH + x;
	};

	XMINT3* boxes = new XMINT3[mLoadedHeight * CHUNK_SQRWIDTH];

	for (uint32 y(0); y < mLoadedHeight; y++)
	{
		for (uint32 z(0); z < CHUNK_WIDTH; z++)
		{
			for (uint32 x(0); x < CHUNK_WIDTH; x++)
			{
				uint32 i = index(x, y, z);

				Block block = mBlocksDirect[i];

				if ((block.mBlocktype == Blocktype::Air) || (block.mBlocktype == Blocktype::Void) || block.mPassable)
					boxes[i] = XMINT3(0, 0, 0);
				else
					boxes[i] = XMINT3(1, 1, 1);
			}
		}
	}
	for (uint32 y(0); y < mLoadedHeight; y++)
	{
		for (uint32 z(0); z < CHUNK_WIDTH; z++)
		{
			for (uint32 x(0); x < CHUNK_WIDTH - 1; x++)
			{
				uint32 i = index(x, y, z);
				uint32 i2 = index(x + 1, y, z);
				if (boxes[i2].x > 0 &&
					boxes[i].x > 0 &&
					boxes[i].y == boxes[i2].y && 
					boxes[i].z == boxes[i2].z)
				{
					boxes[i2].x += boxes[i].x;
					boxes[i] = XMINT3(0, 0, 0);
				}
			}
		}
	}
	for (uint32 y(0); y < mLoadedHeight; y++)
	{
		for (uint32 x(0); x < CHUNK_WIDTH; x++)
		{
			for (uint32 z(0); z < CHUNK_WIDTH - 1; z++)
			{
				uint32 i = index(x, y, z);
				uint32 i2 = index(x, y, z + 1);
				if (boxes[i2].z > 0 &&
					boxes[i].z > 0 &&
					boxes[i].x == boxes[i2].x &&
					boxes[i].y == boxes[i2].y)
				{
					boxes[i2].z += boxes[i].z;
					boxes[i] = XMINT3(0, 0, 0);
				}
			}
		}
	}
	for (uint32 z(0); z < CHUNK_WIDTH; z++)
	{
		for (uint32 x(0); x < CHUNK_WIDTH; x++)
		{
			for (uint32 y(0); y < uint32(mLoadedHeight - 1); y++)
			{
				uint32 i = index(x, y, z);
				uint32 i2 = index(x, y + 1, z);
				if (boxes[i2].y > 0 &&
					boxes[i].y > 0 &&
					boxes[i].x == boxes[i2].x &&
					boxes[i].z == boxes[i2].z)
				{
					boxes[i2].y += boxes[i].y;
					boxes[i] = XMINT3(0, 0, 0);
				}
			}
		}
	}


	if (pCollisionShape != nullptr)
	{
		Rath::PhysicsManager::instance().RemoveActor(pCollisionShape);
		pCollisionShape->release();
	}

	physx::PxTransform localTm((float)mPosition.x, (float)mPosition.y, (float)mPosition.z);
	pCollisionShape = Rath::PhysicsManager::instance().createStatic(localTm);
	physx::PxMaterial* mat = Rath::PhysicsManager::instance().GetMaterial(0);

	for (uint32 y(0); y < mLoadedHeight; y++)
	{
		for (uint32 z(0); z < CHUNK_WIDTH; z++)
		{
			for (uint32 x(0); x < CHUNK_WIDTH; x++)
			{
				uint32 i = index(x, y, z);
				if (boxes[i].x > 0 && 
					boxes[i].y > 0 && 
					boxes[i].z > 0)
				{
					physx::PxVec3 extents = physx::PxVec3((float)boxes[i].x, (float)boxes[i].y, (float)boxes[i].z) / 2.0f;
					physx::PxTransform position = physx::PxTransform(physx::PxVec3((float)x+1, (float)y+1, (float)z+1) - extents);
					physx::PxShape* shape = pCollisionShape->createShape(physx::PxBoxGeometry(extents), *mat, position);
				}
			}
		}
	}
	//Rath::PhysicsManager::instance().setupFiltering(pCollisionShape, CollisionType::ePlayer, CollisionType::eEntityItem | CollisionType::eEnemy);
	Rath::PhysicsManager::instance().AddActor(pCollisionShape);

	delete[] boxes;
}

void Chunk::GenerateMesh(std::vector<BLOCK_MESH> & Mesh, std::vector<uint32> & Light)
{
	mVisibleHeight = 0;
	if (mLoadedHeight > 0)
	{
#define INDEX(x,y,z) (((y) * (CHUNK_WIDTH + 2) * (CHUNK_WIDTH + 2)) + ((z) * (CHUNK_WIDTH + 2)) + (x))
#define LINDEX(x,y,z) (((y) * (CHUNK_WIDTH + 2)) + ((z) * (CHUNK_WIDTH + 2) * CHUNK_HEIGHT) + (x))
		union
		{
			uint8(*cull)[CHUNK_WIDTH][CHUNK_WIDTH];
			uint8(*cullLayer)[CHUNK_SQRWIDTH];
			uint8 *cullDirect;
		};
		cullDirect = (uint8*)malloc(mLoadedHeight * CHUNK_SQRWIDTH);
		memset(cullDirect, 0, (mLoadedHeight - 1) * CHUNK_SQRWIDTH);
		memset(cullLayer[mLoadedHeight - 1], (1 << 5), CHUNK_SQRWIDTH);
		//std::fill(Light.begin(), Light.begin() + (CHUNK_WIDTH + 2) * (CHUNK_WIDTH + 2), 0x00000000);
		//std::fill(Light.begin() + INDEX(0, mLoadedHeight, 0), Light.end(), 0xFFFFFFFF);

		for (uint32 y(0); y < mLoadedHeight; y++)
		{
			for (uint32 z(0); z < CHUNK_WIDTH; z++)
			{
				for (uint32 x(0); x < CHUNK_WIDTH; x++)
				{
					Block block = mBlocks[y][z][x];
					uint8 status = !block.mOpaque;

					if (status)
					{
						if (x < CHUNK_WIDTH - 1)	cull[y][z][x + 1] |= status << 0;
						if (x > 0)					cull[y][z][x - 1] |= status << 1;
						if (z < CHUNK_WIDTH - 1)	cull[y][z + 1][x] |= status << 2;
						if (z > 0)					cull[y][z - 1][x] |= status << 3;
						if (y < (uint32)mLoadedHeight - 1)	cull[y + 1][z][x] |= status << 4;
						if (y > 0)					cull[y - 1][z][x] |= status << 5;
					}

					Light[LINDEX(x + 1, y, z + 1)] = block.mLight.Color();
				}

				Block sides[] = {
					mParent->GetBlock(VectorInt3(mPosition.x - 1, y, mPosition.z + z)),
					mParent->GetBlock(VectorInt3(mPosition.x + CHUNK_WIDTH, y, mPosition.z + z)),
					mParent->GetBlock(VectorInt3(mPosition.x + z, y, mPosition.z - 1)),
					mParent->GetBlock(VectorInt3(mPosition.x + z, y, mPosition.z + CHUNK_WIDTH)),
				};

				cull[y][z][0]				|= (!sides[0].mOpaque) << 0;
				cull[y][z][CHUNK_WIDTH - 1] |= (!sides[1].mOpaque) << 1;
				cull[y][0][z]				|= (!sides[2].mOpaque) << 2;
				cull[y][CHUNK_WIDTH - 1][z] |= (!sides[3].mOpaque) << 3;

				Light[LINDEX(0, y, z + 1)] = sides[0].mLight.Color();
				Light[LINDEX(CHUNK_WIDTH + 1, y, z + 1)] = sides[1].mLight.Color();
				Light[LINDEX(z + 1, y, 0)] = sides[2].mLight.Color();
				Light[LINDEX(z + 1, y, CHUNK_WIDTH + 1)] = sides[3].mLight.Color();
			}

			Block corners[] = {
				mParent->GetBlock(VectorInt3(mPosition.x - 1, y, mPosition.z - 1)),
				mParent->GetBlock(VectorInt3(mPosition.x + CHUNK_WIDTH, y, mPosition.z - 1)),
				mParent->GetBlock(VectorInt3(mPosition.x - 1, y, mPosition.z + CHUNK_WIDTH)),
				mParent->GetBlock(VectorInt3(mPosition.x + CHUNK_WIDTH, y, mPosition.z + CHUNK_WIDTH)),
			};

			Light[LINDEX(0, y, 0)] = corners[0].mLight.Color();
			Light[LINDEX(CHUNK_WIDTH + 1, y, 0)] = corners[1].mLight.Color();
			Light[LINDEX(0, y, CHUNK_WIDTH + 1)] = corners[2].mLight.Color();
			Light[LINDEX(CHUNK_WIDTH + 1, y, CHUNK_WIDTH + 1)] = corners[3].mLight.Color();
		}

		Vector3(*biomecolor)[CHUNK_WIDTH] = (Vector3(*)[CHUNK_WIDTH])malloc(sizeof(Vector3) * CHUNK_SQRWIDTH);
		for (int32 z(0); z < CHUNK_WIDTH; z++)
		{
			for (int32 x(0); x < CHUNK_WIDTH; x++)
			{
				biomecolor[z][x] = BiomeGenerator::getBiomeColor(Vector3(float(mPosition.x + x), 0, float(mPosition.z + z)));
			}
		}

		DWORD	 ShadowMeshCount = 0;
		Vector3  fPosition;
		for (int y(mLoadedHeight - 1); y >= 0; y--)
		{
			if ((y + 1 < mLightHeight) && (ShadowMeshCount == 0))
			{
				ShadowMeshCount = (DWORD)Mesh.size();
			}

			fPosition.y = (FLOAT)(y);
			for (int z(0); z < CHUNK_WIDTH; z++)
			{
				fPosition.z = (FLOAT)(z);
				for (int x(0); x < CHUNK_WIDTH; x++)
				{
					fPosition.x = (FLOAT)(x);
					Block block = mBlocks[y][z][x];

					if ((block.mBlocktype == Blocktype::Air) || (block.mBlocktype == Blocktype::Void))
						continue;
					else if (cull[y][z][x] > 0)
					{
						mVisibleHeight = max(y + 1, mVisibleHeight);
						block.GetMesh(fPosition, biomecolor[z][x], cull[y][z][x], Mesh);
					}
				}
			}
		}
		uiShadowSize = ShadowMeshCount;
		free(biomecolor);
		free(cullDirect);
	}
}

void Chunk::SetLight(std::queue<LightNode> & lightBfsQueue)
{
#define SPLIT_INDEX(i) \
	uint32 x = (i & CHUNK_WIDTH_MASK); \
	uint32 z = (i / CHUNK_WIDTH) & CHUNK_WIDTH_MASK; \
	uint32 y = (i / CHUNK_SQRWIDTH);

#define GET_INDEX(x,y,z) \
	((y) * CHUNK_SQRWIDTH + (z) * CHUNK_WIDTH + (x))

	while (lightBfsQueue.empty() == false)
	{
		// Get a reference to the front node. 
		LightNode node = lightBfsQueue.front();

		Light light = node.chunk->mBlocksDirect[node.index].mLight;
		light -= node.chunk->mBlocksDirect[node.index].mLDV + 1;
		// Pop the front node off the queue. We no longer need the node reference
		lightBfsQueue.pop();

		if (light.rgbs == 0) continue;

		SPLIT_INDEX(node.index);

		auto check = [&](uint32 index, std::shared_ptr<Chunk> chunk)
		{
			Block & block = chunk->mBlocksDirect[index];

			if (block.mOpaque == false && light > block.mLight)
			{
				// Set its light level
				block.mLight |= light;

				// Emplace new node to queue. (could use push as well)
				lightBfsQueue.emplace(index, chunk);
			}
		};

		// Positiv-Y
		if (y < (uint32)node.chunk->mLoadedHeight - 2)
			check(GET_INDEX(x, y + 1, z), node.chunk);
		// Negativ-X
		if (x == 0)
			check(GET_INDEX(CHUNK_WIDTH - 1, y, z), mParent->GetChunk(VectorInt3(mPosition.x - CHUNK_WIDTH, 0, mPosition.z)));
		else
			check(GET_INDEX(x - 1, y, z), node.chunk);
		// Positiv-X
		if (x == CHUNK_WIDTH - 1)
			check(GET_INDEX(0, y, z), mParent->GetChunk(VectorInt3(mPosition.x + CHUNK_WIDTH, 0, mPosition.z)));
		else
			check(GET_INDEX(x + 1, y, z), node.chunk);

		// Negativ-z
		if (z == 0)
			check(GET_INDEX(x, y, CHUNK_WIDTH - 1), mParent->GetChunk(VectorInt3(mPosition.x, 0, mPosition.z - CHUNK_WIDTH)));
		else
			check(GET_INDEX(x, y, z - 1), node.chunk);
		// Positiv-z
		if (z == CHUNK_WIDTH - 1)
			check(GET_INDEX(x, y, 0), mParent->GetChunk(VectorInt3(mPosition.x, 0, mPosition.z + CHUNK_WIDTH)));
		else
			check(GET_INDEX(x, y, z + 1), node.chunk);

		// Negativ-Y
		if (y > 0)
		{
			if (light.s == 14)
				light.s = 15;
			check(GET_INDEX(x, y - 1, z), node.chunk);
		}
	}
}

void Chunk::RemoveLight(std::queue <LightRemovalNode> & lightRemovalBfsQueue)
{
	std::queue<LightNode> lightBfsQueue;

	while (lightRemovalBfsQueue.empty() == false)
	{
		// Get a reference to the front node
		LightRemovalNode node = lightRemovalBfsQueue.front();

		uint32 index = node.index;
		Light lightLevel = node.val;

		// Pop the front node off the queue.
		lightRemovalBfsQueue.pop();

		SPLIT_INDEX(index);

		auto check = [&](uint32 index, std::shared_ptr<Chunk> chunk, bool top=false)
		{
			Block& block = chunk->mBlocksDirect[index];
			Light neighborLevel = block.mLight - block.mLDV;
			if (neighborLevel.rgbs > 0)
			{
				if ((lightLevel > neighborLevel) || ((neighborLevel.s == 15) && top))
				{
					// Set its light level
					if (block.mLightsource)
					{
						block.Enlight();
						lightBfsQueue.emplace(index, chunk);
					}
					else
					{
						block.mLight = 0;
					}

					// Emplace new node to queue. (could use push as well)
					lightRemovalBfsQueue.emplace(index, neighborLevel, chunk);
				}
				else if (neighborLevel >= lightLevel)
				{
					// Add it to the update queue, so it can propagate to fill in the gaps
					// left behind by this removal. We should update the lightBfsQueue after
					// the lightRemovalBfsQueue is empty.
					lightBfsQueue.emplace(index, chunk);
				}
			}
		};

		// Positiv-Y
		if (y < (uint32)node.chunk->mLoadedHeight - 1)
			check(GET_INDEX(x, y + 1, z), node.chunk);
		// Negativ-X
		if (x == 0)
			check(GET_INDEX(CHUNK_WIDTH - 1, y, z), mParent->GetChunk(VectorInt3(mPosition.x - CHUNK_WIDTH, 0, mPosition.z)));
		else
			check(GET_INDEX(x - 1, y, z), node.chunk);
		// Positiv-X
		if (x == CHUNK_WIDTH - 1)
			check(GET_INDEX(0, y, z), mParent->GetChunk(VectorInt3(mPosition.x + CHUNK_WIDTH, 0, mPosition.z)));
		else
			check(GET_INDEX(x + 1, y, z), node.chunk);
		// Negativ-z
		if (z == 0)
			check(GET_INDEX(x, y, CHUNK_WIDTH - 1), mParent->GetChunk(VectorInt3(mPosition.x, 0, mPosition.z - CHUNK_WIDTH)));
		else
			check(GET_INDEX(x, y, z - 1), node.chunk);
		// Positiv-z
		if (z == CHUNK_WIDTH - 1)
			check(GET_INDEX(x, y, 0), mParent->GetChunk(VectorInt3(mPosition.x, 0, mPosition.z + CHUNK_WIDTH)));
		else
			check(GET_INDEX(x, y, z + 1), node.chunk);
		// Negativ-Y
		if (y > 0)
			check(GET_INDEX(x, y - 1, z), node.chunk, true);
	}

	SetLight(lightBfsQueue);
}

void Chunk::SetLight(const VectorInt3& position, const Light light)
{
	std::shared_ptr<Chunk> This = shared_from_this();

	std::queue<LightNode> lightBfsQueue;
	uint32 index = (uint32)(position.y & CHUNK_HEIGHT_MASK) * CHUNK_SQRWIDTH + 
				   (uint32)(position.z & CHUNK_WIDTH_MASK) * CHUNK_WIDTH + 
				   (uint32)(position.x & CHUNK_WIDTH_MASK);
	lightBfsQueue.emplace(index, This);

	SetLight(lightBfsQueue);
}

void Chunk::RemoveLight(const VectorInt3& position, const Light light)
{
	std::shared_ptr<Chunk> This = shared_from_this();

	std::queue <LightRemovalNode> lightRemovalBfsQueue;
	uint32 index = (uint32)(position.y & CHUNK_HEIGHT_MASK) * CHUNK_SQRWIDTH +
				   (uint32)(position.z & CHUNK_WIDTH_MASK) * CHUNK_WIDTH +
				   (uint32)(position.x & CHUNK_WIDTH_MASK);

	if (light.rgbs)
	{
		lightRemovalBfsQueue.emplace(index, light, This);

		mBlocksDirect[index].mLight = 0;
	
		RemoveLight(lightRemovalBfsQueue);
	}
}

void Chunk::GenerateHeight()
{
	if (mHeight == nullptr)
	{
		mHeight = (uint16(*)[CHUNK_WIDTH + 2])malloc((CHUNK_WIDTH + 2) * (CHUNK_WIDTH + 2) * sizeof(uint16));

		for (int32 x = 0; x < CHUNK_WIDTH; x++)
		{
			for (int32 z = 0; z < CHUNK_WIDTH; z++)
			{
				for (int32 y = mLoadedHeight - 1; y >= 0; y--)
				{
					if (mBlocks[y][z][x].mOpaque)
					{
						mHeight[z + 1][x + 1] = (uint16)y;
						break;
					}
				}
			}
		}
	}
}

void Chunk::PlaceSkyLight()
{
	mHeight = (uint16(*)[CHUNK_WIDTH +2])malloc((CHUNK_WIDTH + 2) * (CHUNK_WIDTH + 2) * sizeof(uint16));

	for (int32 x = 0; x < CHUNK_WIDTH; x++)
	{
		for (int32 z = 0; z < CHUNK_WIDTH; z++)
		{
			uint16 light = 15;
			for (int32 y = mLoadedHeight - 1; y >= 0; y--)
			{
				Block & block = mBlocks[y][z][x];

				if (light < 15) light--;

				if (block.mOpaque == false)
				{
					light = max(light - block.mLDV, 0);
					block.mLight.s = light;
				}
				else
				{
					light = 0;
				}

				if (light <= 0)
				{
					mHeight[z + 1][x + 1] = (uint16)y;
					break;
				}
			}
		}
	}
}

void Chunk::PropagateSkyLight()
{
	std::queue<LightNode> lightBfsQueue;
	std::shared_ptr<Chunk> This = shared_from_this();
	std::shared_ptr<Chunk> sides[] = {
		mParent->GetChunk(VectorInt3(mPosition.x - CHUNK_WIDTH, mPosition.y, mPosition.z)),
		mParent->GetChunk(VectorInt3(mPosition.x + CHUNK_WIDTH, mPosition.y, mPosition.z)),
		mParent->GetChunk(VectorInt3(mPosition.x, mPosition.y, mPosition.z - CHUNK_WIDTH)),
		mParent->GetChunk(VectorInt3(mPosition.x, mPosition.y, mPosition.z + CHUNK_WIDTH)),
	};

	for (int32 i = 0; i < CHUNK_WIDTH; i++)
	{
		int16 light = 15;
		for (int32 y = sides[0]->mLoadedHeight - 1; y >= 0; y--)
		{
			Block & block = sides[0]->mBlocks[y][i][CHUNK_WIDTH - 1];

			if (light < 15) light--;

			if (block.mOpaque == false)
				light = max(light - block.mLDV, 0);
			else
				light = 0;

			if (light <= 0)
			{
				mHeight[i + 1][0] = (uint16)y;
				break;
			}
		}
		light = 15;
		for (int32 y = sides[1]->mLoadedHeight - 1; y >= 0; y--)
		{
			Block & block = sides[1]->mBlocks[y][i][0];

			if (light < 15) light--;

			if (block.mOpaque == false)
				light = max(light - block.mLDV, 0);
			else
				light = 0;

			if (light <= 0)
			{
				mHeight[i + 1][CHUNK_WIDTH + 1] = (uint16)y;
				break;
			}
		}
		light = 15;
		for (int32 y = sides[2]->mLoadedHeight - 1; y >= 0; y--)
		{
			Block & block = sides[2]->mBlocks[y][CHUNK_WIDTH - 1][i];

			if (light < 15) light--;

			if (block.mOpaque == false)
				light = max(light - block.mLDV, 0);
			else
				light = 0;

			if (light <= 0)
			{
				mHeight[0][i + 1] = (uint16)y;
				break;
			}
		}
		light = 15;
		for (int32 y = sides[3]->mLoadedHeight - 1; y >= 0; y--)
		{
			Block & block = sides[3]->mBlocks[y][0][i];

			if (light < 15) light--;

			if (block.mOpaque == false)
				light = max(light - block.mLDV, 0);
			else
				light = 0;

			if (light <= 0)
			{
				mHeight[CHUNK_WIDTH + 1][i + 1] = (uint16)y;
				break;
			}
		}
	}
	uint16 lightheight = mLoadedHeight;
	for (uint32 x = 0; x < CHUNK_WIDTH; x++)
	{
		for (uint32 z = 0; z < CHUNK_WIDTH; z++)
		{
			uint16 heights[] =
			{
				mHeight[z][x + 1],
				mHeight[z + 1][x],
				mHeight[z + 1][x + 2],
				mHeight[z + 2][x + 1],
			};
			uint16 maxH = max(max(heights[0], heights[1]), max(heights[2], heights[3]));
			uint16 curH = mHeight[z + 1][x + 1] + 1;
			lightheight = min(lightheight, curH);
			if (maxH > curH)
				for (uint32 y = curH; y < maxH; y++)
					lightBfsQueue.emplace(((y)* CHUNK_SQRWIDTH + (z)* CHUNK_WIDTH + (x)), This);
		}
	}
	free(mHeight);
	mHeight = nullptr;
	mLightHeight = lightheight - 1;

	SetLight(lightBfsQueue);

	//for (uint32 x = 0; x < CHUNK_WIDTH; x++)
	//{
	//	for (uint32 z = 0; z < CHUNK_WIDTH; z++)
	//	{
	//		SetBlock(mPosition + VectorInt3(x, 200, z), Block::Sand);
	//	}
	//}
}
