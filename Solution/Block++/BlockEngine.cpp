#include "stdafx.h"
#include "BlockEngine.h"

#include "EntityItem.h"

BlockEngine* BlockEngine::g_BlockEngine = nullptr;
BlockEngine::BlockEngine(HINSTANCE hInstance) :
	//BrowserManager(hInstance),
	BaseEngine(L"Block++", L"IDI_ICON", nullptr, nullptr),
	mPlayer(&m_State.m_Camera),
	mWorld(&m_State.m_Camera),
	mSky(&m_State.m_Camera, &m_State.m_LightDirection),
	mViewedBlock(Block::Air)
{
	m_State.m_Camera.SetParent(&mPlayer);
	m_State.m_Camera.SetRelativePosition(XMVectorSet(0.0f, 0.2f, 0.0f, 1.0f));

	g_BlockEngine = this;

#if defined(_PROFILE) | defined(_DEBUG)
	if (AllocConsole())
	{
		FILE *stream;
		freopen_s(&stream, "CONIN$", "r", stdin);
		freopen_s(&stream, "CONOUT$", "w", stdout);
		freopen_s(&stream, "CONOUT$", "w", stderr);
	}

	mLuaConsole.AddFunction(setlight, "setlight");
	mLuaConsole.AddFunction(setDebugSetting, "debug");

	mLuaConsole.Start();
#endif

	AddNode(&mPlayer);

	//Rath::Texture* texture = CreateBrowserTexture(512, 512, "https://www.youtube.com/watch?v=C__8lum0ck8");
}

BlockEngine::~BlockEngine()
{
#if defined(_PROFILE) | defined(_DEBUG)
	//mLuaConsole.Kill();

	FreeConsole();
#endif

	mWorld.Release();

	g_BlockEngine = nullptr;
}

void BlockEngine::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
{
	//PX_UNUSED((pairHeader));
	if (pairHeader.actors[0]->userData != nullptr || pairHeader.actors[1]->userData != nullptr)
	{
		EntityItem* entity = (EntityItem*)(pairHeader.actors[0]->userData != nullptr ? pairHeader.actors[0]->userData : pairHeader.actors[1]->userData);
		mWorld.RemoveEntity(entity);
		mDestroyedEntities.emplace_back(entity);
	} 

	//const physx::PxU32 bufferSize = 64;
	//physx::PxContactPairPoint contacts[bufferSize];
	//for (physx::PxU32 i = 0; i<nbPairs; i++)
	//{
	//	const physx::PxContactPair& cp = pairs[i];

	//	//PxU32 nbContacts = pairs[i].extractContacts(contacts, bufferSize);
	//	//for (PxU32 j = 0; j < nbContacts; j++)
	//	//{
	//	//	PxVec3 point = contacts[j].position;
	//	//	PxVec3 impulse = contacts[j].impulse;
	//	//	PxU32 internalFaceIndex0 = contacts[j].internalFaceIndex0;
	//	//	PxU32 internalFaceIndex1 = contacts[j].internalFaceIndex1;
	//	//	//...
	//	//}
	//}
}

void BlockEngine::CreateDevice(ID3D11Device* device)
{
	mWorld.Release();
	mWorld.Create(device);
	mSky.CreateDevice(device);
	mTargetBox.CreateDevice(device);
}

void BlockEngine::CreateResources(ID3D11Device* device)
{

}

void BlockEngine::FrameMove(float fElapsedTime)
{
	mSky.Update(fElapsedTime);

	mDistanceToBlock = 32;
	
	auto keyboard = Keyboard::Get().GetState();
	mKeyboardTracker.Update(keyboard);

	if (mWorld.Raymarch(m_State.m_Camera.GetPosition(), m_State.m_Camera.GetDirection(), mDistanceToBlock, mBlockCoord, mViewedBlock, mBlockFacing))
	{
		VectorInt3 offset;
		switch (mBlockFacing)
		{
		case FACELEFT:
			offset.x = 1;
			break;
		case FACERIGHT:
			offset.x = -1;
			break;
		case FACEDOWN:
			offset.y = 1;
			break;
		case FACEUP:
			offset.y = -1;
			break;
		case FACEBACK:
			offset.z = 1;
			break;
		case FACEFRONT:
			offset.z = -1;
			break;
		}
		if (mKeyboardTracker.IsKeyPressed(Keyboard::B))
		{
			mWorld.SetBlock(mBlockCoord - offset, Block::Stone);
		}
		if (mKeyboardTracker.IsKeyHold(Keyboard::C))
		{
			mWorld.SetBlock(mBlockCoord, Block::Air);
		}
	}

	if (mKeyboardTracker.IsKeyHold(Keyboard::I))
	{
		XMMATRIX world = m_State.m_Camera.GetWorld();
		world *= XMMatrixTranslationFromVector(m_State.m_Camera.GetDirection());
		EntityItem* entity = new EntityItem(world);
		entity->SetVelocity(m_State.m_Camera.GetDirection() * 10.0f);
		mWorld.AddEntity(entity);
	}

	if (mDestroyedEntities.size() > 0)
	{
		for (auto it : mDestroyedEntities)
		{
			delete it;
		}
		mDestroyedEntities.clear();
	}

	mWorld.Update(GetDeviceContext());
}

void BlockEngine::RenderScene(ID3D11DeviceContext* context, const XMMATRIX& mViewProj, Rath::Scene::RenderPass pass)
{
	XMFRUSTUM frustum = XMFRUSTUM(mViewProj);
	XMMATRIX worldoffset = XMMatrixIdentity();

	switch (pass)
	{
	case Rath::Scene::BackgroundPass:
		mSky.Render(context);
		break;
	case Rath::Scene::DepthOnlyPass:
		mWorld.RenderDepthOnly(context, 1, worldoffset, frustum);
		break;

	case Rath::Scene::ShadowPass:
	case Rath::Scene::ShadowPass_Cascade1:
	case Rath::Scene::ShadowPass_Cascade2:
	case Rath::Scene::ShadowPass_Cascade3:
	case Rath::Scene::ShadowPass_Cascade4:
		mWorld.RenderDepthOnly(context, 0, worldoffset, frustum);
		break;
	case Rath::Scene::ScenePass:
		mWorld.Render(context, worldoffset, frustum);
		mTargetBox.Render(context, mBlockCoord.Float(), mViewedBlock);
		break;
	case Rath::Scene::UIPass:
	{
		std::wstringstream buffer;
		buffer << mViewedBlock;
		PrintText(buffer.str().c_str(), XMINT2(221, 21), 18, Colors::Black);
		PrintText(buffer.str().c_str(), XMINT2(220, 20), 18, Colors::Yellow);

		auto print = [&](int x, int y, const wchar_t * _Format, ...)
		{
			WCHAR buffer[_MAX_PATH];
			va_list argptr;
			va_start(argptr, _Format);
			_vsnwprintf_s(buffer, _MAX_PATH, _Format, argptr);
			va_end(argptr);
			PrintText(buffer, XMINT2(x + 1, y + 1), 16, DirectX::Colors::Black);
			PrintText(buffer, XMINT2(x, y), 16, DirectX::Colors::White);
		};

	}
	break;
	default:
		break;
	}

	context->VSSetShader(nullptr, nullptr, 0);
	context->GSSetShader(nullptr, nullptr, 0);
	context->HSSetShader(nullptr, nullptr, 0);
	context->DSSetShader(nullptr, nullptr, 0);
	context->PSSetShader(nullptr, nullptr, 0);
	context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}