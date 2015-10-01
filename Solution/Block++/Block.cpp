#include "stdafx.h"
#include "Block.h"

#include "Shapes.h"

bool Block::operator==(const Block& rhs) const 
{
	return ((mBlocktype == rhs.mBlocktype) && (mMetadata == rhs.mMetadata));
};

bool Block::operator<(const Block& rhs) const 
{
	return ((mBlocktype < rhs.mBlocktype) || ((mBlocktype == rhs.mBlocktype) && (mMetadata < rhs.mMetadata)));
};

const Block Block::Sky = { Blocktype::Air, Light(0, 0xF), Blockstatus::Clear };
const Block Block::Air = { Blocktype::Air, Light(), Blockstatus::Clear };
const Block Block::Dirt = { Blocktype::Dirt, Light(), Blockstatus::Solid };
const Block Block::Grass = { Blocktype::Grass, Light(), Blockstatus::Solid };
const Block Block::SnowyGrass = { Blocktype::Grass, Light(), Blockstatus::Solid, 1};
const Block Block::Stone = { Blocktype::Stone, Light(), Blockstatus::Solid };
const Block Block::Cobble = { Blocktype::Cobble, Light(), Blockstatus::Solid };
const Block Block::CoalOre = { Blocktype::CoalOre, Light(), Blockstatus::Solid };
const Block Block::IronOre = { Blocktype::IronOre, Light(), Blockstatus::Solid };
const Block Block::GoldOre = { Blocktype::GoldOre, Light(), Blockstatus::Solid };
const Block Block::DiamandOre = { Blocktype::DiamandOre, Light(), Blockstatus::Solid };
const Block Block::RedstoneOre = { Blocktype::RedstoneOre, Light(0x5000), Blockstatus::Lightsource | Blockstatus::Solid };
const Block Block::Wood = { Blocktype::Wood, Light(), Blockstatus::Solid, 0x00 };
const Block Block::DarkWood = { Blocktype::Wood, Light(), Blockstatus::Solid, 0x01 };
const Block Block::BirchWood = { Blocktype::Wood, Light(), Blockstatus::Solid, 0x02 };
const Block Block::RedWood = { Blocktype::Wood, Light(), Blockstatus::Solid, 0x03 };
const Block Block::WoodPlank = { Blocktype::WoodPlank, Light(), Blockstatus::Solid, 0x00 };
const Block Block::Leaf = { Blocktype::Leaf, Light(), Blockstatus::LDV2, 0x0F };
const Block Block::DarkLeaf = { Blocktype::Leaf, Light(), Blockstatus::LDV3, 0x1F };
const Block Block::RedLeaf = { Blocktype::Leaf, Light(), Blockstatus::LDV3, 0x2F };
const Block Block::TallGrass = { Blocktype::TallGrass, Light(), Blockstatus::Walkable };
const Block Block::Glowstone = { Blocktype::Glowstone, Light(0xFEB0), Blockstatus::Lightsource | Blockstatus::Solid };
const Block Block::Reed = { Blocktype::Reed, Light(), Blockstatus::Walkable };
const Block Block::Cactus = { Blocktype::Cactus, Light(), Blockstatus::Clear };
const Block Block::Sand = { Blocktype::Sand, Light(), Blockstatus::Solid };
const Block Block::Lightstone = { Blocktype::Lightstone, Light(0xAAA0), Blockstatus::Lightsource | Blockstatus::Solid };
const Block Block::Torch = { Blocktype::Torch, Light(0xEDB0), Blockstatus::Lightsource | Blockstatus::Walkable };
const Block Block::RedstoneDust = { Blocktype::RedstoneDust, Light(), Blockstatus::Walkable };
const Block Block::RedstoneTorch = { Blocktype::RedstoneTorch, Light(0x7000), Blockstatus::Lightsource | Blockstatus::Walkable };
const Block Block::Fence = { Blocktype::Fence, Light(), Blockstatus::Clear };
const Block Block::Water = { Blocktype::Water, Light(), Blockstatus::LDV2 | Blockstatus::Walkable, 0, 0x3232 };
const Block Block::Lava = { Blocktype::Lava, Light(0xFDB0), Blockstatus::LDV10 | Blockstatus::Lightsource | Blockstatus::Walkable, 0, 0x3232 };
const Block Block::Void = { Blocktype::Void, Light(), Blockstatus::Solid };

const std::vector<XMBOUNDINGBOX> BlockBoundingBox =
{
	XMBOUNDINGBOX(XMVectorReplicate(0.5f), XMVectorReplicate(0.5f)),
};

typedef size_t(*GeneratorFunction)(const Vector3&, const Vector3&, const Vector3&, const Vector3&, CHAR, Block, std::vector<BLOCK_MESH>&);
typedef Light(*LightingFunction)(Block);

struct BLOCKFUNCTION
{
	uint8								mUV[6];
	GeneratorFunction					mGenerationFunction;
	LightingFunction					mLightingFunction;
	const std::vector<XMBOUNDINGBOX> *	mBoundingShape;
	//physx::PxShape*						mShape;
};

const std::unordered_map<Blocktype, const BLOCKFUNCTION> BlockfunctionMap =
{
	{ Blocktype::Air,			{ {},							nullptr,	nullptr, &BlockBoundingBox } },
	{ Blocktype::Void,			{ {},							nullptr,	nullptr, &BlockBoundingBox } },
	{ Blocktype::Dirt,			{ { 1, 1, 1, 1, 1, 1 },			Cube,		nullptr, &BlockBoundingBox } },
	{ Blocktype::Grass,			{ { 3, 3, 3, 3, 1, 2 },			Cube,		nullptr, &BlockBoundingBox } },
	{ Blocktype::Stone,			{ { 0, 0, 0, 0, 0, 0 },			Cube,		nullptr, &BlockBoundingBox } },
	{ Blocktype::Cobble,		{ { 4, 4, 4, 4, 4, 4 },			Cube,		nullptr, &BlockBoundingBox } },
	{ Blocktype::Wood,			{ { 7, 7, 7, 7, 6, 6 },			Cube,		nullptr, &BlockBoundingBox } },
	{ Blocktype::WoodPlank,		{ { 6, 6, 6, 6, 6, 6 },			Cube,		nullptr, &BlockBoundingBox } },
	{ Blocktype::Leaf,			{ { 8, 8, 8, 8, 8, 8 },			Cube,		nullptr, &BlockBoundingBox } },
	{ Blocktype::TallGrass,		{ { 9, 9, 9, 9, 9, 9 },			Cross,		nullptr, nullptr	} },
	{ Blocktype::Sand,			{ { 5, 5, 5, 5, 5, 5 },			Cube,		nullptr, &BlockBoundingBox } },
	{ Blocktype::IronOre,		{ { 10, 10, 10, 10, 10, 10 },	Cube,		nullptr, &BlockBoundingBox } },
	{ Blocktype::GoldOre,		{ { 11, 11, 11, 11, 11, 11 },	Cube,		nullptr, &BlockBoundingBox } },
	{ Blocktype::CoalOre,		{ { 12, 12, 12, 12, 12, 12 },	Cube,		nullptr, &BlockBoundingBox } },
	{ Blocktype::DiamandOre,	{ { 13, 13, 13, 13, 13, 13 },	Cube,		nullptr, &BlockBoundingBox } },
	{ Blocktype::RedstoneOre,	{ { 14, 14, 14, 14, 14, 14 },	Cube,		nullptr, &BlockBoundingBox } },
	{ Blocktype::Lightstone,	{ { 15, 15, 15, 15, 15, 15 },	Cube,		nullptr, &BlockBoundingBox } },
};

void Block::GetTextureIdizes(uint8 * pIndizes) const
{
	auto it = BlockfunctionMap.find(mBlocktype);
	if (it != BlockfunctionMap.end())
		memcpy(pIndizes, it->second.mUV, sizeof(uint8) * 6);
}

size_t Block::GetMesh(const Vector3& position, const Vector3& color, uint8 culling, std::vector<BLOCK_MESH> & mesh) const
{
	auto it = BlockfunctionMap.find(mBlocktype);
	if (it != BlockfunctionMap.end())
		return it->second.mGenerationFunction(position, color,
		Vector3(0.5f, 0.5f, 0.5f),
		Vector3(0.5f, 0.5f, 0.5f),
		culling, *this, mesh);
	else
		return 0;
}

const std::vector<XMBOUNDINGBOX> * Block::GetBoundingShape() const
{
	auto it = BlockfunctionMap.find(mBlocktype);
	if (it != BlockfunctionMap.end())
		return it->second.mBoundingShape;
	else
		return nullptr;
}

void Block::Enlight()
{
	auto it = BlockfunctionMap.find(mBlocktype);
	if (it != BlockfunctionMap.end() && 
		it->second.mLightingFunction)
		it->second.mLightingFunction(*this);
}