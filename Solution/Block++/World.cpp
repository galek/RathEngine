#include "stdafx.h"
#include "World.h"
#include "Entity.h"
#include "EntityItem.h"

#include <AssetLibrary.h>

const D3D11_BLEND_DESC DEFAULT_BLEND =
{
	FALSE, //BOOL AlphaToCoverageEnable;
	FALSE, //BOOL IndependentBlendEnable;
	{
		{
			FALSE, //BOOL BlendEnable;
			D3D11_BLEND_ONE, //D3D11_BLEND SrcBlend;
			D3D11_BLEND_ZERO, //D3D11_BLEND DestBlend;
			D3D11_BLEND_OP_ADD, //D3D11_BLEND_OP BlendOp;
			D3D11_BLEND_ONE, //D3D11_BLEND SrcBlendAlpha;
			D3D11_BLEND_ZERO, //D3D11_BLEND DestBlendAlpha;
			D3D11_BLEND_OP_ADD, //D3D11_BLEND_OP BlendOpAlpha;
			D3D11_COLOR_WRITE_ENABLE_ALL, //UINT8 RenderTargetWriteMask;
		},
	}
};

const D3D11_DEPTH_STENCIL_DESC DEPTH_DEPTHSTENCIL =
{
	TRUE,
	D3D11_DEPTH_WRITE_MASK_ALL,
	D3D11_COMPARISON_LESS_EQUAL,
	FALSE,
	D3D11_DEFAULT_STENCIL_READ_MASK,
	D3D11_DEFAULT_STENCIL_WRITE_MASK,
	D3D11_STENCIL_OP_KEEP,
	D3D11_STENCIL_OP_KEEP,
};

const D3D11_DEPTH_STENCIL_DESC SCENE_DEPTHSTENCIL =
{
	TRUE,
	D3D11_DEPTH_WRITE_MASK_ZERO,
	D3D11_COMPARISON_LESS_EQUAL,
	FALSE,
	D3D11_DEFAULT_STENCIL_READ_MASK,
	D3D11_DEFAULT_STENCIL_WRITE_MASK,
	D3D11_STENCIL_OP_KEEP,
	D3D11_STENCIL_OP_KEEP,
};

const D3D11_INPUT_ELEMENT_DESC BLOCK_LAYOUT[] =
{
	{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TANGENT",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "BINORMAL",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR",		0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD",	1, DXGI_FORMAT_R32G32_FLOAT,	0, 60, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

const D3D11_INPUT_ELEMENT_DESC INSTANCED_BLOCK_LAYOUT[] =
{
	{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0,   0, D3D11_INPUT_PER_VERTEX_DATA,	0 },
	{ "TANGENT",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0,  12, D3D11_INPUT_PER_VERTEX_DATA,	0 },
	{ "BINORMAL",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0,  24, D3D11_INPUT_PER_VERTEX_DATA,	0 },
	{ "COLOR",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0,  36, D3D11_INPUT_PER_VERTEX_DATA,	0 },
	{ "TEXCOORD",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0,  48, D3D11_INPUT_PER_VERTEX_DATA,	0 },
	{ "TEXCOORD",	1, DXGI_FORMAT_R32G32_FLOAT,		0,  60, D3D11_INPUT_PER_VERTEX_DATA,	0 },
	{ "WORLD",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	1,   0, D3D11_INPUT_PER_INSTANCE_DATA,	1 },
	{ "WORLD",		1, DXGI_FORMAT_R32G32B32A32_FLOAT,	1,  16, D3D11_INPUT_PER_INSTANCE_DATA,	1 },
	{ "WORLD",		2, DXGI_FORMAT_R32G32B32A32_FLOAT,	1,  32, D3D11_INPUT_PER_INSTANCE_DATA,	1 },
	{ "WORLD",		3, DXGI_FORMAT_R32G32B32A32_FLOAT,	1,  48, D3D11_INPUT_PER_INSTANCE_DATA,	1 },
};

const Rath::TechniqueSetting g_Techniques[] =
{
	{
		"BlockShader", D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST,
		{ "Block_vs", "Block_hs", "Block_ds", nullptr, "Block_ps" },
		BLOCK_LAYOUT, ARRAYSIZE(BLOCK_LAYOUT), nullptr, &SCENE_DEPTHSTENCIL, nullptr
	},
	{
		"BlockDepthShader", D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST,
		{ "BlockShadow_vs", "BlockShadow_hs", "BlockShadow_ds", nullptr, "BlockShadow_ps" },
		BLOCK_LAYOUT, ARRAYSIZE(BLOCK_LAYOUT), nullptr, &DEPTH_DEPTHSTENCIL, &DEFAULT_BLEND
	},
	{
		"BlockInstancingShader", D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST,
		{ "BlockInstancing_vs", "Block_hs", "Block_ds", nullptr, "Block_ps" },
		INSTANCED_BLOCK_LAYOUT, ARRAYSIZE(INSTANCED_BLOCK_LAYOUT), nullptr, nullptr, nullptr
	},
	{
		"BlockDepthInstancingShader", D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST,
		{ "BlockDepthInstancing_vs", "BlockShadow_hs", "BlockShadow_ds", nullptr, "BlockShadow_ps" },
		INSTANCED_BLOCK_LAYOUT, ARRAYSIZE(INSTANCED_BLOCK_LAYOUT), nullptr, nullptr, nullptr
	},
};

DWORD WINAPI World::LoaderFunction(LPVOID lpParam)
{
	World* pParent = (World*)lpParam;
	int32 range;

	while (!pParent->bTermiated)
	{
		uint32 changes = pParent->LoadCycle(range);
		if (changes <= 0)
			Sleep(10);
	}

	return 0;
}

void World::MATERIAL_CB_STRUCT::BindWorld(const XMMATRIX& world)
{
	mWorld = XMMatrixTranspose(world);
};

World::World(Rath::Node* pCharakter) :
mDevice(nullptr),
mDeferredUpdateContext(nullptr),
mEntityMesh(nullptr),
mCharakter(pCharakter),
bTermiated(false),
mLoadRange(CHUNK_WIDTH * 6),
mLastLoadedPosition(g_XMInfinity)
{
	BOOL newFile = CreateDirectory(L"Data", NULL);

	mBlockTechnique = Rath::AssetLibrary::GetTechnique("BlockShader", &g_Techniques[0]);
	mBlockDepthOnlyTechnique = Rath::AssetLibrary::GetTechnique("BlockDepthShader", &g_Techniques[1]);
	mBlockInstancedTechnique = Rath::AssetLibrary::GetTechnique("BlockInstancingShader", &g_Techniques[2]);
	mBlockDepthOnlyInstancedTechnique = Rath::AssetLibrary::GetTechnique("BlockDepthInstancingShader", &g_Techniques[3]);

	mColorTexture = Rath::AssetLibrary::GetTexture("TerrainColor.dds");
	mBiomeTexture = Rath::AssetLibrary::GetTexture("TerrainBiome.dds");
	//mNormalTexture = AssetLibrary::GetTexture("TerrainNormal.dds");

	Factory<Entity>::Instance().RegisterClassCreator<Entity>();
	Factory<Entity>::Instance().RegisterClassCreator<EntityItem>();

	//hSyncEvent = CreateEvent(
	//	NULL,               // default security attributes
	//	TRUE,               // manual-reset event
	//	FALSE,              // initial state is nonsignaled
	//	TEXT("SyncEvent")  // object name
	//	);

	hLoaderThread = CreateThread(
		nullptr,                // default security attributes
		0,                      // use default stack size  
		LoaderFunction,         // thread function name
		this,					// argument to thread function 
		0,						// use default creation flags 
		nullptr);				// returns the thread identifier 
	//SetThreadPriority(hLoaderThread, THREAD_PRIORITY_HIGHEST);
}

World::~World()
{
	bTermiated = true;
	//SetEvent(hSyncEvent);
	WaitForSingleObject(hLoaderThread, INFINITE);
	//CloseHandle(hSyncEvent);

	Release();
	m_Regions.clear();
}

std::shared_ptr<Region> World::GetRegion(const VectorInt3& position)
{
	HASH(position);

	tbb::concurrent_hash_map<int64, std::shared_ptr<Region>>::accessor it;
	if (m_Regions.find(it, hash))
	{
		return it->second;
	}
	else
	{
		return nullptr;
	}
}

std::shared_ptr<Chunk> World::GetChunk(const VectorInt3& position)
{
	HASH(position);
	tbb::concurrent_hash_map<int64, std::shared_ptr<Region>>::accessor it;
	if (m_Regions.find(it, hash))
	{
		return it->second->GetChunk(position);
	}
	else
	{
		VectorInt3 pos = VectorInt3(position.x & REGION_WIDTH_INVMASK, 0,
									position.z & REGION_WIDTH_INVMASK);

		std::shared_ptr<Region> region (new Region(this, pos));

		m_Regions.emplace(hash, region);

		return region->GetChunk(position);
	}
}

Block World::GetBlock(const VectorInt3& position) const
{
	if (position.y & CHUNK_HEIGHT_INVMASK)
		return Block::Void;

	HASH(position);
	tbb::concurrent_hash_map<int64, std::shared_ptr<Region>>::const_accessor it;
	if (m_Regions.find(it, hash))
	{
		return it->second->GetBlock(position);
	}
	else
	{
		return Block::Void;
	}
}

void World::SetBlock(const VectorInt3& position, const Block& block, Blockupdate flag)
{
	HASH(position);
	tbb::concurrent_hash_map<int64, std::shared_ptr<Region>>::accessor it;
	if (m_Regions.find(it, hash))
	{
		it->second->SetBlock(position, block, flag);
	}
	else
	{
		//mRegionLock.lock();
		//VectorInt3 pos = VectorInt3(x, 0, z);
		//mRegions.emplace(hash, new Region(this, pos));
		//mRegionLock.unlock();
	}
}

void World::AddEntity(Entity* node)
{
	VectorInt3 pos = node->GetPosition();
	HASH(pos);
	tbb::concurrent_hash_map<int64, std::shared_ptr<Region>>::accessor it;
	if (m_Regions.find(it, hash))
	{
		it->second->AddEntity(node);
	}
}

void World::RemoveEntity(Entity* node)
{
	VectorInt3 pos = node->GetPosition();
	HASH(pos);
	tbb::concurrent_hash_map<int64, std::shared_ptr<Region>>::accessor it;
	if (m_Regions.find(it, hash))
	{
		it->second->RemoveEntity(node);
	}
}

uint32 World::LoadCycle(int32 & range)
{
	uint32 changes = 0;
	XMVECTOR position = XMVectorSetY(mCharakter->GetPosition(), 0.0f);
	XMVECTOR lastposition = mLastLoadedPosition;
	float dist = XMVectorGetX(XMVector3Length((lastposition - lastposition) * XMVectorSet(1.0f, 0.0f, 1.0f, 1.0f)));
	if (isnan(dist))
	{
		range = CHUNK_WIDTH;
		mLastLoadedPosition = position;
	}
	else
	{
		int32 idist = (int32)dist / 2;
		if (idist >= CHUNK_WIDTH)
		{
			range = max(CHUNK_WIDTH, range - (idist / CHUNK_WIDTH) * CHUNK_WIDTH);
			mLastLoadedPosition = position;
			changes += 1;
		}
	}

	changes += LoadInRange(position, range);

	ClearOutOfRange(position, mLoadRange);

	mUpdateLock.lock();
	if (mDevice) concurrency::parallel_for_each(m_Regions.begin(), m_Regions.end(),
		[&](std::pair<const int64, std::shared_ptr<Region>>& it)
	{
		it.second->CreateResources(mDevice, mDeferredUpdateContext);
	});
	mUpdateLock.unlock();

	//DWORD dwWaitResult = WaitForSingleObject(hSyncEvent, INFINITE);

	range = min(range + CHUNK_WIDTH, mLoadRange);

	return changes;
}

uint32 World::LoadInRange(const XMVECTOR& position, int32 range)
{
	XMINT3 Position;
	XMStoreSInt3(&Position, position);
	XMINT3 Min((Position.x - range) & REGION_WIDTH_INVMASK, 0, (Position.z - range) & REGION_WIDTH_INVMASK);
	XMINT3 Max((Position.x + range) & REGION_WIDTH_INVMASK, 0, (Position.z + range) & REGION_WIDTH_INVMASK);
	XMBOUNDINGBOX boundingbox(position, XMVectorReplicate((float)range));

	for (int32 x(Min.x); x <= Max.x; x += REGION_REALWIDTH)
		for (int32 z(Min.z); z <= Max.z; z += REGION_REALWIDTH)
		{
			XMVECTOR extents = { REGION_REALWIDTH / 2.0f, 0, REGION_REALWIDTH / 2.0f, 0.f };
			XMVECTOR center = XMVectorSet((float)x, 0.f, (float)z, 1.f) + extents;
			XMBOUNDINGBOX aabb(center, extents);

			if (boundingbox.Intersects(aabb))
			{
				int64 hash = int64(x) + (int64(z) << 32);

				tbb::concurrent_hash_map<int64, std::shared_ptr<Region>>::const_accessor it;
				if (!m_Regions.find(it, hash))
				{
					VectorInt3 pos = VectorInt3(x, 0, z);

					std::shared_ptr<Region> region(new Region(this, pos));

					m_Regions.emplace(hash, region);
				}
			}
		}

	concurrency::combinable<uint32> changes;
	concurrency::parallel_for_each(m_Regions.begin(), m_Regions.end(),
		[&](std::pair<const int64, std::shared_ptr<Region>>& it)
	{
		uint32 change = it.second->LoadInRange(boundingbox);
		changes.local() += change;
	});
	return changes.combine(std::plus<uint32>());
}

void World::ClearOutOfRange(const XMVECTOR& position, int32 range)
{
	XMBOUNDINGBOX boundingbox(position, XMVectorReplicate((float)range));

	concurrency::concurrent_vector<int64> unloadingkeys;
	concurrency::parallel_for_each(m_Regions.begin(), m_Regions.end(),
		[&](std::pair<const int64, std::shared_ptr<Region>>& it)
	{
		int32 change = it.second->ClearOutOfRange(boundingbox);
		if (change < 0 && it.second.unique())
		{
			unloadingkeys.push_back(it.first);
		}
	});

	for (auto key : unloadingkeys)
	{
		tbb::concurrent_hash_map<int64, std::shared_ptr<Region>>::const_accessor it;
		if (m_Regions.find(it, key) && 
		   (it->second.unique()))
		{
			m_Regions.erase(it);
		}
	}
}

bool World::Raymarch(const XMVECTOR& Origin, const XMVECTOR& Direction, uint32 & maxdistance, VectorInt3 & outCoord, Block & outBlock, int32 & facing)
{
	//http://www.cse.yorku.ca/~amana/research/grid.pdf  <----- ALGO
	VectorInt3 Position; XMStoreSInt3(&Position, Origin);

	// Determine which way we go.
	XMVECTOR diff = XMVectorGreater(Direction, g_XMZero); // > 0
	XMVECTOR step = XMVectorSelect(XMVectorSelect(g_XMNegativeOne, g_XMOne, diff), g_XMZero, XMVectorNearEqual(Direction, g_XMZero, g_XMEpsilon));

	// Calculate cell boundaries. When the step (i.e. direction sign) is positive,
	// the next boundary is AFTER our current position, meaning that we have to add 1.
	// Otherwise, it is BEFORE our current position, in which case we add nothing.
	XMVECTOR cellBoundary = XMVectorSelect(g_XMZero, g_XMOne, diff) + XMLoadSInt3(&Position);

	// NOTE: For the following calculations, the result will be Single.PositiveInfinity
	// when ray.Direction.X, Y or Z equals zero, which is OK. However, when the left-hand
	// value of the division also equals zero, the result is Single.NaN, which is not OK.

	// Determine how far we can travel along the ray before we hit a voxel boundary.
	XMVECTOR tMax = (cellBoundary - Origin) / Direction;

	// Determine how far we must travel along the ray before we have crossed a gridcell.
	XMVECTOR tDelta = step / Direction;

	// For each step, determine which distance to the next voxel boundary is lowest (i.e.
	// which voxel boundary is nearest) and walk that way.
	VectorInt3 LastPosition = Position;
	VectorInt3 Step;	XMStoreSInt3(&Step, step);
	XMFLOAT3 Max;	XMStoreFloat3(&Max, tMax);
	XMFLOAT3 Delta;	XMStoreFloat3(&Delta, tDelta);
	for (UINT i = 0; i < maxdistance; i++)
	{
		// Return it.
		// yield return new Point3D(x, y, z);
		Block block = GetBlock(Position);
		if (block.mBlocktype != Blocktype::Air) {
			if (block.mBlocktype == Blocktype::Void)
				return false;

			outCoord = Position;
			outBlock = block;
			maxdistance -= i;

			{
				VectorInt3 Difference = VectorInt3(Position.x - LastPosition.x, Position.y - LastPosition.y, Position.z - LastPosition.z);

				if (Difference.x > 0) {
					facing = FACELEFT;
				}
				else if (Difference.x < 0) {
					facing = FACERIGHT;
				}
				else if (Difference.y > 0) {
					facing = FACEDOWN;
				}
				else if (Difference.y < 0) {
					facing = FACEUP;
				}
				else if (Difference.z > 0) {
					facing = FACEBACK;
				}
				else if (Difference.z < 0) {
					facing = FACEFRONT;
				}
			}

			return true;
		}
		else
		{
			LastPosition = Position;
		}

		//	if (pos.y > 0.0f)
		//		*facing += POSITIONUP;
		//	else
		//		*facing += POSITIONDOWN;

		//	if (pos.x > 0.0f)
		//		*facing += POSITIONLEFT;
		//	else
		//		*facing += POSITIONRIGHT;

		//	if (pos.z > 0.0f)
		//		*facing += POSITIONBACK;
		//	else
		//		*facing += POSITIONFRONT;

		//}

		// Do the next step.
		if (Max.x < Max.y && Max.x < Max.z)
		{
			// tMax.X is the lowest, an YZ cell boundary plane is nearest.
			Position.x += Step.x;
			Max.x += Delta.x;
		}
		else if (Max.y < Max.z)
		{
			// tMax.Y is the lowest, an XZ cell boundary plane is nearest.
			Position.y += Step.y;
			Max.y += Delta.y;
		}
		else
		{
			// tMax.Z is the lowest, an XY cell boundary plane is nearest.
			Position.z += Step.z;
			Max.z += Delta.z;
		}
	}
	return false;
}

void World::Release()
{
	mDevice = nullptr;

	//for (auto it : m_Regions)
	//{
	//	if (it.second)
	//		it.second->ReleaseResources();
	//}
	SAFE_RELEASE(mEntityMesh);
	SAFE_RELEASE(mDeferredUpdateContext);
}

void World::Create(ID3D11Device* pd3dDevice)
{
	mDevice = pd3dDevice;
	pd3dDevice->CreateDeferredContext(0, &mDeferredUpdateContext);

	mUpdateLock.lock();
	concurrency::parallel_for_each(m_Regions.begin(), m_Regions.end(),
		[&](std::pair<const int64, std::shared_ptr<Region>>& it)
	{
		it.second->CreateResources(pd3dDevice, mDeferredUpdateContext);
	});
	mUpdateLock.unlock();

	mConstantBufferPrepass.Initialize(pd3dDevice);
	mConstantBufferMainpass.Initialize(pd3dDevice);

	std::vector<BLOCK_MESH> mesh;
	Block::Stone.GetMesh(Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f), 0xFF, mesh);
	XMMATRIX mat = XMMatrixTranslation(-0.5f, -0.5f, -0.5f) * XMMatrixScaling(0.3f, 0.3f, 0.3f);
	for (auto& it : mesh)
	{
		it.Transform(mat);
	}
	mEntityMesh = new Rath::ID3D11InstancedMesh(pd3dDevice, mesh);
}

void World::Update(ID3D11DeviceContext* pImmediateContext)
{
	if (mUpdateLock.try_lock())
	{
		ID3D11CommandList* commandlist;
		mDeferredUpdateContext->FinishCommandList(false, &commandlist);
		pImmediateContext->ExecuteCommandList(commandlist, true);
		commandlist->Release();
		mUpdateLock.unlock();
	}
}

void World::Render(ID3D11DeviceContext* pImmediateContext, const XMMATRIX& world, const XMFRUSTUM& frustum)
{
	std::vector<std::tuple<std::shared_ptr<Chunk>, DirectX::XMMATRIXLIST*>> chunks;

	for (auto it : m_Regions)
	{
		//it.second->SortEntities();
		it.second->VisibleChunks(frustum, 1, &chunks);
	}

	mBlockTechnique->Apply(pImmediateContext);
	mColorTexture->BindToPS(pImmediateContext, 0);
	mBiomeTexture->BindToPS(pImmediateContext, 1);
	mConstantBufferMainpass.SetVS(pImmediateContext, 1);

	for (auto& it : chunks) 
	{
		Chunk* chunk = std::get<0>(it).get();
		if (chunk)
		{
			XMMATRIX mat = world * XMMatrixTranslation((float)chunk->mPosition.x, 0.0f, (float)chunk->mPosition.z);
			mConstantBufferMainpass.Data.BindWorld(mat);
			mConstantBufferMainpass.ApplyChanges(pImmediateContext);
			chunk->Render(pImmediateContext);
		}
	}

	mBlockInstancedTechnique->Apply(pImmediateContext);

	for (auto& it : chunks)
	{
		Chunk* chunk = std::get<0>(it).get();
		if (chunk)
		{
			DirectX::XMMATRIXLIST* entities = std::get<1>(it);
			if (entities)
			{
				chunk->BindLightTexture(pImmediateContext);			
				mEntityMesh->CreateInstancing(mDevice, pImmediateContext, (UINT)entities->size(), &(*entities)[0]);
				mEntityMesh->Render(pImmediateContext);
				delete entities;
			}
		}
	}
}

void World::RenderDepthOnly(ID3D11DeviceContext* pImmediateContext, uint32 levelOfDetail, const XMMATRIX& world, const XMFRUSTUM& frustum)
{
	std::vector<std::shared_ptr<Chunk>> chunks;
	DirectX::XMMATRIXLIST entities;

	for (auto it : m_Regions)
	{
		it.second->VisibleChunks(frustum, levelOfDetail, &chunks, &entities);
	}

	mBlockDepthOnlyTechnique->Apply(pImmediateContext);
	mColorTexture->BindToPS(pImmediateContext, 0);
	mConstantBufferPrepass.SetVS(pImmediateContext, 1);

	for (auto it : chunks) if (it)
	{
		XMMATRIX mat = world * XMMatrixTranslation((float)it->mPosition.x, 0.0f, (float)it->mPosition.z);
		mConstantBufferPrepass.Data.BindWorld(mat);
		mConstantBufferPrepass.ApplyChanges(pImmediateContext);
		it->RenderDepthOnly(pImmediateContext, levelOfDetail);
	}

	if (entities.size() > 0)
	{
		mBlockDepthOnlyInstancedTechnique->Apply(pImmediateContext);
		mEntityMesh->CreateInstancing(mDevice, pImmediateContext, (UINT)entities.size(), &entities[0]);
		mEntityMesh->Render(pImmediateContext);
	}
}