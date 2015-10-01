#pragma once
#include <Node.h>

class World;
class Chunk;
class Loader
{
protected:
	World*									mParent;
	Node*									mCharacter;
	concurrency::concurrent_vector<Chunk*>	mUnLoadList;
	std::atomic<bool>						mReadyUnload;
	XMINT3									mCurrentPosition;
	XMINT3									mLastPosition;
	XMINT3									mMovedPosition;
	UINT									mLoadDistance;
	UINT									mLoadCenter;
	UINT									mStartRing;
	bool									mIdle;
	union
	{
		// Access: Z(16) - X(1)
		UCHAR	mStatus[64][64];
		UCHAR	mStatusDirect[4096];
	};
	void	UpdatePosition();
	void	UpdateStatus();
	UCHAR	GetStatus(size_t ring);
	void	Load(size_t ring);
	void	LightPass(size_t ring);
public:
	Loader(World* world, Node* character);


	void Work();
};

