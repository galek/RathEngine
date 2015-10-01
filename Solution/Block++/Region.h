#pragma once
#include "Chunk.h"
#include "Entity.h"

enum ChunkStatus : uint8
{
	NotLoaded		= 0x00,
	Loaded			= 0x01,
	Generated		= 0x02,
	Finished		= 0x04,
	Visible			= 0x10,
	Changed			= 0x20,
	ReadyForDisplay = 0x40,
};
DEFINE_ENUM_FLAG_OPERATORS(ChunkStatus);

class World;
class Region : public std::enable_shared_from_this<Region>
{
private:
	struct Record
	{
		uint32 Position;
		uint32 Size;
	};
	struct Header
	{
		Record	Records[REGION_SQRWIDTH];
		uint32	Size;
	};
	struct EntityEntry
	{
		Entity*		Node;
		uint32		Index;

		bool operator<(const EntityEntry& rhs) const
		{
			return Index < rhs.Index;
		};
		bool operator==(const EntityEntry& rhs) const
		{
			return Index == rhs.Index;
		};
		bool operator==(const Rath::Node* rhs) const
		{
			return Node == rhs;
		};
	};
protected:
	World*						mParent;
	VectorInt3					mPosition;
	Header						mSavefileHeader;
	std::FILE*					mSavefile;
	std::mutex					mFilelock;
	std::shared_ptr<Chunk>		mChunks[REGION_SQRWIDTH];
	std::weak_ptr<Chunk>		mChunkBackups[REGION_SQRWIDTH];
	uint8						mChunkStatus[REGION_SQRWIDTH];
	std::mutex					mEntitieslock;
	std::vector<EntityEntry>	mEntities;
	uint32						mEntitiesInChunk[REGION_SQRWIDTH][2];
public:
	Region(World* parent, const VectorInt3& position);
	~Region();

	std::shared_ptr<Chunk>	GetChunk(const VectorInt3& position);

	Block	GetBlock(const VectorInt3& position) const;
	void	SetBlock(const VectorInt3& position, const Block& block, Blockupdate flag = Blockupdate::None);

	void	ReleaseResources();
	void	CreateResources(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pImmediateContext);

	void	VisibleChunks(const XMFRUSTUM& frustum, uint32 levelOfDetail, 
						  std::vector<std::shared_ptr<Chunk>>* chunks, 
						  DirectX::XMMATRIXLIST* entities = nullptr);
	void	VisibleChunks(const XMFRUSTUM& frustum, uint32 levelOfDetail,
						  std::vector<std::tuple<std::shared_ptr<Chunk>, DirectX::XMMATRIXLIST*>>* chunks);

	void	AddEntity(Entity* node);
	void	RemoveEntity(Entity* node);
	void	SortEntities();

	uint32	LoadInRange(const XMBOUNDINGBOX& range);
	int32	ClearOutOfRange(const XMBOUNDINGBOX& range);
private:
	void	Save(uint32 index, Chunk* chunk = nullptr);
	void	Load(uint32 index);

	uint32	GetAreaStatus(uint32 x, uint32 z);
};
