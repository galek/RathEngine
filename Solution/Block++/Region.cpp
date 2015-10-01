#include "stdafx.h"
#include "Region.h"
#include "World.h"
#include "WorldGenerator.h"

#include <FileIO.h>
#include <zlib.h>

#define IN_REGION(p) (((((p.x ^ mPosition.x) | (p.z ^ mPosition.z)) & REGION_WIDTH_INVMASK) | ((p.y ^ mPosition.y) & CHUNK_HEIGHT_INVMASK)) == 0)

#define INDEX(x,z) \
	uint32((x) + (z) * REGION_WIDTH)

#define POSITION(p) \
	uint32 x = uint32(p.x & REGION_WIDTH_MASK) / CHUNK_WIDTH; \
	uint32 z = uint32(p.z & REGION_WIDTH_MASK) / CHUNK_WIDTH; \
	uint32 index = INDEX(x, z);

Region::Region(World* parent, const VectorInt3& position) :
mParent(parent),
mPosition(position)
{
	memset(mChunks, 0, sizeof(mChunks));
	memset(mChunkStatus, 0, sizeof(mChunkStatus));
	memset(mEntitiesInChunk, 0, sizeof(mEntitiesInChunk));

	VectorInt3 Position = VectorInt3(
		mPosition.x / REGION_REALWIDTH,
		mPosition.y / CHUNK_HEIGHT,
		mPosition.z / REGION_REALWIDTH);
	WCHAR filename[_MAX_PATH];
	uint64 hash = std::hash<XMINT3>()(Position);
	swprintf_s(filename, _MAX_PATH, L"Data\\Region%llX.rgn", hash);

	mFilelock.lock();
	if (Rath::FileExists(filename))
	{
		errno_t err = _wfopen_s(&mSavefile, filename, L"rb+");
		if (err == 0)
		{
			fread((char*)&mSavefileHeader, sizeof(char), sizeof(Header), mSavefile);
		}
		else
			throw err;
	}
	else
	{
		errno_t err = _wfopen_s(&mSavefile, filename, L"wb+");
		if (err == 0) 
		{
			memset(&mSavefileHeader, 0, sizeof(Header));
			mSavefileHeader.Size = sizeof(Header);
			fwrite((char*)&mSavefileHeader, sizeof(char), sizeof(Header), mSavefile);
			fflush(mSavefile);
		}
		else
			throw err;
	}
	mFilelock.unlock();
}


Region::~Region()
{
	for (size_t i = 0; i < REGION_SQRWIDTH; i++)
		mChunks[i] = nullptr;

	mFilelock.lock();
	fseek(mSavefile, 0, SEEK_SET);
	fwrite((char*)&mSavefileHeader, sizeof(char), sizeof(Header), mSavefile);
	fclose(mSavefile);
	mFilelock.unlock();

	for (auto& it : mEntities)
	{
		it.Node->Release();
	}
}

std::shared_ptr<Chunk> Region::GetChunk(const VectorInt3& position)
{
	if (IN_REGION(position))
	{
		POSITION(position);

		std::shared_ptr<Chunk> chunk = mChunks[index];

		if (chunk == nullptr)
		{
			Load(index);
			chunk = mChunks[index];
		}

		assert(chunk != nullptr);
		return chunk;
	}
	else
	{
		return mParent->GetChunk(position);
	}
}

Block Region::GetBlock(const VectorInt3& position) const
{
	if (IN_REGION(position))
	{
		POSITION(position);

		if (mChunks[index] != nullptr)
			return mChunks[index]->GetBlock(position);
		else
			return Block::Void;
	}
	else
	{
		return mParent->GetBlock(position);
	}
}

void  Region::SetBlock(const VectorInt3& position, const Block& block, Blockupdate flag)
{
	if (IN_REGION(position))
	{
		POSITION(position);

		if (mChunks[index] == nullptr)
		{
			Load(index);
		}

		mChunks[index]->SetBlock(position, block, flag);
		mChunkStatus[index] = ((mChunkStatus[index] | Changed) & ~Visible);
	}
	else
	{
		mParent->SetBlock(position, block, flag);
	}
}

void Region::ReleaseResources()
{
	for (uint32 i = 0; i < REGION_SQRWIDTH; i++)
	{
		mChunkStatus[i] &= ~Visible;
	}
}

void Region::CreateResources(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pImmediateContext)
{
	for (uint32 i = 0; i < REGION_SQRWIDTH; i++)
		if (mChunks[i] != nullptr && 
			((mChunkStatus[i] & COMPILETIME_OR_2FLAGS(ReadyForDisplay, Visible)) == ReadyForDisplay))
		{
			mChunkStatus[i] |= Visible;
			mChunks[i]->Create(pd3dDevice, pImmediateContext);
		}
}

uint32 Region::GetAreaStatus(uint32 x, uint32 z)
{
	uint32 Status = mChunkStatus[INDEX(x,z)];

	if (x == 0)
	{
		std::shared_ptr<Region> reg = mParent->GetRegion(VectorInt3(mPosition.x - REGION_REALWIDTH, 0, mPosition.z));
		Status &= (reg == nullptr) ? 0 : reg->mChunkStatus[INDEX(REGION_WIDTH - 1,z)];
		Status &= mChunkStatus[INDEX(x+1, z)];
	}
	else if (x == REGION_WIDTH - 1)
	{
		std::shared_ptr<Region> reg = mParent->GetRegion(VectorInt3(mPosition.x + REGION_REALWIDTH, 0, mPosition.z));
		Status &= (reg == nullptr) ? 0 : reg->mChunkStatus[INDEX(0, z)];
		Status &= mChunkStatus[INDEX(x-1, z)];
	}
	else
	{
		Status &= mChunkStatus[INDEX(x-1, z)];
		Status &= mChunkStatus[INDEX(x+1, z)];
	}

	if (z == 0)
	{
		std::shared_ptr<Region> reg = mParent->GetRegion(VectorInt3(mPosition.x, 0, mPosition.z - REGION_REALWIDTH));
		Status &= (reg == nullptr) ? 0 : reg->mChunkStatus[INDEX(x,REGION_WIDTH - 1)];
		Status &= mChunkStatus[INDEX(x, z+1)];
	}
	else if (z == REGION_WIDTH - 1)
	{
		std::shared_ptr<Region> reg = mParent->GetRegion(VectorInt3(mPosition.x, 0, mPosition.z + REGION_REALWIDTH));
		Status &= (reg == nullptr) ? 0 : reg->mChunkStatus[INDEX(x, 0)];
		Status &= mChunkStatus[INDEX(x, z-1)];
	}
	else
	{
		Status &= mChunkStatus[INDEX(x, z-1)];
		Status &= mChunkStatus[INDEX(x, z+1)];
	}

	return Status;
}

uint32 Region::LoadInRange(const XMBOUNDINGBOX& range)
{
	uint32 changes = 0;

	std::function<void(const XMBOUNDINGBOX&, uint32, uint32, uint32)> LoadInRangeSub
		= [&](const XMBOUNDINGBOX& caabb, uint32 depth, uint32 x, uint32 z)
	{
		if (depth == 0)
		{
			uint32 index = INDEX(x, z);
			uint8 oldstatus = mChunkStatus[index];
			std::shared_ptr<Chunk> chunk = mChunks[index];
			if (chunk != nullptr)
			{
				//mFilelock.lock();
				if ((mChunkStatus[index] & 0x0F) == Loaded)// && (GetAreaStatus(x, z) & Loaded) == Loaded)
				{
					WorldGenerator::PopulateChunk(chunk.get());

					mChunkStatus[index] |= COMPILETIME_OR_2FLAGS(Generated, Changed);
				}			
				//mFilelock.unlock();
			}
			else
			{
				Load(index);	
			}

			if (((mChunkStatus[index] & ReadyForDisplay) == 0) && 
				(GetAreaStatus(x, z) & Generated) == Generated)
				mChunkStatus[index] |= ReadyForDisplay;

			if (oldstatus != mChunkStatus[index]) 
				changes += 1;

		}
		else
		{
			XMBOUNDINGBOX aabb = caabb;
			aabb.Extents *= XMVectorSet(0.5f, 1.0f, 0.5f, 1.0f);
			XMBOUNDINGBOX bb[] =
			{
				XMBOUNDINGBOX(aabb.Center + (aabb.Extents * XMVectorSet(-1.0f, 0.0f, -1.0f, 1.0f)), aabb.Extents),
				XMBOUNDINGBOX(aabb.Center + (aabb.Extents * XMVectorSet(-1.0f, 0.0f, 1.0f, 1.0f)), aabb.Extents),
				XMBOUNDINGBOX(aabb.Center + (aabb.Extents * XMVectorSet(1.0f, 0.0f, -1.0f, 1.0f)), aabb.Extents),
				XMBOUNDINGBOX(aabb.Center + (aabb.Extents * XMVectorSet(1.0f, 0.0f, 1.0f, 1.0f)), aabb.Extents),
			};
			for (uint32 k(0); k < 4; k++) if (range.Intersects(bb[k]))
			{
				uint32 xs = x + (k / 2) * depth;
				uint32 zs = z + (k % 2) * depth;

				LoadInRangeSub(bb[k], depth / 2, xs, zs);
			}
		}
	};

	XMVECTOR extents = { REGION_REALWIDTH / 2.0f, 0, REGION_REALWIDTH / 2.0f, 0.f };
	XMVECTOR center = XMLoadSInt3(&mPosition) + extents;
	XMBOUNDINGBOX aabb(center, extents);

	if (range.Intersects(aabb))
	{
		LoadInRangeSub(aabb, REGION_WIDTH / 2, 0, 0);
	}
	else
	{
		changes = 0;
	}
	return changes;
}

int32 Region::ClearOutOfRange(const XMBOUNDINGBOX& range)
{
	std::function<void(const XMBOUNDINGBOX&, uint32, uint32, uint32)> ClearOutOfRangeSub
		= [&](const XMBOUNDINGBOX& caabb, uint32 depth, uint32 x, uint32 z)
	{
		if (depth > 0)
		{
			XMBOUNDINGBOX aabb = caabb;
			aabb.Extents *= XMVectorSet(0.5f, 1.0f, 0.5f, 1.0f);
			XMBOUNDINGBOX bb[] =
			{
				XMBOUNDINGBOX(aabb.Center + (aabb.Extents * XMVectorSet(-1.0f, 0.0f, -1.0f, 1.0f)), aabb.Extents),
				XMBOUNDINGBOX(aabb.Center + (aabb.Extents * XMVectorSet(-1.0f, 0.0f, 1.0f, 1.0f)), aabb.Extents),
				XMBOUNDINGBOX(aabb.Center + (aabb.Extents * XMVectorSet(1.0f, 0.0f, -1.0f, 1.0f)), aabb.Extents),
				XMBOUNDINGBOX(aabb.Center + (aabb.Extents * XMVectorSet(1.0f, 0.0f, 1.0f, 1.0f)), aabb.Extents),
			};
			for (uint32 k(0); k < 4; k++)
			{
				uint32 xs = x + (k / 2) * depth;
				uint32 zs = z + (k % 2) * depth;

				switch (range.Contains(bb[k]))
				{
				case DISJOINT:
					for (uint32 x(xs); x < xs + depth; x++)
						for (uint32 z(zs); z < zs + depth; z++)
						{
							int32 index = INDEX(x, z);
							//Save(index);
							mChunks[index] = nullptr;
						}
					break;
				case INTERSECTS:
					ClearOutOfRangeSub(bb[k], depth / 2, xs, zs);
					break;
				case CONTAINS:
					break;
				}
			}
		}
	};

	XMVECTOR extents = { REGION_REALWIDTH / 2.0f, 0, REGION_REALWIDTH / 2.0f, 0.f };
	XMVECTOR center = XMLoadSInt3(&mPosition) + extents;
	XMBOUNDINGBOX aabb(center, extents);

	int32 result = 0;
	switch (range.Contains(aabb))
	{
	case DISJOINT:
		result = -1;
		break;
	case INTERSECTS:
		ClearOutOfRangeSub(aabb, REGION_WIDTH / 2, 0, 0);
		break;
	case CONTAINS:
		result = 1;
		break;
	}
	return result;
}

void Region::VisibleChunks(const XMFRUSTUM& frustum, uint32 levelOfDetail, std::vector<std::shared_ptr<Chunk>>* chunks, DirectX::XMMATRIXLIST* entities)
{
	std::function<void(const XMBOUNDINGBOX&, uint32, uint32, uint32)> VisibleChunksSub = [&](const XMBOUNDINGBOX& cboundingbox, uint32 depth, uint32 x, uint32 z)
	{
		XMBOUNDINGBOX boundingbox = cboundingbox;
		if (depth == 0)
		{
			uint32 index = INDEX(x, z);
			if (mChunks[index])
			{
				FLOAT heightmax = FLOAT(mChunks[index]->mVisibleHeight);
				FLOAT heightmin = levelOfDetail > 0 ? 0.0f : FLOAT(mChunks[index]->mLightHeight);
				FLOAT center = (heightmax + heightmin) / 2.0f;
				FLOAT extents = heightmax - center;
				boundingbox.Center = XMVectorSetY(boundingbox.Center, center);
				boundingbox.Extents = XMVectorSetY(boundingbox.Extents, extents);

				if (frustum.Intersects(boundingbox))
				{
					chunks->emplace_back(mChunks[index]);

					if (entities)
					{
						uint32 i = 0;
						while (i < mEntitiesInChunk[index][1])
						{
							entities->emplace_back(XMMatrixTranspose(mEntities[mEntitiesInChunk[index][0] + i].Node->GetWorld()));
							i++;
						}
					}
				}
			}
		}
		else
		{
			if (frustum.Intersects(boundingbox))
			{
				boundingbox.Extents *= XMVectorSet(0.5f, 1.0f, 0.5f, 1.0f);
				XMBOUNDINGBOX bb[] =
				{
					XMBOUNDINGBOX(boundingbox.Center + (boundingbox.Extents * XMVectorSet(-1.0f, 0.0f, -1.0f, 1.0f)), boundingbox.Extents),
					XMBOUNDINGBOX(boundingbox.Center + (boundingbox.Extents * XMVectorSet(-1.0f, 0.0f, 1.0f, 1.0f)), boundingbox.Extents),
					XMBOUNDINGBOX(boundingbox.Center + (boundingbox.Extents * XMVectorSet(1.0f, 0.0f, -1.0f, 1.0f)), boundingbox.Extents),
					XMBOUNDINGBOX(boundingbox.Center + (boundingbox.Extents * XMVectorSet(1.0f, 0.0f, 1.0f, 1.0f)), boundingbox.Extents),
				};
				for (uint32 k(0); k < 4; k++)
				{
					uint32 xs = x + (k / 2) * depth;
					uint32 zs = z + (k % 2) * depth;

					VisibleChunksSub(bb[k], depth / 2, xs, zs);
				}
			}
		}
	};

	XMVECTOR extents = { REGION_REALWIDTH / 2.0f, REGION_REALWIDTH / 2.0f, REGION_REALWIDTH / 2.0f, 0.f};
	XMVECTOR center = XMLoadSInt3(&mPosition) + extents;
	XMBOUNDINGBOX bb(center, extents);

	VisibleChunksSub(bb, REGION_WIDTH / 2, 0, 0);
}

void Region::VisibleChunks(const XMFRUSTUM& frustum, uint32 levelOfDetail, std::vector<std::tuple<std::shared_ptr<Chunk>, DirectX::XMMATRIXLIST*>>* chunks)
{
	std::function<void(const XMBOUNDINGBOX&, uint32, uint32, uint32)> VisibleChunksSub = [&](const XMBOUNDINGBOX& cboundingbox, uint32 depth, uint32 x, uint32 z)
	{
		XMBOUNDINGBOX boundingbox = cboundingbox;
		if (depth == 0)
		{
			uint32 index = INDEX(x, z);
			if (mChunks[index])
			{
				FLOAT heightmax = FLOAT(mChunks[index]->mVisibleHeight);
				FLOAT heightmin = levelOfDetail > 0 ? 0.0f : FLOAT(mChunks[index]->mLightHeight);
				FLOAT center = (heightmax + heightmin) / 2.0f;
				FLOAT extents = heightmax - center;
				boundingbox.Center = XMVectorSetY(boundingbox.Center, center);
				boundingbox.Extents = XMVectorSetY(boundingbox.Extents, extents);

				if (frustum.Intersects(boundingbox))
				{
					DirectX::XMMATRIXLIST* entities = nullptr;
					if (mEntitiesInChunk[index][1] > 0)
					{
						entities = new DirectX::XMMATRIXLIST;
						uint32 i = 0;
						while (i < mEntitiesInChunk[index][1])
						{
							entities->emplace_back(XMMatrixTranspose(mEntities[mEntitiesInChunk[index][0] + i].Node->GetWorld()));
							i++;
						}
					}
					chunks->emplace_back(mChunks[index], entities);
				}
			}
		}
		else
		{
			if (frustum.Intersects(boundingbox))
			{
				boundingbox.Extents *= XMVectorSet(0.5f, 1.0f, 0.5f, 1.0f);
				XMBOUNDINGBOX bb[] =
				{
					XMBOUNDINGBOX(boundingbox.Center + (boundingbox.Extents * XMVectorSet(-1.0f, 0.0f, -1.0f, 1.0f)), boundingbox.Extents),
					XMBOUNDINGBOX(boundingbox.Center + (boundingbox.Extents * XMVectorSet(-1.0f, 0.0f, 1.0f, 1.0f)), boundingbox.Extents),
					XMBOUNDINGBOX(boundingbox.Center + (boundingbox.Extents * XMVectorSet(1.0f, 0.0f, -1.0f, 1.0f)), boundingbox.Extents),
					XMBOUNDINGBOX(boundingbox.Center + (boundingbox.Extents * XMVectorSet(1.0f, 0.0f, 1.0f, 1.0f)), boundingbox.Extents),
				};
				for (uint32 k(0); k < 4; k++)
				{
					uint32 xs = x + (k / 2) * depth;
					uint32 zs = z + (k % 2) * depth;

					VisibleChunksSub(bb[k], depth / 2, xs, zs);
				}
			}
		}
	};

	XMVECTOR extents = { REGION_REALWIDTH / 2.0f, REGION_REALWIDTH / 2.0f, REGION_REALWIDTH / 2.0f, 0.f };
	XMVECTOR center = XMLoadSInt3(&mPosition) + extents;
	XMBOUNDINGBOX bb(center, extents);

	VisibleChunksSub(bb, REGION_WIDTH / 2, 0, 0);
}

void Region::Save(uint32 index, Chunk* pChunk)
{
	mFilelock.lock();
	if (pChunk != nullptr)
	{
		if ((mChunkStatus[index] & COMPILETIME_OR_2FLAGS(ReadyForDisplay, Changed)) == COMPILETIME_OR_2FLAGS(ReadyForDisplay, Changed))
		{
			auto writeChunk = [&](void)
			{
				uint32 offset = mSavefileHeader.Size;
				uint8 status = (mChunkStatus[index] & 0x0F);
				uint32 UnCompressedSize = sizeof(Block) * CHUNK_SQRWIDTH * pChunk->mLoadedHeight;
				uLong CompressedSpace = uLong(UnCompressedSize + sizeof(Block) * CHUNK_SQRWIDTH);
				Bytef* CompressedSpaceBuffer = (Bytef*)malloc(CompressedSpace);

				int result = compress(CompressedSpaceBuffer, &CompressedSpace, (Bytef*)pChunk->mBlocksDirect, (uLong)UnCompressedSize);
				assert(result == Z_OK);

				UnCompressedSize = (uint32)CompressedSpace;

				fseek(mSavefile, offset, SEEK_SET);
				fwrite((const char*)&pChunk->mLoadedHeight, sizeof(char), sizeof(Chunk::mLoadedHeight), mSavefile);
				fwrite((const char*)&pChunk->mVisibleHeight, sizeof(char), sizeof(Chunk::mVisibleHeight), mSavefile);
				fwrite((const char*)&pChunk->mLightHeight, sizeof(char), sizeof(Chunk::mLightHeight), mSavefile);
				fwrite((const char*)&UnCompressedSize, sizeof(char), sizeof(UnCompressedSize), mSavefile);
				fwrite((const char*)CompressedSpaceBuffer, sizeof(char), CompressedSpace, mSavefile);
				fwrite((const char*)&status, sizeof(char), sizeof(status), mSavefile);

				free(CompressedSpaceBuffer);

				uint32 size = sizeof(Chunk::mLoadedHeight) + 
							  sizeof(Chunk::mVisibleHeight) + 
							  sizeof(Chunk::mLightHeight) + 
							  sizeof(UnCompressedSize) +
							  CompressedSpace +
							  sizeof(status) +
							  sizeof(uint32);

				fwrite((const char*)&mEntitiesInChunk[index][1], sizeof(char), sizeof(uint32), mSavefile);
				uint32 i = 0;
				while (i < mEntitiesInChunk[index][1])
				{
					size += mEntities[mEntitiesInChunk[index][0] + i].Node->Save(mSavefile);
					i++;
				}

				fflush(mSavefile);

				mSavefileHeader.Size += size;
				mSavefileHeader.Size -= mSavefileHeader.Records[index].Size;
				mSavefileHeader.Records[index].Position = offset;
				mSavefileHeader.Records[index].Size = size;
			};

			if (mSavefileHeader.Records[index].Size == 0)
			{
				writeChunk();
			}
			else if (0)
			{
				uint32 offset = mSavefileHeader.Records[index].Position;
				uint32 begin = min(offset + mSavefileHeader.Records[index].Size, mSavefileHeader.Size);
				uint32 size = mSavefileHeader.Size - begin;
				
				if (size > 0)
				{
					for (size_t i = 0; i < REGION_SQRWIDTH; i++) 
						if (mSavefileHeader.Records[i].Position > offset)
							mSavefileHeader.Records[i].Position -= mSavefileHeader.Records[index].Size;
					mSavefileHeader.Size -= mSavefileHeader.Records[index].Size;
					mSavefileHeader.Records[index].Size = 0;

					char* buffer = (char*)malloc(size);
					fseek(mSavefile, begin, SEEK_SET);
					fread(buffer, sizeof(char), size, mSavefile);
					fseek(mSavefile, offset, SEEK_SET);
					fwrite(buffer, sizeof(char), size, mSavefile);
					free(buffer);
				}
				writeChunk();
			}
		}
		mChunkStatus[index] = NotLoaded;
		delete pChunk;
	}
	mFilelock.unlock();
}

void Region::Load(uint32 index)
{
	mFilelock.lock();
	if (mChunks[index] == nullptr)
	{
		std::shared_ptr<Chunk> chunk = mChunkBackups[index].lock();
		if (mChunkBackups[index].expired())
		{
			int32 x = (index % REGION_WIDTH) * CHUNK_WIDTH;
			int32 z = (index / REGION_WIDTH) * CHUNK_WIDTH;
			VectorInt3 pos(mPosition.x + x, mPosition.y, mPosition.z + z);

			chunk.reset(new Chunk(this, pos), std::bind(&Region::Save, this, index, std::placeholders::_1));

			if (mSavefileHeader.Records[index].Size > 0)
			{
				uint32 offset = mSavefileHeader.Records[index].Position;
				uint32 CompressedSize = 0;

				fseek(mSavefile, offset, SEEK_SET);
				fread((char*)&chunk->mLoadedHeight, sizeof(char), sizeof(Chunk::mLoadedHeight), mSavefile);
				fread((char*)&chunk->mVisibleHeight, sizeof(char), sizeof(Chunk::mVisibleHeight), mSavefile);
				fread((char*)&chunk->mLightHeight, sizeof(char), sizeof(Chunk::mLightHeight), mSavefile);
				fread((char*)&CompressedSize, sizeof(char), sizeof(CompressedSize), mSavefile);

				uLong CompressedSpace = uLong(sizeof(Block) * CHUNK_SQRWIDTH * (chunk->mLoadedHeight + 1));
				Bytef* CompressedSpaceBuffer = (Bytef*)malloc(CompressedSpace);

				fread((char*)CompressedSpaceBuffer, sizeof(char), CompressedSize, mSavefile);
				fread((char*)&mChunkStatus[index], sizeof(char), sizeof(uint8), mSavefile);

				uLong UncompressedSpace = (uLong)sizeof(Block) * CHUNK_SQRWIDTH * chunk->mLoadedHeight;
				chunk->mBlocksDirect = (Block*)malloc(UncompressedSpace);

				int result = uncompress((Bytef*)chunk->mBlocksDirect, &UncompressedSpace, CompressedSpaceBuffer, CompressedSpace);
				assert(result == Z_OK);

				free(CompressedSpaceBuffer);

				if (chunk->mLightHeight == CHUNK_HEIGHT)
					chunk->GenerateHeight();

				uint32 i = 0;
				fread((char*)&i, sizeof(char), sizeof(uint32), mSavefile);
				mEntitiesInChunk[index][1] = i;
				while (i-- > 0)
				{
					AddEntity(Factory<Entity>::Instance().Load(mSavefile));
				}
			}
			else
			{
				WorldGenerator::GenerateChunk(chunk.get());

				mChunkStatus[index] = Loaded;
			}
			mChunkBackups[index] = chunk;
		}
		mChunks[index] = chunk;
	}
	mFilelock.unlock();
}

void Region::AddEntity(Entity* node)
{
	VectorInt3 pos = node->GetPosition();
	if (IN_REGION(pos))
	{
		mEntitieslock.lock();
		mEntities.emplace_back(EntityEntry{ node, 0 });
		mEntitieslock.unlock();

		SortEntities();
	}
	else
	{
		mParent->AddEntity(node);
	}

}

void Region::RemoveEntity(Entity* node)
{
	mEntitieslock.lock();
	auto it = std::find(mEntities.begin(), mEntities.end(), node);
	if (it != mEntities.end())
		mEntities.erase(it);
	mEntitieslock.unlock();

	SortEntities();
}

void Region::SortEntities()
{
	mEntitieslock.lock();
	if (mEntities.size() > 0)
	{
		auto it = mEntities.begin();
		while (it != mEntities.end())
		{
			VectorInt3 pos = it->Node->GetPosition();
			if (IN_REGION(pos))
			{
				POSITION(pos);
				it->Index = index;
				it++;
			}
			else
			{
				mParent->AddEntity(it->Node);
				it = mEntities.erase(it);
			}
		}

		std::sort(mEntities.begin(), mEntities.end());
		memset(mEntitiesInChunk, 0, sizeof(mEntitiesInChunk));

		uint32 chunkId = mEntities[0].Index;
		uint32 start = 0;
		uint32 size = 0;
		for (uint32 i = 0; i < (uint32)mEntities.size(); i++)
		{
			if (mEntities[i].Index == chunkId)
			{
				size++;
			}
			else
			{
				mEntitiesInChunk[chunkId][0] = start;
				mEntitiesInChunk[chunkId][1] = size;

				start = i;
				size = 1;
				chunkId = mEntities[i].Index;
			}
		}
		mEntitiesInChunk[chunkId][0] = start;
		mEntitiesInChunk[chunkId][1] = size;
	}
	else
	{
		memset(mEntitiesInChunk, 0, sizeof(mEntitiesInChunk));
	}
	mEntitieslock.unlock();
}