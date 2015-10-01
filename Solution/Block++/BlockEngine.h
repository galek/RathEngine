#pragma once
#include "BaseEngine.h"
#include "NodeController.h"
#include "LuaConsole.h"

//#include "BrowserManager.h"

#include "Sky.h"
#include "World.h"
#include "Player.h"
#include "SelectionBox.h"

class BlockEngine : public Rath::BaseEngine
{
protected:
#if defined(_PROFILE) | defined(_DEBUG)
	LuaConsole			mLuaConsole;
#endif
	Player					mPlayer;
	//Rath::NodeControllerFPS	mController;

	Rath::Sky				mSky;
	World					mWorld;
	SelectionBox			mTargetBox;

	UINT							mDistanceToBlock;
	VectorInt3						mBlockCoord;
	Block							mViewedBlock;
	INT								mBlockFacing;
	Keyboard::KeyboardStateTracker	mKeyboardTracker;

	std::vector<Entity*>			mDestroyedEntities;
	void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs);
private:
	static BlockEngine* g_BlockEngine;
#if defined(_PROFILE) | defined(_DEBUG)
	static int setlight(lua_State* lua);
	static int setDebugSetting(lua_State* lua);
#endif
public:
	BlockEngine(HINSTANCE hInstance);
	~BlockEngine();

	void CreateDevice(ID3D11Device* device) override;
	void CreateResources(ID3D11Device* device) override;

	void RenderScene(ID3D11DeviceContext* context, const XMMATRIX& mViewProj, Rath::Scene::RenderPass pass) override;

	void FrameMove(float fElapsedTime) override;
};

