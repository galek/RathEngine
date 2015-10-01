#pragma once
#include "stdafx.h"
#include "Defines.h"
#include "Light.h"

#pragma pack(push,1)
enum class Blocktype : uint16
{
	Air = 0,
	Dirt,
	Grass,
	Stone,
	Cobble,
	CoalOre,
	IronOre,
	GoldOre,
	DiamandOre,
	RedstoneOre,
	Wood,
	WoodPlank,
	Leaf,
	TallGrass,
	Glowstone,
	Reed,
	Cactus,
	Sand,
	Glass,
	Wool = 25,
	Lightstone = 30,
	Torch = 40,
	RedstoneDust = 90,
	RedstoneTorch = 91,
	Fence = 95,
	NetherFence,
	Water = 100,
	Ice,
	Lava,
	Void = 255,
};

enum class Blockstatus : uint8
{
	Clear = 0x00,
	Solid = 0x01,
	Walkable = 0x02,
	Powered = 0x04,
	Lightsource = 0x08,
	LDVMask = 0xF0,
	LDV1 = 0x10,
	LDV2 = 0x20,
	LDV3 = 0x30,
	LDV4 = 0x40,
	LDV5 = 0x50,
	LDV6 = 0x60,
	LDV7 = 0x70,
	LDV8 = 0x80,
	LDV9 = 0x90,
	LDV10 = 0xA0,
	LDV11 = 0xB0,
	LDV12 = 0xC0,
	LDV13 = 0xD0,
	LDV14 = 0xE0,
	LDV15 = 0xF0,
};
DEFINE_ENUM_FLAG_OPERATORS(Blockstatus);

enum class Blockupdate : uint8
{
	None = 0x00,
	Light = 0x01,
	Geometry = 0x02,
	All = 0x0F,
	IfAir = 0x10,
	IfNotAir = 0x20,
	Condition = 0xF0,
};
DEFINE_ENUM_FLAG_OPERATORS(Blockupdate);

struct Block
{
	union
	{
		struct
		{
			Blocktype	mBlocktype;
			Light		mLight;
			union
			{
				Blockstatus mStatus;
				struct
				{
					bool	mOpaque			: 1;
					bool	mPassable		: 1;
					bool	mPowered		: 1;
					bool	mLightsource	: 1;
					uint8	mLDV			: 4;
				};
			};
			union
			{
				uint8 mMetadata;
				struct
				{
					uint8 mMetaHi : 4;
					uint8 mMetaLo : 4;
				};
			};
			union
			{
				uint16 mUserdata;
				struct
				{
					uint8 mUserHi;
					uint8 mUserLo;
				};
			};
		};
		uint64 mBits;
	};

	bool operator==(const Block& rhs) const;
	bool operator<(const Block& rhs) const;

	void					GetTextureIdizes(uint8 * pIndizes) const;
	void					Enlight();
	size_t					GetMesh(const Vector3& position, const Vector3& color, uint8 culling, std::vector<BLOCK_MESH> & mesh) const;
	const	std::vector<XMBOUNDINGBOX> * GetBoundingShape() const;

	static const Block Sky;
	static const Block Air;
	static const Block Dirt;
	static const Block Grass;
	static const Block SnowyGrass;
	static const Block Stone;
	static const Block Cobble;
	static const Block CoalOre;
	static const Block IronOre;
	static const Block GoldOre;
	static const Block DiamandOre;
	static const Block RedstoneOre;
	static const Block Wood;
	static const Block DarkWood;
	static const Block BirchWood;
	static const Block RedWood;
	static const Block WoodPlank;
	static const Block Leaf;
	static const Block DarkLeaf;
	static const Block RedLeaf;
	static const Block TallGrass;
	static const Block Glowstone;
	static const Block Reed;
	static const Block Cactus;
	static const Block Sand;
	static const Block Lightstone;
	static const Block Torch;
	static const Block RedstoneDust;
	static const Block RedstoneTorch;
	static const Block Fence;
	static const Block Water;
	static const Block Lava;
	static const Block Void;
};
static_assert(sizeof(Block) == 8, "Block-type size is not 64bit");

#pragma pack(pop)

namespace std
{
	template <typename charT, typename traitsT>
	basic_ostream<charT, traitsT> & operator << (basic_ostream<charT, traitsT> & strm, const Block & rhs)
	{
		return strm
			<< _L("ID:")
			<< (uint16)rhs.mBlocktype
			<< _L(" Light:")
			<< rhs.mLight
			<< _L(" Status:0x")
			<< setfill(charT('0'))
			<< hex
			<< setw(2)
			<< (uint8)rhs.mStatus
			<< _L(" Metadata:0x")
			<< setw(2)
			<< rhs.mMetadata
			<< _L(" Userdata:0x")
			<< setw(4)
			<< rhs.mUserdata;
	};
};