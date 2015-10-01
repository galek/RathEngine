#pragma once
#include <Technique.h>
#include <Texture.h>
#include <GraphicTypes.h>
#include <Node.h>

#include "tbb/concurrent_hash_map.h"

#include <ppl.h>

#include "Region.h"
#include "WorldGenerator.h"

class World 
{
protected:
	struct MATERIAL_CB_STRUCT
	{
		Matrix  mWorld;                         // World matrix
		Vector4	vMaterialAmbientColor;			// Material's ambient color
		Vector4	vMaterialDiffuseColor;			// Material's diffuse color
		Vector4	vMaterialSpecularColor;			// Material's specular color
		Vector4	vSpecularExponent;				// Material's specular exponent

		void BindWorld(const XMMATRIX& world);
	};
	Rath::ConstantBuffer<MATERIAL_CB_STRUCT>  mConstantBufferPrepass;
	Rath::ConstantBuffer<MATERIAL_CB_STRUCT>  mConstantBufferMainpass;

	Rath::TechniquePtr	mBlockTechnique;
	Rath::TechniquePtr	mBlockDepthOnlyTechnique;
	Rath::TechniquePtr	mBlockInstancedTechnique;
	Rath::TechniquePtr	mBlockDepthOnlyInstancedTechnique;

	Rath::TexturePtr	mColorTexture;
	Rath::TexturePtr	mBiomeTexture;
	Rath::TexturePtr	mNormalTexture;

	Rath::ID3D11InstancedMesh*	mEntityMesh;

	tbb::concurrent_hash_map<int64, std::shared_ptr<Region>>	m_Regions;

	Rath::Node*					mCharakter;
	Vector3						mLastLoadedPosition;
	int32						mLoadRange;
	std::mutex					mUpdateLock;
	ID3D11Device*				mDevice;
	ID3D11DeviceContext*		mDeferredUpdateContext;
protected:
	uint32	LoadCycle(int32 & range);
	uint32	LoadInRange(const XMVECTOR& position, int32 range);
	void	ClearOutOfRange(const XMVECTOR& position, int32 range);
private:
	bool	bTermiated;
	HANDLE	hLoaderThread;
	HANDLE	hSyncEvent;
	static DWORD WINAPI LoaderFunction(LPVOID lpParam);
public:
	World(Rath::Node* pCharakter);
	~World();

	std::shared_ptr<Region>	GetRegion(const VectorInt3& position);
	std::shared_ptr<Chunk>	GetChunk(const VectorInt3& position);
	Block	GetBlock(const VectorInt3& position) const;
	void	SetBlock(const VectorInt3& position, const Block& block, Blockupdate flag = Blockupdate::None);

	void	AddEntity(Entity* node);
	void	RemoveEntity(Entity* node);

	bool	Raymarch(const XMVECTOR& Origin, const XMVECTOR& Direction, uint32 & maxdistance, VectorInt3 & outCoord, Block & outBlock, int32 & facing);

	void Release();
	void Create(ID3D11Device* pd3dDevice);
	void Update(ID3D11DeviceContext* pImmediateContext);
	void Render(ID3D11DeviceContext* pImmediateContext, const XMMATRIX& world, const XMFRUSTUM& frustum);
	void RenderDepthOnly(ID3D11DeviceContext* pImmediateContext, uint32 levelOfDetail, const XMMATRIX& world, const XMFRUSTUM& frustum);
};

