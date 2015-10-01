#pragma once
#include "Block.h"
#include "ChunkMesh.h"

class Region;
class Chunk : public ChunkMesh, public std::enable_shared_from_this<Chunk>
{
	friend class WorldGenerator;
	friend class Region;
private:
	struct LightNode
	{
		LightNode(uint32 indx, std::shared_ptr<Chunk> ch) : index(indx), chunk(ch) {}
		uint32 index; //this is the x y z coordinate!
		std::shared_ptr<Chunk> chunk; //pointer to the chunk that owns it!
	};

	struct LightRemovalNode
	{
		LightRemovalNode(uint32 indx, Light v, std::shared_ptr<Chunk> ch) : index(indx), val(v), chunk(ch) {}
		uint32	index; //this is the x y z coordinate!
		Light	val;
		std::shared_ptr<Chunk>	chunk; //pointer to the chunk that owns it!
	};

	void SetLight(std::queue<LightNode> & lightBfsQueue);
	void RemoveLight(std::queue <LightRemovalNode> & lightRemovalBfsQueue);
private:
	uint16(*mHeight)[CHUNK_WIDTH + 2];
public:
	Region*		mParent;
	VectorInt3  mPosition;
	uint16		mLoadedHeight;
	uint16		mVisibleHeight;
	uint16		mLightHeight;
	union
	{
		// Access: Y(256) - Z(16) - X(1)
		Block(*mBlocks)[CHUNK_WIDTH][CHUNK_WIDTH];
		Block  *mBlocksDirect;
	};

	void	Resize(WORD height, bool FillWithSky = false);

	Chunk(Region* parent, const VectorInt3& position);
	~Chunk();

	Block	GetBlock(const VectorInt3& position) const;
	void	SetBlock(const VectorInt3& position, const Block& block, Blockupdate flag = Blockupdate::None);

	template <size_t X, size_t Y, size_t Z>
	void	GetBlocks(const VectorInt3& position, Block(*blocks)[Z][X]);

	void	SetLight(const VectorInt3& position, const Light oldlight);
	void	RemoveLight(const VectorInt3& position, const Light newlight);
private:
	void	PlaceSkyLight();
	void	PropagateSkyLight();
	void	GenerateHeight();
	void	GenerateMesh(std::vector<BLOCK_MESH>& Mesh, std::vector<uint32>& Light);
	void	GenerateShape();
};