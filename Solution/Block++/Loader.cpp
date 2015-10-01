#include "stdafx.h"
#include "Loader.h"
#include "World.h"

#define UNLOADED 0
#define LOADED 1
#define PRELIGHTED 2
#define LIGHTED 4
#define FINISHED 8

Loader::Loader(World* world, Node* character) :
mParent(world),
mCharacter(character),
mMovedPosition(XMINT3()),
mLoadDistance(32),
mLoadCenter((mLoadDistance & 0x1) + (mLoadDistance >> 1)),
mStartRing((mLoadDistance & 0x1) ^ 0x1),
mIdle(false)
{
	memset(mStatusDirect, 0, sizeof(mStatusDirect));

	XMStoreSInt3(&mCurrentPosition, mCharacter->GetPosition());
	mCurrentPosition = XMINT3(mCurrentPosition.x & CHUNK_WIDTH_INVMASK, 0, mCurrentPosition.z & CHUNK_WIDTH_INVMASK);
	mLastPosition = mCurrentPosition;

	XMINT3 min = XMINT3(mCurrentPosition.x - mLoadDistance * CHUNK_WIDTH / 2, 0, mCurrentPosition.z - mLoadDistance * CHUNK_WIDTH / 2);
	XMINT3 max = XMINT3(mCurrentPosition.x + mLoadDistance * CHUNK_WIDTH / 2, 0, mCurrentPosition.z + mLoadDistance * CHUNK_WIDTH / 2);

	mParent->MultiLoadRegion(min, max);
}

void Loader::UpdatePosition()
{
	XMINT3 current;
	XMStoreSInt3(&current, mCharacter->GetPosition());
	current = XMINT3(current.x & CHUNK_WIDTH_INVMASK, 0, current.z & CHUNK_WIDTH_INVMASK);

	mMovedPosition = XMINT3(
		min(mLoadDistance, UINT((current.x - mLastPosition.x) / CHUNK_WIDTH)),
		0, 
		min(mLoadDistance, UINT((current.z - mLastPosition.z) / CHUNK_WIDTH)));

	if (mMovedPosition.x || mMovedPosition.z)
	{
		mLastPosition = mCurrentPosition;
		mCurrentPosition = current;

		XMINT3 min = XMINT3(current.x - mLoadDistance * CHUNK_WIDTH / 2, 0, current.z - mLoadDistance * CHUNK_WIDTH / 2);
		XMINT3 max = XMINT3(current.x + mLoadDistance * CHUNK_WIDTH / 2, 0, current.z + mLoadDistance * CHUNK_WIDTH / 2);

		mParent->MultiLoadRegion(min, max);

		mIdle = false;
	}
}

void Loader::UpdateStatus()
{
	std::vector<XMINT3> positions;
	if (mMovedPosition.x > 0)
	{
		for (UINT z = 0; z < mLoadDistance; z++)
		{
			for (UINT x = mMovedPosition.x; x < mLoadDistance; x++)
			{
				mStatus[z][x - mMovedPosition.x] = mStatus[z][x];
			}
			for (UINT x = 0; x < UINT(mMovedPosition.x); x++)
			{
				if (mStatus[z][x])
				{
					positions.push_back(XMINT3(mCurrentPosition.x + (int(x) - mLoadCenter) * CHUNK_WIDTH, 0, mCurrentPosition.z + (int(z) - mLoadCenter) * CHUNK_WIDTH));
					mStatus[z][x] = 0;
				}
			}
		}
	}
	else if (mMovedPosition.x < 0)
	{
		for (UINT z = 0; z < mLoadDistance; z++)
		{
			for (UINT x = 0; x < mLoadDistance - mMovedPosition.x; x++)
			{
				mStatus[z][x] = mStatus[z][x + mMovedPosition.x];
			}
			for (UINT x = mLoadDistance - mMovedPosition.x; x < mLoadDistance; x++)
			{
				if (mStatus[z][x])
				{
					positions.push_back(XMINT3(mCurrentPosition.x + (int(x) - mLoadCenter) * CHUNK_WIDTH, 0, mCurrentPosition.z + (int(z) - mLoadCenter) * CHUNK_WIDTH));
					mStatus[z][x] = 0;
				}
			}
		}
	}
	if (mMovedPosition.z > 0)
	{
		for (UINT x = 0; x < mLoadDistance; x++)
		{
			for (UINT z = mMovedPosition.z; z < mLoadDistance; z++)
			{
				mStatus[z - mMovedPosition.z][x] = mStatus[z][x];
			}
			for (UINT z = 0; z < UINT(mMovedPosition.z); z++)
			{
				if (mStatus[z][x])
				{
					positions.push_back(XMINT3(mCurrentPosition.x + (int(x) - mLoadCenter) * CHUNK_WIDTH, 0, mCurrentPosition.z + (int(z) - mLoadCenter) * CHUNK_WIDTH));
					mStatus[z][x] = 0;
				}
			}
		}
	}
	else if (mMovedPosition.z < 0)
	{
		for (UINT x = 0; x < mLoadDistance; x++)
		{
			for (UINT z = 0; z < mLoadDistance - mMovedPosition.z; z++)
			{
				mStatus[z][x] = mStatus[z + mMovedPosition.z][x];
			}
			for (UINT z = mLoadDistance - mMovedPosition.z; z < mLoadDistance; z++)
			{
				if (mStatus[z][x])
				{
					positions.push_back(XMINT3(mCurrentPosition.x + (int(x) - mLoadCenter) * CHUNK_WIDTH, 0, mCurrentPosition.z + (int(z) - mLoadCenter) * CHUNK_WIDTH));
					mStatus[z][x] = 0;
				}
			}
		}
	}

	if (positions.size() > 0)
	{
		mParent->MultiUnLoadChunk(positions, mUnLoadList);
		mReadyUnload = false;
	}
}

UCHAR Loader::GetStatus(size_t ring)
{
	UCHAR status = 0xFF;
	size_t min = mLoadCenter + mStartRing - ring;
	size_t max = mLoadCenter + ring;
	for (size_t x = min; x <= max; x++)
	{
		for (size_t z = min; z <= max; z++)
		{
			if (x == min || x == max || z == min || z == max)
			{
				status = min(status, mStatus[z][x]);
			}
		}
	}
	return status;
}

void Loader::Load(size_t ring)
{
	int min = mStartRing - int(ring);
	int max = int(ring);
	std::vector<XMINT3> positions;
	for (int x = min; x <= max; x++)
	{
		for (int z = min; z <= max; z++)
		{
			if (x == min || x == max || z == min || z == max)
			{
				positions.push_back(XMINT3(mCurrentPosition.x + x * CHUNK_WIDTH, 0, mCurrentPosition.z + z * CHUNK_WIDTH));
				mStatus[mLoadCenter + z][mLoadCenter + x] = PRELIGHTED;
			}
		}
	}
	mParent->MultiLoadChunk(positions);
}

//void Loader::LightPass(size_t ring)
//{
//	int min = mStartRing - int(ring);
//	int max = int(ring);
//	std::vector<XMINT3> positions;
//	for (int x = min; x <= max; x++)
//	{
//		for (int z = min; z <= max; z++)
//		{
//			if (x == min || x == max || z == min || z == max)
//			{
//				if (mStatus[mLoadCenter + z][mLoadCenter + x] == PRELIGHTED)
//				{
//					positions.push_back(XMINT3(mCurrentPosition.x + x * CHUNK_WIDTH, 0, mCurrentPosition.z + z * CHUNK_WIDTH));
//					mStatus[mLoadCenter + z][mLoadCenter + x] = LIGHTED;
//				}
//				else if (mStatus[mLoadCenter + z][mLoadCenter + x] == LIGHTED)
//				{
//					positions.push_back(XMINT3(mCurrentPosition.x + x * CHUNK_WIDTH, 0, mCurrentPosition.z + z * CHUNK_WIDTH));
//					mStatus[mLoadCenter + z][mLoadCenter + x] = FINISHED;
//				}
//			}
//		}
//	}
//	mParent->MultiCalculateLight(positions);
//}

void Loader::LightPass(size_t ring)
{
	size_t min = mLoadCenter + mStartRing - ring;
	size_t max = mLoadCenter + ring;
	std::vector<XMINT3> positions;

	for (size_t x = min; x <= max; x++)
	{
		for (size_t z = min; z <= max; z++)
		{
			if (mStatus[z][x] < FINISHED)
			{
				UCHAR Status = min(mStatus[z][x + 1], mStatus[z][x - 1]);
				Status = min(Status, mStatus[z + 1][x]);
				Status = min(Status, mStatus[z - 1][x]);
				if (Status >= mStatus[z][x])
				{
					positions.push_back(XMINT3(mCurrentPosition.x + int(x - mLoadCenter) * CHUNK_WIDTH, 0, mCurrentPosition.z + int(z - mLoadCenter) * CHUNK_WIDTH));
					mStatus[z][x] *= 2;
				}
			}
		}
	}

	mParent->MultiCalculateLight(positions);
}

void Loader::Work()
{
	UpdatePosition();

	if (!mIdle)
	{
		UpdateStatus();

		UCHAR* pStatus = (UCHAR*)calloc(mLoadCenter + 2, sizeof(UCHAR));
		pStatus[0] = 0xFF;
		for (UINT ring(mStartRing), i(1); ring <= mLoadCenter; ring++, i++)
		{
			pStatus[i] = GetStatus(ring);
		}

		if (std::all_of(pStatus, pStatus + (mLoadCenter + 1), [](UCHAR x){ return x >= FINISHED; }))
			mIdle = true;
		else
		{
			for (UINT ring(mStartRing), i(1); ring <= mLoadCenter; ring++, i++)
			{
				if (pStatus[i] == UNLOADED)
				{
					Load(ring);
					LightPass(ring);
					break;
				}
				if (ring == mLoadCenter)
				{
					LightPass(ring);
				}
			}
		}
		free(pStatus);
	}
}